#ifndef TRANSCRIPTIONEDITOR_H
#define TRANSCRIPTIONEDITOR_H

#include <QWidget>

class NotesRepository;
class QTextEdit;
class QLineEdit;

class TranscriptionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit TranscriptionEditor(NotesRepository *repository, QWidget *parent = nullptr);

    void loadNote(int noteId);

signals:
    void backRequested();

private slots:
    void onBoldToggled();
    void onItalicToggled();
    void onUnderlineToggled();
    void saveCurrentNote();

private:
    NotesRepository *m_repository;
    int m_noteId = -1;
    QLineEdit *m_titleEdit;
    QTextEdit *m_textEdit;
};

#endif
