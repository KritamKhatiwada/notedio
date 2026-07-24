#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QDateTime>
#include <QVector>

struct NoteRecord {
    int id = -1;
    QString title;
    QString description;
    QString transcriptionHtml;
    QString audioPath;
    QDateTime createdAt;
    qint64 durationMs = 0;
};

class NotesRepository : public QObject
{
    Q_OBJECT
public:
    explicit NotesRepository(QObject *parent = nullptr);

    bool open(const QString &dbPath);
    void close();

    int createNote(const QString &title, const QString &audioPath, const QDateTime &createdAt);
    bool updateTranscription(int noteId, const QString &transcriptionHtml, const QString &description);
    bool updateDuration(int noteId, qint64 durationMs);
    bool updateTitle(int noteId, const QString &title);
    bool deleteNote(int noteId);

    QVector<NoteRecord> listNotes() const;
    NoteRecord getNote(int noteId) const;

private:
    bool ensureSchema();

    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif
