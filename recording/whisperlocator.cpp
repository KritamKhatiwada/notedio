#include "whisperlocator.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace WhisperLocator
{

QString findWhisperExe(const QString &appDir)
{
#ifdef Q_OS_WIN
    const QString exeName = "whisper-cli.exe";
#else
    const QString exeName = "whisper-cli";
#endif

    const QStringList candidates = {
        QDir(appDir).filePath(exeName),
        QDir(appDir).filePath("bin/" + exeName),
        QDir(appDir).filePath("whisper.cpp/" + exeName),
        QDir(appDir).filePath("whisper.cpp/bin/" + exeName),
        QDir(appDir).filePath("whisper.cpp/build/bin/" + exeName),
        QDir(appDir).filePath("whisper.cpp/build/bin/Release/" + exeName),
        QDir(appDir).filePath("whisper.cpp/build/bin/Debug/" + exeName),
        QDir(appDir).filePath("whisper.cpp/build/examples/cli/" + exeName),
    };

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            qInfo() << "[WhisperLocator] found whisper-cli at" << path;
            return path;
        }
    }

    qWarning() << "[WhisperLocator] whisper-cli NOT FOUND. Checked:";
    for (const QString &path : candidates)
        qWarning() << "  -" << path;

    return QString();
}

QString findModel(const QString &appDir, const QString &modelFileName)
{
    QStringList candidates = {
        QDir(appDir).filePath("models/" + modelFileName),
        QDir(appDir).filePath("whisper.cpp/models/" + modelFileName),
    };

#ifdef NOTEDIO_SOURCE_DIR
    candidates << QDir(QStringLiteral(NOTEDIO_SOURCE_DIR)).filePath("whisper.cpp/models/" + modelFileName);
#endif

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            qInfo() << "[WhisperLocator] found model at" << path;
            return path;
        }
    }

    qWarning() << "[WhisperLocator] model NOT FOUND. Checked:";
    for (const QString &path : candidates)
        qWarning() << "  -" << path;

    return QString();
}

}
