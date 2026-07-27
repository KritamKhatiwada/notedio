#include "AudioChunkRecorder.h"
#include "AppConfig.h"

#include <QMediaFormat>
#include <QUrl>
#include <QDebug>

AudioChunkRecorder::AudioChunkRecorder(QObject *parent)
    : QObject(parent)
    , m_state(State::Stopped)
    , m_chunkIndex(0)
    , m_pendingFinal(false)
{
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    m_recorder.setMediaFormat(format);

    m_captureSession.setAudioInput(&m_audioInput);
    m_captureSession.setRecorder(&m_recorder);

    m_chunkTimer.setInterval(AppConfig::instance().chunkDurationSeconds() * 1000);
    connect(&m_chunkTimer, &QTimer::timeout, this, &AudioChunkRecorder::rotateChunk);
    connect(&m_recorder, &QMediaRecorder::recorderStateChanged, this, [this](QMediaRecorder::RecorderState state) {
        if (state == QMediaRecorder::StoppedState)
            onRecorderStopped();
    });
}

AudioChunkRecorder::State AudioChunkRecorder::state() const
{
    return m_state;
}

QString AudioChunkRecorder::chunkFilePath(int index) const
{
    return AppConfig::instance().chunkWorkDir().filePath(QString("chunk_%1.wav").arg(index, 4, 10, QChar('0')));
}

void AudioChunkRecorder::start()
{
    if (m_state != State::Stopped)
        return;

    m_chunkIndex = 0;
    m_pendingFinal = false;
    beginChunk();
    m_chunkTimer.start();

    m_state = State::Recording;
    emit stateChanged(m_state);
    qDebug() << "[AudioChunkRecorder] recording started, chunk duration"
             << AppConfig::instance().chunkDurationSeconds() << "s";
}

void AudioChunkRecorder::beginChunk()
{
    const QString path = chunkFilePath(m_chunkIndex);
    m_recorder.setOutputLocation(QUrl::fromLocalFile(path));
    m_recorder.record();
    qDebug() << "[AudioChunkRecorder] chunk" << m_chunkIndex << "recording to" << path;
}

void AudioChunkRecorder::rotateChunk()
{
    if (m_state != State::Recording)
        return;

    m_pendingFinal = false;
    m_recorder.stop();
}

void AudioChunkRecorder::onRecorderStopped()
{
    const int completedIndex = m_chunkIndex;
    emit chunkReady(chunkFilePath(completedIndex), completedIndex, m_pendingFinal);

    if (m_pendingFinal) {
        m_state = State::Stopped;
        emit stateChanged(m_state);
        qDebug() << "[AudioChunkRecorder] recording finalized after" << (completedIndex + 1) << "chunk(s)";
        return;
    }

    if (m_state == State::Recording) {
        m_chunkIndex++;
        beginChunk();
    }
}

void AudioChunkRecorder::pause()
{
    if (m_state != State::Recording)
        return;
    m_chunkTimer.stop();
    m_recorder.pause();
    m_state = State::Paused;
    emit stateChanged(m_state);
    qDebug() << "[AudioChunkRecorder] paused";
}

void AudioChunkRecorder::resume()
{
    if (m_state != State::Paused)
        return;
    m_recorder.record();
    m_chunkTimer.start();
    m_state = State::Recording;
    emit stateChanged(m_state);
    qDebug() << "[AudioChunkRecorder] resumed";
}

void AudioChunkRecorder::stop()
{
    if (m_state == State::Stopped)
        return;
    m_chunkTimer.stop();
    m_pendingFinal = true;
    m_recorder.stop();
    qDebug() << "[AudioChunkRecorder] stop requested, finalizing last chunk";
}
