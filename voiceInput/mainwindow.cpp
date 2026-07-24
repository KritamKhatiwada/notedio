#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../recording/recordingsession.h"
#include "../database/database.h"
#include <QTimer>
#include <QIcon>
#include <QMessageBox>
MainWindow::MainWindow(NotesRepository *repository, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_session = new RecordingSession(repository, this);
    m_uiTimer = new QTimer(this);
    m_uiTimer->setInterval(500);
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::onElapsedTick);
    connect(m_session, &RecordingSession::finalizingProgress, this, &MainWindow::onFinalizingProgress);
    connect(m_session, &RecordingSession::noteReady, this, &MainWindow::onNoteReady);
    connect(ui->recordBTN, &QPushButton::clicked, this, &MainWindow::onRecordClicked);
    connect(ui->pauseBTN, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    resetUiToIdle();
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::showPopup(const QString &title, const QString &message)
{
    // Non-modal so it never blocks the recording/transcription pipeline
    // running in the background.
    QMessageBox *box = new QMessageBox(QMessageBox::Information, title, message, QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setModal(false);
    box->show();
}
void MainWindow::onRecordClicked()
{
    if (!m_session->isRecording() && !m_session->isPaused()) {
        m_session->begin();
        m_uiTimer->start();
        ui->recordBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop));
        ui->recordBTN->setText("STOP");
        ui->recordBTN->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold;");
        ui->pauseBTN->setEnabled(true);
    } else {
        m_session->finish();
        m_uiTimer->stop();
        ui->recordBTN->setEnabled(false);
        ui->pauseBTN->setEnabled(false);
        ui->recordBTN->setText("SAVING...");
        showPopup("Recording Stopped", "Recording stopped. Transcribing your audio now...");
        emit backToHomeRequested();
    }
}
void MainWindow::onPauseClicked()
{
    if (m_session->isPaused()) {
        m_session->resume();
        ui->pauseBTN->setText("PAUSE");
        ui->pauseBTN->setStyleSheet("background-color: #6d6d6d;");
        ui->pauseBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
        m_uiTimer->start();
    } else if (m_session->isRecording()) {
        m_session->pause();
        ui->pauseBTN->setText("RESUME");
        ui->pauseBTN->setStyleSheet("background-color: #008000; color: white; font-weight: bold;");
        ui->pauseBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
        m_uiTimer->stop();
    }
}
void MainWindow::onElapsedTick()
{
    ui->lineEdit->setText(formatElapsed(m_session->elapsedMs()));
}
void MainWindow::onFinalizingProgress(int completed, int total)
{
    ui->lineEdit->setText(QString("Transcribing %1/%2").arg(completed).arg(total));
}
void MainWindow::onNoteReady(int noteId, bool hadFailures)
{
    emit noteReady(noteId, hadFailures);
    resetUiToIdle();
}
void MainWindow::resetUiToIdle()
{
    ui->recordBTN->setEnabled(true);
    ui->pauseBTN->setEnabled(false);
    ui->recordBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioInputMicrophone));
    ui->recordBTN->setText("RECORD");
    ui->recordBTN->setStyleSheet("background-color: #6d6d6d;");
    ui->lineEdit->setText("00:00:00");
}
QString MainWindow::formatElapsed(qint64 ms) const
{
    const qint64 totalSeconds = ms / 1000;
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds % 3600) / 60;
    const int seconds = totalSeconds % 60;
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
