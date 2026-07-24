#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class NotesRepository;
class HomePage;
class MainWindow;
class TranscriptionEditor;

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

private slots:
    void showHomePage();
    void showRecordingPage();
    void showEditorPage(int noteId);

private:
    NotesRepository *m_repository;
    QStackedWidget *m_stack;
    HomePage *m_homePage;
    MainWindow *m_recordingPage;
    TranscriptionEditor *m_editorPage;
};

#endif
