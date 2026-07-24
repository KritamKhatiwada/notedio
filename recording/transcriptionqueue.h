#ifndef TRANSCRIPTIONQUEUE_H
#define TRANSCRIPTIONQUEUE_H

#include <QObject>
#include <QQueue>
#include <QMap>
#include <QVector>
#include <QString>

class TranscriptionWorker;

struct ChunkJob {
    int index;
    QString audioFilePath;
    QString outputBasePath;
};

class TranscriptionQueue : public QObject
{
    Q_OBJECT
public:
    explicit TranscriptionQueue(QObject *parent = nullptr);

    void configure(const QString &whisperExePath, const QString &modelPath, int threadsPerWorker, int maxConcurrentWorkers);
    void enqueue(const ChunkJob &job);
    void markNoMoreJobs();
    void reset();

    bool isFullyFinished() const;
    QString assembleOrderedTranscript() const;

signals:
    void chunkTranscribed(int chunkIndex, const QString &text);
    void chunkErrored(int chunkIndex, const QString &errorMessage);
    void allChunksFinished();

private:
    void dispatchNext();
    void handleWorkerFinished(TranscriptionWorker *worker, int chunkIndex, const QString &text);
    void handleWorkerFailed(TranscriptionWorker *worker, int chunkIndex, const QString &errorMessage);
    void checkForCompletion();
    void logQueueState(const QString &reason) const;

    QString m_whisperExePath;
    QString m_modelPath;
    int m_threadsPerWorker = 8;
    int m_maxConcurrentWorkers = 2;

    QQueue<ChunkJob> m_pendingJobs;
    QVector<TranscriptionWorker *> m_workerPool;
    QMap<TranscriptionWorker *, bool> m_workerBusy;
    QMap<int, QString> m_completedChunks;
    int m_totalJobsEnqueued = 0;
    bool m_noMoreJobsWillArrive = false;
};

#endif
