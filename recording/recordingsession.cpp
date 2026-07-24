#include "recordingsession.h"
#include "chunkrecorder.h"
#include "transcriptionqueue.h"
#include "../database/database.h"
#include "whisperlocator.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDebug>

namespace {
constexpr int kChunkDurationMs = 20000;
constexpr int kThreadsPerWorker = 1;
constexpr int kMaxConcurrentWorkers = 1;
}

RecordingSession::RecordingSession(NotesRepository *repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    m_recorder = new ChunkRecorder(this);
    m_queue = new TranscriptionQueue(this);

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString whisperExe = WhisperLocator::findWhisperExe(appDir);
    const QString modelPath = WhisperLocator::findModel(appDir);

    if (whisperExe.isEmpty() || modelPath.isEmpty()) {
        qCritical() << "[Session] whisper-cli or model missing - transcription will fail for every chunk."
                    << "Build whisper.cpp (see CMakeLists notes) and/or copy the model into a models/ folder next to the executable.";
    }

    m_queue->configure(whisperExe, modelPath, kThreadsPerWorker, kMaxConcurrentWorkers);

    connect(m_recorder, &ChunkRecorder::chunkReady, this, &RecordingSession::onChunkReady);
    connect(m_recorder, &ChunkRecorder::sessionStopped, this, &RecordingSession::onSessionStopped);
    connect(m_queue, &TranscriptionQueue::chunkTranscribed, this, &RecordingSession::onChunkTranscribed);
    connect(m_queue, &TranscriptionQueue::chunkErrored, this, [this](int idx, const QString &err) {
        qWarning() << "[Session] chunk" << idx << "failed:" << err;
        m_completedChunks++;
        m_failedChunks++;
        emit finalizingProgress(m_completedChunks, m_totalChunks);
    });
    connect(m_queue, &TranscriptionQueue::allChunksFinished, this, &RecordingSession::onAllChunksFinished);
}

void RecordingSession::begin()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    m_sessionDirectory = QDir(appDir).filePath(QString("voiceInput/sessions/%1").arg(timestamp));
    m_transcriptDirectory = QDir(m_sessionDirectory).filePath("transcripts");
    QDir().mkpath(m_transcriptDirectory);

    qInfo() << "[Session] BEGIN, sessionDir=" << m_sessionDirectory;

    m_noteId = m_repository->createNote(QString("Untitled recording"), m_sessionDirectory, QDateTime::currentDateTime());
    m_totalChunks = 0;
    m_completedChunks = 0;
    m_failedChunks = 0;
    m_queue->reset();

    m_recorder->configure(m_sessionDirectory, kChunkDurationMs);
    m_recorder->startSession();
}

void RecordingSession::pause()
{
    qInfo() << "[Session] PAUSE";
    m_recorder->pauseSession();
}

void RecordingSession::resume()
{
    qInfo() << "[Session] RESUME";
    m_recorder->resumeSession();
}

void RecordingSession::finish()
{
    qInfo() << "[Session] FINISH requested";
    m_recorder->stopSession();
}

bool RecordingSession::isRecording() const
{
    return m_recorder->isRecording();
}

bool RecordingSession::isPaused() const
{
    return m_recorder->isPaused();
}

qint64 RecordingSession::elapsedMs() const
{
    return m_recorder->elapsedMs();
}

bool RecordingSession::isFinalizing() const
{
    return m_totalChunks > 0 && m_completedChunks < m_totalChunks;
}

void RecordingSession::onChunkReady(int chunkIndex, const QString &filePath)
{
    ChunkJob job;
    job.index = chunkIndex;
    job.audioFilePath = filePath;
    job.outputBasePath = QDir(m_transcriptDirectory).filePath(QString("chunk_%1").arg(chunkIndex, 4, 10, QChar('0')));

    m_totalChunks++;
    qInfo() << "[Session] chunk" << chunkIndex << "handed to transcription queue, total so far=" << m_totalChunks;
    m_queue->enqueue(job);
}

void RecordingSession::onSessionStopped(int totalChunkCount)
{
    Q_UNUSED(totalChunkCount)
    qInfo() << "[Session] recorder stopped, finalizing" << (m_totalChunks - m_completedChunks) << "remaining chunks";
    m_repository->updateDuration(m_noteId, elapsedMs());
    m_queue->markNoMoreJobs();
}

void RecordingSession::onChunkTranscribed(int chunkIndex, const QString &text)
{
    Q_UNUSED(chunkIndex)
    Q_UNUSED(text)
    m_completedChunks++;
    emit finalizingProgress(m_completedChunks, m_totalChunks);
}

void RecordingSession::onAllChunksFinished()
{
    const QString fullText = m_queue->assembleOrderedTranscript();
    const QString description = fullText.left(160);
    const QString html = "<p>" + fullText.toHtmlEscaped().replace("\n", "</p><p>") + "</p>";

    qInfo() << "[Session] note" << m_noteId << "COMPLETE, transcript chars=" << fullText.length();

    m_repository->updateTranscription(m_noteId, html, description);

    if (m_failedChunks > 0) {
        qWarning() << "[Session] note" << m_noteId << "finished with" << m_failedChunks
                   << "of" << m_totalChunks << "chunks failed";
    }

    emit noteReady(m_noteId, m_failedChunks > 0);
}