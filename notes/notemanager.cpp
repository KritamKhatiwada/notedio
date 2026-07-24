#include "notemanager.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>

NoteManager::NoteManager(const QString &databaseDir, QObject *parent)
    : QObject(parent), m_databaseDir(databaseDir)
{
    QDir dir(m_databaseDir);
    if (!dir.exists())
        QDir().mkpath(m_databaseDir);
}

QString NoteManager::notesIndexPath() const
{
    return m_databaseDir + "/notes.json";
}

QVector<Note> NoteManager::loadNotes() const
{
    QVector<Note> result;
    QFile f(notesIndexPath());
    if (!f.open(QIODevice::ReadOnly))
        return result;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    for (const QJsonValue &v : doc.array()) {
        QJsonObject o = v.toObject();
        Note n;
        n.id = o["id"].toString();
        n.subject = o["subject"].toString();
        n.description = o["description"].toString();
        n.timestamp = QDateTime::fromString(o["timestamp"].toString(), Qt::ISODate);
        n.transcriptionPath = o["transcriptionPath"].toString();
        result.append(n);
    }
    return result;
}

void NoteManager::saveNotes(const QVector<Note> &notes) const
{
    QJsonArray arr;
    for (const Note &n : notes) {
        QJsonObject o;
        o["id"] = n.id;
        o["subject"] = n.subject;
        o["description"] = n.description;
        o["timestamp"] = n.timestamp.toString(Qt::ISODate);
        o["transcriptionPath"] = n.transcriptionPath;
        arr.append(o);
    }

    QFile f(notesIndexPath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
        f.close();
    }
}

Note NoteManager::createNote(const QString &subject, const QString &description, const QString &transcriptionText)
{
    Note n;
    n.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    n.subject = subject;
    n.description = description;
    n.timestamp = QDateTime::currentDateTime();
    n.transcriptionPath = m_databaseDir + "/" + n.id + ".txt";

    QFile txt(n.transcriptionPath);
    if (txt.open(QIODevice::WriteOnly | QIODevice::Text)) {
        txt.write(transcriptionText.toUtf8());
        txt.close();
    }

    QVector<Note> notes = loadNotes();
    notes.append(n);
    saveNotes(notes);
    return n;
}

void NoteManager::updateTranscription(const QString &id, const QString &newText)
{
    for (const Note &n : loadNotes()) {
        if (n.id == id) {
            QFile f(n.transcriptionPath);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                f.write(newText.toUtf8());
                f.close();
            }
            return;
        }
    }
}

QString NoteManager::readTranscription(const Note &note) const
{
    QFile f(note.transcriptionPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QString content = QString::fromUtf8(f.readAll());
    f.close();
    return content;
}
