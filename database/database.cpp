#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QUuid>
#include <QFileInfo>

NotesRepository::NotesRepository(QObject *parent)
    : QObject(parent)
{
    m_connectionName = QStringLiteral("notedio_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

bool NotesRepository::open(const QString &dbPath)
{
    qInfo() << "[DB] opening" << dbPath << "exists=" << QFileInfo::exists(dbPath);

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "[DB] FAILED to open database:" << m_db.lastError().text();
        return false;
    }

    qInfo() << "[DB] opened successfully, connection=" << m_connectionName;

    if (!ensureSchema()) {
        qWarning() << "[DB] FAILED to create/verify schema";
        return false;
    }

    qInfo() << "[DB] schema OK, note count=" << listNotes().size();
    return true;
}

void NotesRepository::close()
{
    if (m_db.isOpen()) {
        qInfo() << "[DB] closing";
        m_db.close();
    }
}

bool NotesRepository::ensureSchema()
{
    QSqlQuery query(m_db);
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS notes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT DEFAULT '',"
        "transcription_html TEXT DEFAULT '',"
        "audio_path TEXT DEFAULT '',"
        "created_at TEXT NOT NULL,"
        "duration_ms INTEGER DEFAULT 0"
        ")"
        );
    if (!ok)
        qWarning() << "[DB] ensureSchema query failed:" << query.lastError().text();
    return ok;
}

int NotesRepository::createNote(const QString &title, const QString &audioPath, const QDateTime &createdAt)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO notes (title, description, transcription_html, audio_path, created_at, duration_ms) "
        "VALUES (?, '', '', ?, ?, 0)"
        );
    query.addBindValue(title);
    query.addBindValue(audioPath);
    query.addBindValue(createdAt.toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "[DB] createNote FAILED:" << query.lastError().text();
        return -1;
    }

    const int id = query.lastInsertId().toInt();
    qInfo() << "[DB] createNote OK id=" << id << "title=" << title;
    return id;
}

bool NotesRepository::updateTranscription(int noteId, const QString &transcriptionHtml, const QString &description)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE notes SET transcription_html = ?, description = ? WHERE id = ?");
    query.addBindValue(transcriptionHtml);
    query.addBindValue(description);
    query.addBindValue(noteId);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "[DB] updateTranscription FAILED for note" << noteId << ":" << query.lastError().text();
    else
        qInfo() << "[DB] updateTranscription OK note=" << noteId
                << "htmlLen=" << transcriptionHtml.length()
                << "rowsAffected=" << query.numRowsAffected();
    return ok;
}

bool NotesRepository::updateDuration(int noteId, qint64 durationMs)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE notes SET duration_ms = ? WHERE id = ?");
    query.addBindValue(durationMs);
    query.addBindValue(noteId);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "[DB] updateDuration FAILED for note" << noteId << ":" << query.lastError().text();
    else
        qInfo() << "[DB] updateDuration OK note=" << noteId << "durationMs=" << durationMs;
    return ok;
}

bool NotesRepository::updateTitle(int noteId, const QString &title)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE notes SET title = ? WHERE id = ?");
    query.addBindValue(title);
    query.addBindValue(noteId);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "[DB] updateTitle FAILED for note" << noteId << ":" << query.lastError().text();
    return ok;
}

bool NotesRepository::deleteNote(int noteId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notes WHERE id = ?");
    query.addBindValue(noteId);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "[DB] deleteNote FAILED for note" << noteId << ":" << query.lastError().text();
    else
        qInfo() << "[DB] deleteNote OK note=" << noteId;
    return ok;
}

QVector<NoteRecord> NotesRepository::listNotes() const
{
    QVector<NoteRecord> results;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT id, title, description, transcription_html, audio_path, created_at, duration_ms FROM notes ORDER BY created_at DESC")) {
        qWarning() << "[DB] listNotes query FAILED:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        NoteRecord record;
        record.id = query.value(0).toInt();
        record.title = query.value(1).toString();
        record.description = query.value(2).toString();
        record.transcriptionHtml = query.value(3).toString();
        record.audioPath = query.value(4).toString();
        record.createdAt = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        record.durationMs = query.value(6).toLongLong();
        results.append(record);
    }

    return results;
}

NoteRecord NotesRepository::getNote(int noteId) const
{
    NoteRecord record;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, title, description, transcription_html, audio_path, created_at, duration_ms FROM notes WHERE id = ?");
    query.addBindValue(noteId);

    if (!query.exec()) {
        qWarning() << "[DB] getNote query FAILED for" << noteId << ":" << query.lastError().text();
        return record;
    }

    if (query.next()) {
        record.id = query.value(0).toInt();
        record.title = query.value(1).toString();
        record.description = query.value(2).toString();
        record.transcriptionHtml = query.value(3).toString();
        record.audioPath = query.value(4).toString();
        record.createdAt = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        record.durationMs = query.value(6).toLongLong();
    } else {
        qWarning() << "[DB] getNote: no row for id" << noteId;
    }

    return record;
}