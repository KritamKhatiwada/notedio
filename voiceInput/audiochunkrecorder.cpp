#include "audiochunkrecorder.h"
#include <QMediaFormat>
#include <QUrl>


AudioChunkRecorder::AudioChunkRecorder(const QString &sessionDir, QObject *parent)
    : QObject(parent), m_sessionDir(sessionDir)
{
    if (!m_sessionDir.exists())
        QDir().mkpath(m_sessionDir.absolutePath());

    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    m_recorder.setMediaFormat(format);
    m_session.setAudioInput(&m_input);
    m_session.setRecorder(&m_recorder);

    m_rotationTimer.setInterval(kChunkIntervalMs);
    connect(&m_rotationTimer, &QTimer::timeout, this, &AudioChunkRecorder::rotateChunk);
}

void AudioChunkRecorder::start()
{
    if (m_recording)
        return;

    m_chunkIndex = 0;
    m_recording = true;
    m_paused = false;
    m_elapsed.start();
    beginSegment();
    m_rotationTimer.start();
    emit recordingStarted();
}

void AudioChunkRecorder::beginSegment()
{
    QString path = m_sessionDir.filePath(QString("chunk_%1.wav").arg(m_chunkIndex, 4, 10, QChar('0')));
    m_recorder.setOutputLocation(QUrl::fromLocalFile(path));
    m_recorder.record();
}

void AudioChunkRecorder::rotateChunk()
{
    if (!m_recording || m_paused)
        return;

    endSegment(false);
    m_chunkIndex++;
    beginSegment();
}

void AudioChunkRecorder::endSegment(bool isFinal)
{
    QString path = m_recorder.outputLocation().toLocalFile();
    m_recorder.stop();
    emit chunkReady(m_chunkIndex, path, isFinal);
}

void AudioChunkRecorder::pause()
{
    if (!m_recording || m_paused)
        return;

    m_paused = true;
    m_rotationTimer.stop();
    m_recorder.pause();
}

void AudioChunkRecorder::resume()
{
    if (!m_recording || !m_paused)
        return;

    m_paused = false;
    m_recorder.record();
    m_rotationTimer.start();
}

void AudioChunkRecorder::stop()
{
    if (!m_recording)
        return;

    m_rotationTimer.stop();
    endSegment(true);
    m_recording = false;
    m_paused = false;
    emit recordingStopped();
}

qint64 AudioChunkRecorder::elapsedMs() const
{
    return m_recording ? m_elapsed.elapsed() : 0;
}

bool AudioChunkRecorder::isRecording() const { return m_recording; }
bool AudioChunkRecorder::isPaused() const { return m_paused; }
