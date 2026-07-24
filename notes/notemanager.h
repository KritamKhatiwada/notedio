#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>

struct Note {
    QString id;
    QString subject;
    QString description;
    QDateTime timestamp;
    QString transcriptionPath;
};

class NoteManager : public QObject
{
    Q_OBJECT
public:
    explicit NoteManager(const QString &databaseDir, QObject *parent = nullptr);

    QVector<Note> loadNotes() const;
    Note createNote(const QString &subject, const QString &description, const QString &transcriptionText);
    void updateTranscription(const QString &id, const QString &newText);
    QString readTranscription(const Note &note) const;

private:
    QString notesIndexPath() const;
    void saveNotes(const QVector<Note> &notes) const;

    QString m_databaseDir;
};
