#include "appwindow.h"
#include "../database/database.h"
#include "../ui/homepage.h"
#include "../ui/transcriptioneditor.h"
#include "../voiceInput/mainwindow.h"

#include <QStackedWidget>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(262, 500);
    setWindowTitle("notedio");

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    m_repository = new NotesRepository(this);
    m_repository->open(QDir(dataDir).filePath("notedio.db"));

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_homePage = new HomePage(m_repository, this);
    m_recordingPage = new MainWindow(m_repository, this);
    m_editorPage = new TranscriptionEditor(m_repository, this);

    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_recordingPage);
    m_stack->addWidget(m_editorPage);

    connect(m_homePage, &HomePage::newRecordingRequested, this, &AppWindow::showRecordingPage);
    connect(m_homePage, &HomePage::noteOpened, this, &AppWindow::showEditorPage);
    connect(m_recordingPage, &MainWindow::backToHomeRequested, this, &AppWindow::showHomePage);
    connect(m_recordingPage, &MainWindow::noteReady, this, [this](int, bool hadFailures) {
        m_homePage->refreshNotes();
        if (hadFailures) {
            QMessageBox::warning(this, "Transcription Incomplete",
                "The note was saved, but one or more audio chunks failed to transcribe. "
                "Part of the recording may be missing from the text.");
        } else {
            QMessageBox::information(this, "Note Saved", "New note created!");
        }
    });
    connect(m_editorPage, &TranscriptionEditor::backRequested, this, &AppWindow::showHomePage);

    showHomePage();
}

AppWindow::~AppWindow()
{
}

void AppWindow::showHomePage()
{
    m_homePage->refreshNotes();
    m_stack->setCurrentWidget(m_homePage);
}

void AppWindow::showRecordingPage()
{
    m_stack->setCurrentWidget(m_recordingPage);
}

void AppWindow::showEditorPage(int noteId)
{
    m_editorPage->loadNote(noteId);
    m_stack->setCurrentWidget(m_editorPage);
}
