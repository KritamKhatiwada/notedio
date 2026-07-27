#pragma once

#include <QString>
#include <QStringList>
#include <QDir>

class AppConfig
{
public:
    static AppConfig &instance();

    QDir audioDir() const;
    QDir notesDir() const;
    QDir chunkWorkDir() const;

    int chunkDurationSeconds() const;

    QString whisperExecutable() const;
    QString whisperModel() const;
    int whisperThreads() const;
    QStringList whisperExtraArgs() const;

    QString llamaExecutable() const;
    QString llamaModel() const;
    int llamaThreads() const;
    QStringList llamaExtraArgs() const;

    QString summarizePromptTemplate() const;

private:
    AppConfig();
    void load();
    QString resolvePath(const QString &relativeOrAbsolute) const;
    QDir ensureDir(const QString &relativePath) const;

    QString m_audioDirectory;
    QString m_notesDirectory;
    QString m_chunkWorkDirectory;
    int m_chunkDurationSeconds;

    QString m_whisperExecutable;
    QString m_whisperModel;
    int m_whisperThreads;
    QStringList m_whisperExtraArgs;

    QString m_llamaExecutable;
    QString m_llamaModel;
    int m_llamaThreads;
    QStringList m_llamaExtraArgs;

    QString m_summarizePromptTemplate;
};
