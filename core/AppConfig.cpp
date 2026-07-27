#include "AppConfig.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFileInfo>

AppConfig &AppConfig::instance()
{
    static AppConfig config;
    return config;
}

AppConfig::AppConfig()
{
    load();
}

void AppConfig::load()
{
    const QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
    QFile file(configPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[AppConfig] could not open" << configPath << "- falling back to defaults";
        m_audioDirectory = "voiceInput";
        m_notesDirectory = "database/notes";
        m_chunkWorkDirectory = "voiceInput/chunks";
        m_chunkDurationSeconds = 20;
        m_whisperExecutable = "whisper.cpp/build/bin/whisper-cli";
        m_whisperModel = "whisper.cpp/models/ggml-large-v3-q5_0.bin";
        m_whisperThreads = 12;
        m_llamaExecutable = "llama.cpp/build/bin/llama-cli";
        m_llamaModel = "llama.cpp/models/model.gguf";
        m_llamaThreads = 12;
        m_summarizePromptTemplate = "Summarize:\n%1";
        return;
    }

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

    m_audioDirectory = root.value("audioDirectory").toString("voiceInput");
    m_notesDirectory = root.value("notesDirectory").toString("database/notes");
    m_chunkWorkDirectory = root.value("chunkWorkDirectory").toString("voiceInput/chunks");
    m_chunkDurationSeconds = root.value("chunkDurationSeconds").toInt(20);

    m_whisperExecutable = root.value("whisperExecutable").toString();
    m_whisperModel = root.value("whisperModel").toString();
    m_whisperThreads = root.value("whisperThreads").toInt(12);
    for (const QJsonValue &v : root.value("whisperExtraArgs").toArray())
        m_whisperExtraArgs << v.toString();

    m_llamaExecutable = root.value("llamaExecutable").toString();
    m_llamaModel = root.value("llamaModel").toString();
    m_llamaThreads = root.value("llamaThreads").toInt(12);
    for (const QJsonValue &v : root.value("llamaExtraArgs").toArray())
        m_llamaExtraArgs << v.toString();

    m_summarizePromptTemplate = root.value("summarizePromptTemplate").toString("Summarize:\n%1");

    qDebug() << "[AppConfig] loaded configuration from" << configPath;
}

QString AppConfig::resolvePath(const QString &relativeOrAbsolute) const
{
    QFileInfo info(relativeOrAbsolute);
    if (info.isAbsolute())
        return relativeOrAbsolute;
    return QDir(QCoreApplication::applicationDirPath()).filePath(relativeOrAbsolute);
}

QDir AppConfig::ensureDir(const QString &relativePath) const
{
    const QString resolved = resolvePath(relativePath);
    QDir dir(resolved);
    if (!dir.exists()) {
        QDir().mkpath(resolved);
        qDebug() << "[AppConfig] created directory" << resolved;
    }
    return dir;
}

QDir AppConfig::audioDir() const { return ensureDir(m_audioDirectory); }
QDir AppConfig::notesDir() const { return ensureDir(m_notesDirectory); }
QDir AppConfig::chunkWorkDir() const { return ensureDir(m_chunkWorkDirectory); }

int AppConfig::chunkDurationSeconds() const { return m_chunkDurationSeconds; }

QString AppConfig::whisperExecutable() const { return resolvePath(m_whisperExecutable); }
QString AppConfig::whisperModel() const { return resolvePath(m_whisperModel); }
int AppConfig::whisperThreads() const { return m_whisperThreads; }
QStringList AppConfig::whisperExtraArgs() const { return m_whisperExtraArgs; }

QString AppConfig::llamaExecutable() const { return resolvePath(m_llamaExecutable); }
QString AppConfig::llamaModel() const { return resolvePath(m_llamaModel); }
int AppConfig::llamaThreads() const { return m_llamaThreads; }
QStringList AppConfig::llamaExtraArgs() const { return m_llamaExtraArgs; }

QString AppConfig::summarizePromptTemplate() const { return m_summarizePromptTemplate; }
