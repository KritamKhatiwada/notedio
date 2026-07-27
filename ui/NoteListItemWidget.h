#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>
#include "../core/Note.h"
#include "NoteEditorWidget.h"

class NoteListItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteListItemWidget(const Note &note, QWidget *parent = nullptr);

    QString noteId() const;

signals:
    void editorContentChanged(const QString &noteId, const QString &html);
    void summaryContentChanged(const QString &noteId, const QString &text);
    void summarizeRequested(const QString &noteId, const QString &transcript);
    void deleteRequested(const QString &noteId);

public slots:
    void setSummary(const QString &summary);
    void setSummarizing(bool active);

private slots:
    void scheduleSummaryAutosave();
    void emitSummaryAutosave();

private:
    QString m_noteId;
    QString m_transcript;
    NoteEditorWidget *m_editor;
    QPushButton *m_summarizeButton;
    QPushButton *m_deleteButton;
    QTextEdit *m_summaryEdit;
    QTimer m_summaryAutosaveTimer;
};
