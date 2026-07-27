#include "NoteListItemWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QSignalBlocker>

NoteListItemWidget::NoteListItemWidget(const Note &note, QWidget *parent)
    : QWidget(parent)
    , m_noteId(note.id)
    , m_transcript(note.rawTranscript)
    , m_editor(new NoteEditorWidget(note, this))
    , m_summarizeButton(new QPushButton("Summarize", this))
    , m_deleteButton(new QPushButton("Delete", this))
    , m_summaryEdit(new QTextEdit(this))
{
    m_summaryEdit->setAcceptRichText(false);
    m_summaryEdit->setPlainText(note.summary);
    m_summaryEdit->setPlaceholderText("Summary will appear here once generated...");
    m_summaryEdit->setStyleSheet(
        "QTextEdit { color: white; background-color: #262626; border-radius: 8px; padding: 10px; }"
    );
    m_summaryEdit->setMinimumHeight(80);
    m_summaryEdit->setVisible(!note.summary.isEmpty());

    m_deleteButton->setStyleSheet("color: #D32F2F;");

    auto *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);

    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->addWidget(m_editor);

    auto *actionsRow = new QHBoxLayout;
    actionsRow->addStretch();
    actionsRow->addWidget(m_summarizeButton);
    actionsRow->addWidget(m_deleteButton);
    frameLayout->addLayout(actionsRow);

    auto *summaryHeader = new QLabel("Summary", frame);
    summaryHeader->setStyleSheet("color: #aaaaaa; font-weight: bold; margin-top: 4px;");
    summaryHeader->setVisible(!note.summary.isEmpty());
    connect(this, &NoteListItemWidget::summarizeRequested, this, [summaryHeader]() {
        summaryHeader->setVisible(true);
    });
    frameLayout->addWidget(summaryHeader);
    frameLayout->addWidget(m_summaryEdit);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->addWidget(frame);

    m_summaryAutosaveTimer.setSingleShot(true);
    m_summaryAutosaveTimer.setInterval(800);
    connect(&m_summaryAutosaveTimer, &QTimer::timeout, this, &NoteListItemWidget::emitSummaryAutosave);
    connect(m_summaryEdit, &QTextEdit::textChanged, this, &NoteListItemWidget::scheduleSummaryAutosave);

    connect(m_editor, &NoteEditorWidget::contentChanged, this, [this](const QString &html) {
        emit editorContentChanged(m_noteId, html);
    });

    connect(m_summarizeButton, &QPushButton::clicked, this, [this]() {
        const QString currentTranscript = m_editor->currentPlainText();
        qDebug() << "[NoteListItemWidget] summarize requested for note" << m_noteId
                 << "transcript length" << currentTranscript.length();
        emit summarizeRequested(m_noteId, currentTranscript);
    });

    connect(m_deleteButton, &QPushButton::clicked, this, [this]() {
        const auto choice = QMessageBox::question(this, "Delete Note",
                                                  "Delete this note permanently? This cannot be undone.",
                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes)
            return;
        qDebug() << "[NoteListItemWidget] delete requested for note" << m_noteId;
        emit deleteRequested(m_noteId);
    });
}

QString NoteListItemWidget::noteId() const
{
    return m_noteId;
}

void NoteListItemWidget::setSummary(const QString &summary)
{
    const QSignalBlocker blocker(m_summaryEdit);
    m_summaryEdit->setPlainText(summary);
    m_summaryEdit->setVisible(!summary.isEmpty());
    m_summarizeButton->setEnabled(true);
    m_summarizeButton->setText("Summarize");
}

void NoteListItemWidget::setSummarizing(bool active)
{
    m_summarizeButton->setEnabled(!active);
    m_summarizeButton->setText(active ? "Summarizing..." : "Summarize");
}

void NoteListItemWidget::scheduleSummaryAutosave()
{
    m_summaryAutosaveTimer.start();
}

void NoteListItemWidget::emitSummaryAutosave()
{
    qDebug() << "[NoteListItemWidget] autosaving summary for note" << m_noteId;
    emit summaryContentChanged(m_noteId, m_summaryEdit->toPlainText());
}
