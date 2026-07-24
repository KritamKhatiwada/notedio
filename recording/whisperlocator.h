#ifndef WHISPERLOCATOR_H
#define WHISPERLOCATOR_H

#include <QString>

namespace WhisperLocator
{
    QString findWhisperExe(const QString &appDir);
    QString findModel(const QString &appDir, const QString &modelFileName = "ggml-large-v3-q5_0.bin");
}

#endif
