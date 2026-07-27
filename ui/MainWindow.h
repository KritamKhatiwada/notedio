#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QMap>
#include "../core/NoteStore.h"
#include "../core/SummarizationEngine.h"
#include "NoteListItemWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onNewRecordingClicked();
    void onNoteAdded(const Note &note);
    void onEditorContentChanged(const QString &noteId, const QString &html);
    void onSummaryContentChanged(const QString &noteId, const QString &text);
    void onSummarizeRequested(const QString &noteId, const QString &transcript);
    void onSummaryReady(const QString &noteId, const QString &summary);
    void onDeleteRequested(const QString &noteId);

private:
    void addNoteToList(const Note &note);

    NoteStore *m_noteStore;
    SummarizationEngine *m_summarizer;

    QListWidget *m_noteList;
    QPushButton *m_newRecordingButton;

    QMap<QString, NoteListItemWidget *> m_widgetsByNoteId;
};
