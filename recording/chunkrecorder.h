#ifndef CHUNKRECORDER_H
#define CHUNKRECORDER_H

#include <QObject>
#include <QString>

class QMediaCaptureSession;
class QAudioInput;
class QMediaRecorder;
class QTimer;
class QElapsedTimer;

class ChunkRecorder : public QObject
{
    Q_OBJECT
public:
    explicit ChunkRecorder(QObject *parent = nullptr);

    void configure(const QString &sessionDirectory, int chunkDurationMs);
    void startSession();
    void pauseSession();
    void resumeSession();
    void stopSession();

    bool isRecording() const;
    bool isPaused() const;
    qint64 elapsedMs() const;

signals:
    void chunkReady(int chunkIndex, const QString &filePath);
    void sessionStopped(int totalChunkCount);

private slots:
    void rotateChunk();
    void onRecorderStateChanged();

private:
    QString nextChunkPath() const;

    QMediaCaptureSession *m_captureSession = nullptr;
    QAudioInput *m_audioInput = nullptr;
    QMediaRecorder *m_recorder = nullptr;
    QTimer *m_rotationTimer = nullptr;
    QElapsedTimer *m_sessionClock = nullptr;

    QString m_sessionDirectory;
    int m_chunkDurationMs = 60000;
    int m_chunkIndex = -1;
    bool m_stopping = false;
    bool m_paused = false;
};

#endif
