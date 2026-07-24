#include "transcriptionqueue.h"
#include "transcriptionworker.h"
#include <QDebug>

TranscriptionQueue::TranscriptionQueue(QObject *parent)
    : QObject(parent)
{
}

void TranscriptionQueue::configure(const QString &whisperExePath, const QString &modelPath, int threadsPerWorker, int maxConcurrentWorkers)
{
    m_whisperExePath = whisperExePath;
    m_modelPath = modelPath;
    m_threadsPerWorker = threadsPerWorker;
    m_maxConcurrentWorkers = maxConcurrentWorkers;

    qInfo() << "[Queue] configure: workers=" << m_maxConcurrentWorkers
            << "threads/worker=" << m_threadsPerWorker
            << "exe=" << m_whisperExePath << "model=" << m_modelPath;

    for (int i = 0; i < m_maxConcurrentWorkers; ++i) {
        auto *worker = new TranscriptionWorker(i, this);
        worker->configure(m_whisperExePath, m_modelPath, m_threadsPerWorker);

        connect(worker, &TranscriptionWorker::chunkFinished, this,
                [this, worker](int chunkIndex, const QString &text) {
                    handleWorkerFinished(worker, chunkIndex, text);
                });

        connect(worker, &TranscriptionWorker::chunkFailed, this,
                [this, worker](int chunkIndex, const QString &errorMessage) {
                    handleWorkerFailed(worker, chunkIndex, errorMessage);
                });

        m_workerPool.append(worker);
        m_workerBusy[worker] = false;
    }
}

void TranscriptionQueue::enqueue(const ChunkJob &job)
{
    qInfo() << "[Queue] ENQUEUE chunk" << job.index << "->" << job.audioFilePath;
    m_pendingJobs.enqueue(job);
    m_totalJobsEnqueued++;
    dispatchNext();
}

void TranscriptionQueue::markNoMoreJobs()
{
    qInfo() << "[Queue] no more jobs will arrive. pending=" << m_pendingJobs.size()
    << "completed=" << m_completedChunks.size() << "/" << m_totalJobsEnqueued;
    m_noMoreJobsWillArrive = true;
    checkForCompletion();
}

void TranscriptionQueue::reset()
{
    qInfo() << "[Queue] reset";
    m_pendingJobs.clear();
    m_completedChunks.clear();
    m_totalJobsEnqueued = 0;
    m_noMoreJobsWillArrive = false;
}

void TranscriptionQueue::logQueueState(const QString &reason) const
{
    int busyCount = 0;
    for (auto it = m_workerBusy.constBegin(); it != m_workerBusy.constEnd(); ++it)
        if (it.value()) busyCount++;

    qInfo() << "[Queue]" << reason
            << "pending=" << m_pendingJobs.size()
            << "busyWorkers=" << busyCount << "/" << m_workerPool.size()
            << "completed=" << m_completedChunks.size() << "/" << m_totalJobsEnqueued
            << "noMoreJobs=" << m_noMoreJobsWillArrive;
}

void TranscriptionQueue::dispatchNext()
{
    if (m_pendingJobs.isEmpty()) {
        logQueueState("dispatchNext: nothing pending");
        return;
    }

    if (m_workerPool.isEmpty()) {
        qWarning() << "[Queue] dispatchNext: worker pool is EMPTY — configure() was never called or ran with 0 workers";
        return;
    }

    for (auto *worker : std::as_const(m_workerPool)) {
        if (m_workerBusy[worker])
            continue;

        const ChunkJob job = m_pendingJobs.dequeue();
        m_workerBusy[worker] = true;
        worker->start(job.index, job.audioFilePath, job.outputBasePath);

        if (m_pendingJobs.isEmpty())
            break;
    }

    logQueueState("dispatchNext: after dispatch");
}

void TranscriptionQueue::handleWorkerFinished(TranscriptionWorker *worker, int chunkIndex, const QString &text)
{
    m_completedChunks[chunkIndex] = text;
    m_workerBusy[worker] = false;

    qInfo() << "[Queue] chunk" << chunkIndex << "TRANSCRIBED, chars=" << text.length();

    emit chunkTranscribed(chunkIndex, text);

    dispatchNext();
    checkForCompletion();
}

void TranscriptionQueue::handleWorkerFailed(TranscriptionWorker *worker, int chunkIndex, const QString &errorMessage)
{
    qWarning() << "[Queue] chunk" << chunkIndex << "ERRORED:" << errorMessage;

    m_completedChunks[chunkIndex] = QString();
    m_workerBusy[worker] = false;

    emit chunkErrored(chunkIndex, errorMessage);

    dispatchNext();
    checkForCompletion();
}

void TranscriptionQueue::checkForCompletion()
{
    if (isFullyFinished()) {
        qInfo() << "[Queue] ALL CHUNKS FINISHED (" << m_completedChunks.size() << "total )";
        emit allChunksFinished();
    }
}

bool TranscriptionQueue::isFullyFinished() const
{
    return m_noMoreJobsWillArrive
           && m_pendingJobs.isEmpty()
           && m_completedChunks.size() == m_totalJobsEnqueued;
}

QString TranscriptionQueue::assembleOrderedTranscript() const
{
    QString result;
    for (auto it = m_completedChunks.constBegin(); it != m_completedChunks.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            if (!result.isEmpty())
                result += " ";
            result += it.value();
        }
    }
    return result;
}
