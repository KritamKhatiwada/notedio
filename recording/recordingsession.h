#ifndef RECORDINGSESSION_H
#define RECORDINGSESSION_H

#include <QObject>
#include <QString>

class ChunkRecorder;
class TranscriptionQueue;
class NotesRepository;

class RecordingSession : public QObject
{
    Q_OBJECT
public:
    explicit RecordingSession(NotesRepository *repository, QObject *parent = nullptr);

    void begin();
    void pause();
    void resume();
    void finish();

    bool isRecording() const;
    bool isPaused() const;
    qint64 elapsedMs() const;
    bool isFinalizing() const;

signals:
    // hadFailures is true if one or more chunks could not be transcribed -
    // the note is still saved, but the caller should tell the user their
    // transcript may be incomplete rather than silently doing nothing.
    void noteReady(int noteId, bool hadFailures);
    void finalizingProgress(int completedChunks, int totalChunks);

private slots:
    void onChunkReady(int chunkIndex, const QString &filePath);
    void onSessionStopped(int totalChunkCount);
    void onChunkTranscribed(int chunkIndex, const QString &text);
    void onAllChunksFinished();

private:
    ChunkRecorder *m_recorder = nullptr;
    TranscriptionQueue *m_queue = nullptr;
    NotesRepository *m_repository = nullptr;

    QString m_sessionDirectory;
    QString m_transcriptDirectory;
    int m_noteId = -1;
    int m_totalChunks = 0;
    int m_completedChunks = 0;
    int m_failedChunks = 0;
};

#endif
