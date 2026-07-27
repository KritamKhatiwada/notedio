#pragma once

#include <QObject>
#include <QVector>
#include <QSqlDatabase>
#include "Note.h"

class NoteStore : public QObject
{
    Q_OBJECT

public:
    explicit NoteStore(QObject *parent = nullptr);
    ~NoteStore();

    QVector<Note> allNotes() const;
    Note noteById(const QString &id) const;

    Note createNote(const QString &rawTranscript);
    void updateNote(const Note &note);
    void removeNote(const QString &id);

signals:
    void noteAdded(const Note &note);
    void noteUpdated(const Note &note);
    void noteRemoved(const QString &id);

private:
    bool ensureSchema();
    void loadAll();
    void persist(const Note &note);

    QSqlDatabase m_db;
    QString m_connectionName;
    QVector<Note> m_notes;
};

