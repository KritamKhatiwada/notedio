#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class Note
{
public:
    Note();

    QString id;
    QString title;
    QString richTextHtml;
    QString rawTranscript;
    QString summary;
    QDateTime createdAt;
    QDateTime updatedAt;

    QJsonObject toJson() const;
    static Note fromJson(const QJsonObject &object);
    static QString newId();
};
