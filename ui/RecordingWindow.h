#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include "../core/AudioChunkRecorder.h"
#include "../core/TranscriptionEngine.h"

class RecordingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RecordingWindow(QWidget *parent = nullptr);

signals:
    void recordingFinished(const QString &fullTranscript);

private slots:
    void onRecordClicked();
    void onPauseClicked();
    void onRecorderStateChanged(AudioChunkRecorder::State state);
    void onChunkReady(const QString &filePath, int chunkIndex, bool isFinal);
    void onTranscriptProgress(const QString &text);
    void onTranscriptFinalized(const QString &text);

private:
    AudioChunkRecorder *m_recorder;
    TranscriptionEngine *m_transcriber;

    QPushButton *m_recordButton;
    QPushButton *m_pauseButton;
    QLabel *m_statusLabel;
    QTextEdit *m_liveTranscript;

    bool m_awaitingFinalTranscript;
};
