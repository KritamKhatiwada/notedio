#include "NoteStore.h"
#include "AppConfig.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>
#include <algorithm>

NoteStore::NoteStore(QObject *parent)
    : QObject(parent)
{
    m_connectionName = QStringLiteral("notedio_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    const QString dbPath = AppConfig::instance().notesDir().filePath("notedio.db");

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "[NoteStore] FAILED to open database:" << m_db.lastError().text();
        return;
    }

    qDebug() << "[NoteStore] opened database at" << dbPath;

    if (!ensureSchema()) {
        qDebug() << "[NoteStore] FAILED to create/verify schema";
        return;
    }

    loadAll();
}

NoteStore::~NoteStore()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool NoteStore::ensureSchema()
{
    QSqlQuery query(m_db);
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS notes ("
        "id TEXT PRIMARY KEY,"
        "title TEXT NOT NULL,"
        "rich_text_html TEXT DEFAULT '',"
        "raw_transcript TEXT DEFAULT '',"
        "summary TEXT DEFAULT '',"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL"
        ")"
    );
    if (!ok)
        qDebug() << "[NoteStore] ensureSchema FAILED:" << query.lastError().text();
    return ok;
}

void NoteStore::loadAll()
{
    m_notes.clear();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT id, title, rich_text_html, raw_transcript, summary, created_at, updated_at "
                     "FROM notes ORDER BY created_at DESC")) {
        qDebug() << "[NoteStore] loadAll FAILED:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        Note note;
        note.id = query.value(0).toString();
        note.title = query.value(1).toString();
        note.richTextHtml = query.value(2).toString();
        note.rawTranscript = query.value(3).toString();
        note.summary = query.value(4).toString();
        note.createdAt = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        note.updatedAt = QDateTime::fromString(query.value(6).toString(), Qt::ISODate);
        m_notes.append(note);
    }

    qDebug() << "[NoteStore] loaded" << m_notes.size() << "notes from database";
}

QVector<Note> NoteStore::allNotes() const
{
    return m_notes;
}

Note NoteStore::noteById(const QString &id) const
{
    for (const Note &note : m_notes) {
        if (note.id == id)
            return note;
    }
    return Note();
}

void NoteStore::persist(const Note &note)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO notes (id, title, rich_text_html, raw_transcript, summary, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "title = excluded.title, "
        "rich_text_html = excluded.rich_text_html, "
        "raw_transcript = excluded.raw_transcript, "
        "summary = excluded.summary, "
        "updated_at = excluded.updated_at"
    );
    query.addBindValue(note.id);
    query.addBindValue(note.title);
    query.addBindValue(note.richTextHtml);
    query.addBindValue(note.rawTranscript);
    query.addBindValue(note.summary);
    query.addBindValue(note.createdAt.toString(Qt::ISODate));
    query.addBindValue(note.updatedAt.toString(Qt::ISODate));

    if (!query.exec())
        qDebug() << "[NoteStore] persist FAILED for note" << note.id << ":" << query.lastError().text();
    else
        qDebug() << "[NoteStore] persisted note" << note.id;
}

Note NoteStore::createNote(const QString &rawTranscript)
{
    Note note;
    note.id = Note::newId();
    note.title = QString("Note %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    note.rawTranscript = rawTranscript;
    note.richTextHtml = QString("<p>%1</p>").arg(rawTranscript.toHtmlEscaped());

    m_notes.prepend(note);
    persist(note);

    qDebug() << "[NoteStore] created note" << note.id;
    emit noteAdded(note);
    return note;
}

void NoteStore::updateNote(const Note &note)
{
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].id == note.id) {
            m_notes[i] = note;
            m_notes[i].updatedAt = QDateTime::currentDateTime();
            persist(m_notes[i]);
            emit noteUpdated(m_notes[i]);
            return;
        }
    }
}

void NoteStore::removeNote(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notes WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec())
        qDebug() << "[NoteStore] removeNote FAILED for" << id << ":" << query.lastError().text();

    m_notes.erase(std::remove_if(m_notes.begin(), m_notes.end(),
                                  [&id](const Note &n) { return n.id == id; }),
                  m_notes.end());

    qDebug() << "[NoteStore] removed note" << id;
    emit noteRemoved(id);
}
