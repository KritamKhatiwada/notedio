#include "RecordingWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

RecordingWindow::RecordingWindow(QWidget *parent)
    : QDialog(parent)
    , m_recorder(new AudioChunkRecorder(this))
    , m_transcriber(new TranscriptionEngine(this))
    , m_recordButton(new QPushButton("RECORD", this))
    , m_pauseButton(new QPushButton("PAUSE", this))
    , m_statusLabel(new QLabel("Ready", this))
    , m_liveTranscript(new QTextEdit(this))
    , m_awaitingFinalTranscript(false)
{
    setWindowTitle("New Recording");
    resize(520, 420);

    m_pauseButton->setEnabled(false);
    m_liveTranscript->setReadOnly(true);
    m_liveTranscript->setPlaceholderText("Live transcript will appear here as background chunks complete...");

    auto *buttonsRow = new QHBoxLayout;
    buttonsRow->addWidget(m_recordButton);
    buttonsRow->addWidget(m_pauseButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonsRow);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_liveTranscript);

    connect(m_recordButton, &QPushButton::clicked, this, &RecordingWindow::onRecordClicked);
    connect(m_pauseButton, &QPushButton::clicked, this, &RecordingWindow::onPauseClicked);
    connect(m_recorder, &AudioChunkRecorder::stateChanged, this, &RecordingWindow::onRecorderStateChanged);
    connect(m_recorder, &AudioChunkRecorder::chunkReady, this, &RecordingWindow::onChunkReady);
    connect(m_transcriber, &TranscriptionEngine::transcriptProgress, this, &RecordingWindow::onTranscriptProgress);
    connect(m_transcriber, &TranscriptionEngine::transcriptFinalized, this, &RecordingWindow::onTranscriptFinalized);
    connect(m_transcriber, &TranscriptionEngine::chunkFailed, this, [](int index, const QString &reason) {
        qDebug() << "[RecordingWindow] chunk" << index << "transcription failed:" << reason;
    });
}

void RecordingWindow::onRecordClicked()
{
    if (m_recorder->state() == AudioChunkRecorder::State::Stopped) {
        m_transcriber->reset();
        m_liveTranscript->clear();
        m_recorder->start();
        m_recordButton->setText("STOP");
        m_pauseButton->setEnabled(true);
        m_statusLabel->setText("Recording...");
    } else {
        m_awaitingFinalTranscript = true;
        m_recorder->stop();
        m_recordButton->setEnabled(false);
        m_pauseButton->setEnabled(false);
        m_statusLabel->setText("Finalizing transcript...");
    }
}

void RecordingWindow::onPauseClicked()
{
    if (m_recorder->state() == AudioChunkRecorder::State::Recording) {
        m_recorder->pause();
        m_pauseButton->setText("RESUME");
    } else if (m_recorder->state() == AudioChunkRecorder::State::Paused) {
        m_recorder->resume();
        m_pauseButton->setText("PAUSE");
    }
}

void RecordingWindow::onRecorderStateChanged(AudioChunkRecorder::State state)
{
    qDebug() << "[RecordingWindow] recorder state changed";
    if (state == AudioChunkRecorder::State::Stopped && !m_awaitingFinalTranscript) {
        m_recordButton->setEnabled(true);
        m_recordButton->setText("RECORD");
        m_pauseButton->setEnabled(false);
    }
}

void RecordingWindow::onChunkReady(const QString &filePath, int chunkIndex, bool isFinal)
{
    qDebug() << "[RecordingWindow] chunk" << chunkIndex << "ready, final =" << isFinal;
    m_transcriber->enqueueChunk(filePath, chunkIndex, isFinal);
}

void RecordingWindow::onTranscriptProgress(const QString &text)
{
    m_liveTranscript->setPlainText(text);
}

void RecordingWindow::onTranscriptFinalized(const QString &text)
{
    m_liveTranscript->setPlainText(text);
    m_statusLabel->setText("Done");
    m_recordButton->setEnabled(true);
    m_recordButton->setText("RECORD");
    m_awaitingFinalTranscript = false;

    qDebug() << "[RecordingWindow] emitting recordingFinished, length" << text.length();
    emit recordingFinished(text);
    accept();
}
