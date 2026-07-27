#include "MainWindow.h"
#include "RecordingWindow.h"

#include <QVBoxLayout>
#include <QWidget>
#include <QListWidgetItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_noteStore(new NoteStore(this))
    , m_summarizer(new SummarizationEngine(this))
    , m_noteList(new QListWidget(this))
    , m_newRecordingButton(new QPushButton("+ New Recording", this))
{
    setWindowTitle("Notedio");
    resize(720, 800);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    m_newRecordingButton->setStyleSheet("padding: 10px; font-weight: bold;");
    layout->addWidget(m_newRecordingButton);
    layout->addWidget(m_noteList);
    setCentralWidget(central);

    connect(m_newRecordingButton, &QPushButton::clicked, this, &MainWindow::onNewRecordingClicked);
    connect(m_noteStore, &NoteStore::noteAdded, this, &MainWindow::onNoteAdded);
    connect(m_summarizer, &SummarizationEngine::summaryReady, this, &MainWindow::onSummaryReady);
    connect(m_summarizer, &SummarizationEngine::summaryFailed, this, [](const QString &noteId, const QString &reason) {
        qDebug() << "[MainWindow] summary failed for" << noteId << ":" << reason;
    });

    for (const Note &note : m_noteStore->allNotes())
        addNoteToList(note);

    qDebug() << "[MainWindow] initialized with" << m_noteStore->allNotes().size() << "existing notes";
}

void MainWindow::onNewRecordingClicked()
{
    auto *recordingWindow = new RecordingWindow(this);
    connect(recordingWindow, &RecordingWindow::recordingFinished, this, [this](const QString &transcript) {
        m_noteStore->createNote(transcript);
    });
    recordingWindow->setAttribute(Qt::WA_DeleteOnClose);
    recordingWindow->exec();
}

void MainWindow::addNoteToList(const Note &note)
{
    auto *item = new QListWidgetItem(m_noteList);
    auto *widget = new NoteListItemWidget(note, m_noteList);

    item->setSizeHint(widget->sizeHint());
    m_noteList->setItemWidget(item, widget);
    m_widgetsByNoteId.insert(note.id, widget);

    connect(widget, &NoteListItemWidget::editorContentChanged, this, &MainWindow::onEditorContentChanged);
    connect(widget, &NoteListItemWidget::summaryContentChanged, this, &MainWindow::onSummaryContentChanged);
    connect(widget, &NoteListItemWidget::summarizeRequested, this, &MainWindow::onSummarizeRequested);
    connect(widget, &NoteListItemWidget::deleteRequested, this, &MainWindow::onDeleteRequested);
}

void MainWindow::onNoteAdded(const Note &note)
{
    qDebug() << "[MainWindow] note added to list" << note.id;
    auto *item = new QListWidgetItem;
    auto *widget = new NoteListItemWidget(note, m_noteList);
    item->setSizeHint(widget->sizeHint());
    m_noteList->insertItem(0, item);
    m_noteList->setItemWidget(item, widget);
    m_widgetsByNoteId.insert(note.id, widget);

    connect(widget, &NoteListItemWidget::editorContentChanged, this, &MainWindow::onEditorContentChanged);
    connect(widget, &NoteListItemWidget::summaryContentChanged, this, &MainWindow::onSummaryContentChanged);
    connect(widget, &NoteListItemWidget::summarizeRequested, this, &MainWindow::onSummarizeRequested);
    connect(widget, &NoteListItemWidget::deleteRequested, this, &MainWindow::onDeleteRequested);
}

void MainWindow::onEditorContentChanged(const QString &noteId, const QString &html)
{
    Note note = m_noteStore->noteById(noteId);
    if (note.id.isEmpty())
        return;
    note.richTextHtml = html;
    m_noteStore->updateNote(note);
}

void MainWindow::onSummaryContentChanged(const QString &noteId, const QString &text)
{
    Note note = m_noteStore->noteById(noteId);
    if (note.id.isEmpty())
        return;
    note.summary = text;
    m_noteStore->updateNote(note);
}

void MainWindow::onSummarizeRequested(const QString &noteId, const QString &transcript)
{
    if (m_widgetsByNoteId.contains(noteId))
        m_widgetsByNoteId[noteId]->setSummarizing(true);
    m_summarizer->summarize(noteId, transcript);
}

void MainWindow::onSummaryReady(const QString &noteId, const QString &summary)
{
    Note note = m_noteStore->noteById(noteId);
    if (!note.id.isEmpty()) {
        note.summary = summary;
        m_noteStore->updateNote(note);
    }

    if (m_widgetsByNoteId.contains(noteId))
        m_widgetsByNoteId[noteId]->setSummary(summary);
}

void MainWindow::onDeleteRequested(const QString &noteId)
{
    qDebug() << "[MainWindow] deleting note" << noteId;

    for (int row = 0; row < m_noteList->count(); ++row) {
        QListWidgetItem *item = m_noteList->item(row);
        if (m_noteList->itemWidget(item) == m_widgetsByNoteId.value(noteId)) {
            delete m_noteList->takeItem(row);
            break;
        }
    }

    m_widgetsByNoteId.remove(noteId);
    m_noteStore->removeNote(noteId);
}
