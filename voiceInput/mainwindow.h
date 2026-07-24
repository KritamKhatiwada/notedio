#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class RecordingSession;
class NotesRepository;
class QTimer;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(NotesRepository *repository, QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void backToHomeRequested();
    void noteReady(int noteId, bool hadFailures);
private slots:
    void onRecordClicked();
    void onPauseClicked();
    void onElapsedTick();
    void onFinalizingProgress(int completed, int total);
    void onNoteReady(int noteId, bool hadFailures);
private:
    void resetUiToIdle();
    void showPopup(const QString &title, const QString &message);
    QString formatElapsed(qint64 ms) const;
    Ui::MainWindow *ui;
    RecordingSession *m_session = nullptr;
    QTimer *m_uiTimer = nullptr;
};
#endif
