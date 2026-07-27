#include "NoteEditorWidget.h"

#include <QVBoxLayout>
#include <QAction>
#include <QColorDialog>
#include <QTextCharFormat>
#include <QIcon>
#include <QDebug>

NoteEditorWidget::NoteEditorWidget(const Note &note, QWidget *parent)
    : QWidget(parent)
    , m_noteId(note.id)
    , m_toolbar(new QToolBar(this))
    , m_editor(new QTextEdit(this))
{
    m_editor->setAcceptRichText(true);
    m_editor->setHtml(note.richTextHtml);

    buildToolbar();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_toolbar);
    layout->addWidget(m_editor);

    m_autosaveTimer.setSingleShot(true);
    m_autosaveTimer.setInterval(800);
    connect(&m_autosaveTimer, &QTimer::timeout, this, &NoteEditorWidget::emitAutosave);
    connect(m_editor, &QTextEdit::textChanged, this, &NoteEditorWidget::scheduleAutosave);
}

void NoteEditorWidget::buildToolbar()
{
    auto *boldAction = m_toolbar->addAction(QIcon::fromTheme("format-text-bold"), "Bold");
    boldAction->setCheckable(true);
    connect(boldAction, &QAction::triggered, this, &NoteEditorWidget::toggleBold);

    auto *italicAction = m_toolbar->addAction(QIcon::fromTheme("format-text-italic"), "Italic");
    italicAction->setCheckable(true);
    connect(italicAction, &QAction::triggered, this, &NoteEditorWidget::toggleItalic);

    auto *underlineAction = m_toolbar->addAction(QIcon::fromTheme("format-text-underline"), "Underline");
    underlineAction->setCheckable(true);
    connect(underlineAction, &QAction::triggered, this, &NoteEditorWidget::toggleUnderline);

    auto *colorAction = m_toolbar->addAction(QIcon::fromTheme("format-text-color"), "Text Color");
    connect(colorAction, &QAction::triggered, this, &NoteEditorWidget::chooseTextColor);

    auto *highlightAction = m_toolbar->addAction(QIcon::fromTheme("format-fill-color"), "Highlight");
    connect(highlightAction, &QAction::triggered, this, &NoteEditorWidget::chooseHighlightColor);
}

void NoteEditorWidget::toggleBold()
{
    QTextCharFormat format;
    format.setFontWeight(m_editor->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    m_editor->mergeCurrentCharFormat(format);
}

void NoteEditorWidget::toggleItalic()
{
    QTextCharFormat format;
    format.setFontItalic(!m_editor->fontItalic());
    m_editor->mergeCurrentCharFormat(format);
}

void NoteEditorWidget::toggleUnderline()
{
    QTextCharFormat format;
    format.setFontUnderline(!m_editor->fontUnderline());
    m_editor->mergeCurrentCharFormat(format);
}

void NoteEditorWidget::chooseTextColor()
{
    const QColor color = QColorDialog::getColor(m_editor->textColor(), this, "Select Text Color");
    if (!color.isValid())
        return;
    QTextCharFormat format;
    format.setForeground(color);
    m_editor->mergeCurrentCharFormat(format);
}

void NoteEditorWidget::chooseHighlightColor()
{
    const QColor color = QColorDialog::getColor(Qt::yellow, this, "Select Highlight Color");
    if (!color.isValid())
        return;
    QTextCharFormat format;
    format.setBackground(color);
    m_editor->mergeCurrentCharFormat(format);
}

void NoteEditorWidget::scheduleAutosave()
{
    m_autosaveTimer.start();
}

void NoteEditorWidget::emitAutosave()
{
    qDebug() << "[NoteEditorWidget] autosaving note" << m_noteId;
    emit contentChanged(m_editor->toHtml());
}

QString NoteEditorWidget::currentHtml() const
{
    return m_editor->toHtml();
}

QString NoteEditorWidget::currentPlainText() const
{
    return m_editor->toPlainText();
}
