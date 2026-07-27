#pragma once

#include <QObject>
#include <QTimer>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>

class AudioChunkRecorder : public QObject
{
    Q_OBJECT

public:
    explicit AudioChunkRecorder(QObject *parent = nullptr);

    enum class State { Stopped, Recording, Paused };
    State state() const;

public slots:
    void start();
    void pause();
    void resume();
    void stop();

signals:
    void chunkReady(const QString &filePath, int chunkIndex, bool isFinal);
    void stateChanged(AudioChunkRecorder::State state);

private slots:
    void rotateChunk();
    void onRecorderStopped();

private:
    void beginChunk();
    void finishCurrentChunk(bool isFinal);
    QString chunkFilePath(int index) const;

    QMediaCaptureSession m_captureSession;
    QAudioInput m_audioInput;
    QMediaRecorder m_recorder;
    QTimer m_chunkTimer;

    State m_state;
    int m_chunkIndex;
    bool m_pendingFinal;
};
