#include "transcriptioneditor.h"
#include "../database/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCharFormat>
#include <QFont>

TranscriptionEditor::TranscriptionEditor(NotesRepository *repository, QWidget *parent)
    : QWidget(parent)
    , m_repository(repository)
{
    setStyleSheet("background-color: #1e1e1e;");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(10);

    auto *topBar = new QHBoxLayout();
    auto *backBtn = new QPushButton("\u2190 Back", this);
    backBtn->setStyleSheet("color: white; background-color: #333; border-radius: 6px; padding: 6px 12px;");
    connect(backBtn, &QPushButton::clicked, this, &TranscriptionEditor::backRequested);
    topBar->addWidget(backBtn);
    topBar->addStretch();
    rootLayout->addLayout(topBar);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setStyleSheet("color: white; font-size: 20px; font-weight: bold; background: transparent; border: none;");
    rootLayout->addWidget(m_titleEdit);

    auto *toolbar = new QToolBar(this);
    auto *boldAction = toolbar->addAction("B");
    auto *italicAction = toolbar->addAction("I");
    auto *underlineAction = toolbar->addAction("U");
    connect(boldAction, &QAction::triggered, this, &TranscriptionEditor::onBoldToggled);
    connect(italicAction, &QAction::triggered, this, &TranscriptionEditor::onItalicToggled);
    connect(underlineAction, &QAction::triggered, this, &TranscriptionEditor::onUnderlineToggled);
    rootLayout->addWidget(toolbar);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setStyleSheet("color: white; background-color: #262626; border-radius: 8px; padding: 10px;");
    rootLayout->addWidget(m_textEdit, 1);

    auto *saveBtn = new QPushButton("Save", this);
    saveBtn->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold; border-radius: 6px; padding: 10px;");
    connect(saveBtn, &QPushButton::clicked, this, &TranscriptionEditor::saveCurrentNote);
    rootLayout->addWidget(saveBtn);
}

void TranscriptionEditor::loadNote(int noteId)
{
    m_noteId = noteId;
    const NoteRecord note = m_repository->getNote(noteId);
    m_titleEdit->setText(note.title);
    m_textEdit->setHtml(note.transcriptionHtml);
}

void TranscriptionEditor::onBoldToggled()
{
    QTextCharFormat format;
    format.setFontWeight(m_textEdit->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    m_textEdit->mergeCurrentCharFormat(format);
}

void TranscriptionEditor::onItalicToggled()
{
    QTextCharFormat format;
    format.setFontItalic(!m_textEdit->fontItalic());
    m_textEdit->mergeCurrentCharFormat(format);
}

void TranscriptionEditor::onUnderlineToggled()
{
    QTextCharFormat format;
    format.setFontUnderline(!m_textEdit->fontUnderline());
    m_textEdit->mergeCurrentCharFormat(format);
}

void TranscriptionEditor::saveCurrentNote()
{
    if (m_noteId < 0)
        return;

    m_repository->updateTitle(m_noteId, m_titleEdit->text());
    m_repository->updateTranscription(m_noteId, m_textEdit->toHtml(), m_textEdit->toPlainText().left(160));
}
