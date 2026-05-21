#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStandardPaths>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QDir>
#include <QIcon>
#include <QUrl>
#include <QMediaFormat>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_mediacapturesession= new QMediaCaptureSession;
    m_audioinput= new QAudioInput;
    m_audiorecorder= new QMediaRecorder;

    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    m_audiorecorder->setMediaFormat(format);

    QString baseDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString whisperFolder = baseDataPath + "/whispercpp/";

    QDir dir;
    if (!dir.exists(whisperFolder)) {
        dir.mkpath(whisperFolder);
    }

    QString savePath =whisperFolder + "audio.wav";
    m_audiorecorder->setOutputLocation(QUrl::fromLocalFile(savePath));
    qDebug() << "\033[1;32m 🎧 AUDIO WILL SAVE TO:" << savePath << "\033[0m";

    m_mediacapturesession->setAudioInput(m_audioinput);
    m_mediacapturesession->setRecorder(m_audiorecorder);

    connect(ui->recordBTN,&QPushButton::clicked , this , &MainWindow::onRecordClicked);
    connect(ui->pauseBTN,&QPushButton::clicked , this , &MainWindow::onPauseClicked);


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onRecordClicked(){
    if(m_audiorecorder->recorderState() == QMediaRecorder::StoppedState){
    qDebug() << "recording...";
    m_audiorecorder->record();
     ui->recordBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop));
    ui->recordBTN->setText("STOP");
    ui->recordBTN->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold;");
    }
    else if(m_audiorecorder->recorderState() == QMediaRecorder::RecordingState){
        qDebug() << "stoping...";
        m_audiorecorder->stop();
        ui->recordBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioInputMicrophone));
        ui->recordBTN->setText("RECORD");
        ui->recordBTN->setStyleSheet("background-color: #6d6d6d; ");

    }

}
void MainWindow::onPauseClicked(){

    if(m_audiorecorder->recorderState() == QMediaRecorder::PausedState){
        m_audiorecorder->record();
        ui->pauseBTN->setText("PAUSE");
        qDebug() << "Recording is resumed...";
        ui->pauseBTN->setStyleSheet("background-color: #6d6d6d; ");
        ui->pauseBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
    }
    else if(m_audiorecorder->recorderState() == QMediaRecorder::RecordingState){
        m_audiorecorder->pause();
        ui->pauseBTN->setStyleSheet("background-color: #008000; color: white; font-weight: bold;");
        ui->pauseBTN->setText("RESUME");
        qDebug() << "Recordinng is paused.";
        ui->pauseBTN->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    }
    else if (m_audiorecorder->recorderState() == QMediaRecorder::StoppedState) {

        qDebug() << "Cannot pause; the recorder is currently stopped.";
    }
}
