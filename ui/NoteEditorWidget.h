#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QTimer>
#include "../core/Note.h"

class NoteEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteEditorWidget(const Note &note, QWidget *parent = nullptr);

    QString currentHtml() const;
    QString currentPlainText() const;

signals:
    void contentChanged(const QString &html);

private slots:
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void chooseTextColor();
    void chooseHighlightColor();
    void scheduleAutosave();
    void emitAutosave();

private:
    void buildToolbar();

    QString m_noteId;
    QToolBar *m_toolbar;
    QTextEdit *m_editor;
    QTimer m_autosaveTimer;
};
