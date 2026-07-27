#pragma once

#include <QObject>
#include <QQueue>
#include <QProcess>
#include <QMap>

class TranscriptionEngine : public QObject
{
    Q_OBJECT

public:
    explicit TranscriptionEngine(QObject *parent = nullptr);

public slots:
    void enqueueChunk(const QString &filePath, int chunkIndex, bool isFinal);
    void reset();

signals:
    void transcriptProgress(const QString &fullTextSoFar);
    void transcriptFinalized(const QString &fullText);
    void chunkFailed(int chunkIndex, const QString &reason);

private slots:
    void processNext();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    struct PendingChunk {
        QString filePath;
        int chunkIndex;
        bool isFinal;
    };

    QString outputTextPathFor(const QString &wavPath) const;
    void emitAssembledText();

    QQueue<PendingChunk> m_queue;
    QMap<int, QString> m_transcribedChunks;
    QProcess *m_process;
    PendingChunk m_current;
    bool m_busy;
    int m_nextExpectedIndex;
};
