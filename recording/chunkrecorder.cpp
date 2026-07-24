#include "chunkrecorder.h"

#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>
#include <QUrl>
#include <QDebug>

ChunkRecorder::ChunkRecorder(QObject *parent)
    : QObject(parent)
{
    m_captureSession = new QMediaCaptureSession(this);
    m_audioInput = new QAudioInput(this);
    m_recorder = new QMediaRecorder(this);

    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    m_recorder->setMediaFormat(format);

    m_captureSession->setAudioInput(m_audioInput);
    m_captureSession->setRecorder(m_recorder);

    m_rotationTimer = new QTimer(this);
    connect(m_rotationTimer, &QTimer::timeout, this, &ChunkRecorder::rotateChunk);

    m_sessionClock = new QElapsedTimer;

    connect(m_recorder, &QMediaRecorder::recorderStateChanged, this, &ChunkRecorder::onRecorderStateChanged);
}

void ChunkRecorder::configure(const QString &sessionDirectory, int chunkDurationMs)
{
    m_sessionDirectory = sessionDirectory;
    m_chunkDurationMs = chunkDurationMs;
    QDir().mkpath(m_sessionDirectory);
}

QString ChunkRecorder::nextChunkPath() const
{
    return QDir(m_sessionDirectory).filePath(QString("chunk_%1.wav").arg(m_chunkIndex, 4, 10, QChar('0')));
}


void ChunkRecorder::startSession()
{
    m_chunkIndex = 0;
    m_stopping = false;
    m_paused = false;
    m_sessionClock->start();

    qInfo() << "[Recorder] SESSION START, chunkDurationMs=" << m_chunkDurationMs
            << "dir=" << m_sessionDirectory;

    m_recorder->setOutputLocation(QUrl::fromLocalFile(nextChunkPath()));
    m_recorder->record();
    m_rotationTimer->start(m_chunkDurationMs);
}

void ChunkRecorder::pauseSession()
{
    m_paused = true;
    m_rotationTimer->stop();
    m_recorder->pause();
}

void ChunkRecorder::resumeSession()
{
    m_paused = false;
    m_recorder->record();
    m_rotationTimer->start(m_chunkDurationMs);
}

void ChunkRecorder::stopSession()
{
    m_stopping = true;
    m_rotationTimer->stop();
    m_recorder->stop();
}

void ChunkRecorder::rotateChunk()
{
    m_recorder->stop();
}

void ChunkRecorder::onRecorderStateChanged()
{
    if (m_recorder->recorderState() != QMediaRecorder::StoppedState)
        return;

    const QString finishedPath = m_recorder->outputLocation().toLocalFile();
    if (finishedPath.isEmpty())
        return;

    const int finishedIndex = m_chunkIndex;
    qInfo() << "[Recorder] chunk" << finishedIndex << "READY at" << elapsedMs() << "ms elapsed ->" << finishedPath;
    emit chunkReady(finishedIndex, finishedPath);

    if (m_stopping) {
        qInfo() << "[Recorder] SESSION STOPPED, total chunks=" << finishedIndex + 1;
        emit sessionStopped(finishedIndex + 1);
        return;
    }

    m_chunkIndex++;
    m_recorder->setOutputLocation(QUrl::fromLocalFile(nextChunkPath()));
    m_recorder->record();
    qInfo() << "[Recorder] chunk" << m_chunkIndex << "STARTED";
}
bool ChunkRecorder::isRecording() const
{
    return m_recorder->recorderState() == QMediaRecorder::RecordingState;
}

bool ChunkRecorder::isPaused() const
{
    return m_paused;
}

qint64 ChunkRecorder::elapsedMs() const
{
    return m_sessionClock->isValid() ? m_sessionClock->elapsed() : 0;
}
