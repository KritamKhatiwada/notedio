#include "notecard.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>

NoteCard::NoteCard(const NoteRecord &note, QWidget *parent)
    : QWidget(parent)
    , m_noteId(note.id)
{
    setCursor(Qt::PointingHandCursor);
    setObjectName("noteCard");
    setStyleSheet(
        "#noteCard { background-color: #2b2b2b; border-radius: 10px; }"
        "#noteCard:hover { background-color: #353535; }"
        );

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setStyleSheet("color: #bbbbbb; font-size: 12px;");
    m_descriptionLabel->setWordWrap(true);

    m_timestampLabel = new QLabel(this);
    m_timestampLabel->setStyleSheet("color: #888888; font-size: 11px;");

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_descriptionLabel);
    layout->addWidget(m_timestampLabel);

    auto *footerLayout = new QHBoxLayout();

    m_deleteButton = new QPushButton("Delete", this);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setStyleSheet(
        "QPushButton { background-color: #3a2323; color: #e08080; font-size: 11px; "
        "border-radius: 6px; padding: 4px 10px; }"
        "QPushButton:hover { background-color: #5a2b2b; color: white; }"
        );
    connect(m_deleteButton, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_noteId);
    });
    footerLayout->addWidget(m_deleteButton);

    footerLayout->addStretch();

    m_summarizeButton = new QPushButton("Summarize", this);
    m_summarizeButton->setCursor(Qt::PointingHandCursor);
    m_summarizeButton->setStyleSheet(
        "QPushButton { background-color: #3a3a3a; color: white; font-size: 11px; "
        "border-radius: 6px; padding: 4px 10px; }"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:disabled { color: #666666; background-color: #2a2a2a; }"
        );
    connect(m_summarizeButton, &QPushButton::clicked, this, [this]() {
        emit summarizeRequested(m_noteId);
    });
    footerLayout->addWidget(m_summarizeButton);

    layout->addLayout(footerLayout);

    updateContent(note);
}

void NoteCard::updateContent(const NoteRecord &note)
{
    m_titleLabel->setText(note.title.isEmpty() ? "Untitled recording" : note.title);

    const bool stillTranscribing = note.description.isEmpty() && note.transcriptionHtml.isEmpty();
    m_descriptionLabel->setText(stillTranscribing ? "Transcribing…" : note.description);
    m_summarizeButton->setEnabled(!stillTranscribing);

    m_timestampLabel->setText(note.createdAt.toString("MMM d, yyyy \u00b7 hh:mm"));
}

void NoteCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_noteId);
    QWidget::mousePressEvent(event);
}