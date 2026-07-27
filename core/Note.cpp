#include "Note.h"

#include <QUuid>

Note::Note()
    : createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime())
{
}

QString Note::newId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QJsonObject Note::toJson() const
{
    QJsonObject object;
    object["id"] = id;
    object["title"] = title;
    object["richTextHtml"] = richTextHtml;
    object["rawTranscript"] = rawTranscript;
    object["summary"] = summary;
    object["createdAt"] = createdAt.toString(Qt::ISODate);
    object["updatedAt"] = updatedAt.toString(Qt::ISODate);
    return object;
}

Note Note::fromJson(const QJsonObject &object)
{
    Note note;
    note.id = object.value("id").toString();
    note.title = object.value("title").toString();
    note.richTextHtml = object.value("richTextHtml").toString();
    note.rawTranscript = object.value("rawTranscript").toString();
    note.summary = object.value("summary").toString();
    note.createdAt = QDateTime::fromString(object.value("createdAt").toString(), Qt::ISODate);
    note.updatedAt = QDateTime::fromString(object.value("updatedAt").toString(), Qt::ISODate);
    return note;
}
