#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QMediaCaptureSession *m_mediacapturesession = nullptr;
    QAudioInput *m_audioinput;
    QMediaRecorder *m_audiorecorder;

    ~MainWindow();

private slots:
    void onRecordClicked();
    void onPauseClicked();
private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
