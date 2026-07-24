#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

class NotesRepository;
class QVBoxLayout;
class QScrollArea;

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(NotesRepository *repository, QWidget *parent = nullptr);

    void refreshNotes();

signals:
    void newRecordingRequested();
    void noteOpened(int noteId);
    void summarizeRequested(int noteId);

private slots:
    void deleteNote(int noteId);

private:
    NotesRepository *m_repository;
    QVBoxLayout *m_notesLayout;
    QScrollArea *m_scrollArea;
};

#endif