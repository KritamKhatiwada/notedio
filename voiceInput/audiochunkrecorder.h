#pragma once
#include <QObject>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>

class AudioChunkRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioChunkRecorder(const QString &sessionDir, QObject *parent = nullptr);

    void start();
    void pause();
    void resume();
    void stop();

    qint64 elapsedMs() const;
    bool isRecording() const;
    bool isPaused() const;

signals:
    void chunkReady(int index, const QString &path, bool isFinal);
    void recordingStarted();
    void recordingStopped();

private slots:
    void rotateChunk();

private:
    void beginSegment();
    void endSegment(bool isFinal);

    QMediaCaptureSession m_session;
    QAudioInput m_input;
    QMediaRecorder m_recorder;
    QTimer m_rotationTimer;
    QElapsedTimer m_elapsed;
    QDir m_sessionDir;
    int m_chunkIndex = 0;
    bool m_recording = false;
    bool m_paused = false;
    static constexpr int kChunkIntervalMs = 60000;
};
