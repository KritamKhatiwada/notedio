#include "homepage.h"
#include "../widgets/notecard.h"
#include "../database/database.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

HomePage::HomePage(NotesRepository *repository, QWidget *parent)
    : QWidget(parent)
    , m_repository(repository)
{
    setStyleSheet("background-color: #1e1e1e;");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *header = new QWidget(this);
    header->setStyleSheet("background-color: #262626;");
    auto *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(20, 20, 20, 20);

    auto *newRecordingBtn = new QPushButton("+ New Recording", header);
    newRecordingBtn->setMinimumHeight(48);
    newRecordingBtn->setStyleSheet(
        "QPushButton { background-color: #D32F2F; color: white; font-weight: bold; "
        "font-size: 14px; border-radius: 8px; }"
        "QPushButton:hover { background-color: #b71c1c; }"
        );
    connect(newRecordingBtn, &QPushButton::clicked, this, &HomePage::newRecordingRequested);

    headerLayout->addWidget(newRecordingBtn);
    rootLayout->addWidget(header);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("border: none;");

    auto *notesContainer = new QWidget();
    m_notesLayout = new QVBoxLayout(notesContainer);
    m_notesLayout->setContentsMargins(16, 16, 16, 16);
    m_notesLayout->setSpacing(10);
    m_notesLayout->addStretch();

    m_scrollArea->setWidget(notesContainer);
    rootLayout->addWidget(m_scrollArea);

    refreshNotes();
}

void HomePage::refreshNotes()
{
    QLayoutItem *item;
    while ((item = m_notesLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const auto notes = m_repository->listNotes();
    qInfo() << "[HomePage] refreshNotes: loaded" << notes.size() << "notes";

    for (const auto &note : notes) {
        auto *card = new NoteCard(note, m_scrollArea->widget());
        connect(card, &NoteCard::clicked, this, &HomePage::noteOpened);
        connect(card, &NoteCard::summarizeRequested, this, &HomePage::summarizeRequested);
        connect(card, &NoteCard::deleteRequested, this, &HomePage::deleteNote);
        m_notesLayout->addWidget(card);
    }

    m_notesLayout->addStretch();
}

void HomePage::deleteNote(int noteId)
{
    const auto reply = QMessageBox::question(
        this,
        "Delete recording",
        "Delete this recording and its transcription? This can't be undone.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    if (m_repository->deleteNote(noteId))
        refreshNotes();
    else
        qWarning() << "[HomePage] failed to delete note" << noteId;
}