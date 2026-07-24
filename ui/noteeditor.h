#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include "notemanager.h"

class NoteEditorPage : public QWidget
{
    Q_OBJECT
public:
    explicit NoteEditorPage(NoteManager *noteManager, QWidget *parent = nullptr);
    void loadNote(const Note &note);

signals:
    void backRequested();

private:
    void setupToolbar(QVBoxLayout *layout);

    NoteManager *m_noteManager;
    QTextEdit *m_editor;
    Note m_currentNote;
};
