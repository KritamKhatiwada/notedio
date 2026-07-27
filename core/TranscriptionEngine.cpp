#include "TranscriptionEngine.h"
#include "AppConfig.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

TranscriptionEngine::TranscriptionEngine(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_busy(false)
    , m_nextExpectedIndex(0)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TranscriptionEngine::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &TranscriptionEngine::onProcessErrorOccurred);
}

void TranscriptionEngine::onProcessErrorOccurred(QProcess::ProcessError error)
{
    static const QMap<QProcess::ProcessError, QString> reasons = {
        { QProcess::FailedToStart, "failed to start - check whisperExecutable path in config.json" },
        { QProcess::Crashed, "crashed" },
        { QProcess::Timedout, "timed out" },
        { QProcess::WriteError, "write error" },
        { QProcess::ReadError, "read error" },
        { QProcess::UnknownError, "unknown error" }
    };

    const QString reason = reasons.value(error, "unrecognized error");
    qDebug() << "[TranscriptionEngine] chunk" << m_current.chunkIndex << "process error:" << reason;
    emit chunkFailed(m_current.chunkIndex, reason);

    if (error == QProcess::FailedToStart)
        processNext();
}

void TranscriptionEngine::reset()
{
    m_queue.clear();
    m_transcribedChunks.clear();
    m_nextExpectedIndex = 0;
    qDebug() << "[TranscriptionEngine] reset for new recording session";
}

QString TranscriptionEngine::outputTextPathFor(const QString &wavPath) const
{
    QFileInfo info(wavPath);
    return info.absolutePath() + "/" + info.completeBaseName();
}

void TranscriptionEngine::enqueueChunk(const QString &filePath, int chunkIndex, bool isFinal)
{
    m_queue.enqueue({ filePath, chunkIndex, isFinal });
    qDebug() << "[TranscriptionEngine] queued chunk" << chunkIndex << "(" << filePath << ")"
             << "queue depth" << m_queue.size();
    if (!m_busy)
        processNext();
}

void TranscriptionEngine::processNext()
{
    if (m_queue.isEmpty()) {
        m_busy = false;
        return;
    }

    m_busy = true;
    m_current = m_queue.dequeue();

    const AppConfig &config = AppConfig::instance();
    const QString outputPrefix = outputTextPathFor(m_current.filePath);

    QStringList arguments;
    arguments << "-m" << config.whisperModel()
              << "-f" << m_current.filePath
              << "-t" << QString::number(config.whisperThreads())
              << "-otxt" << "-of" << outputPrefix
              << config.whisperExtraArgs();

    qDebug() << "[TranscriptionEngine] transcribing chunk" << m_current.chunkIndex
             << "->" << config.whisperExecutable() << arguments.join(' ');

    m_process->start(config.whisperExecutable(), arguments);
}

void TranscriptionEngine::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit || exitCode != 0) {
        qDebug() << "[TranscriptionEngine] chunk" << m_current.chunkIndex << "failed, exit code" << exitCode;
        emit chunkFailed(m_current.chunkIndex, QString("whisper-cli exit code %1").arg(exitCode));
        processNext();
        return;
    }

    const QString textPath = outputTextPathFor(m_current.filePath) + ".txt";
    QFile textFile(textPath);
    QString text;
    if (textFile.open(QIODevice::ReadOnly | QIODevice::Text))
        text = QString::fromUtf8(textFile.readAll()).trimmed();
    else
        qDebug() << "[TranscriptionEngine] could not read output" << textPath;

    m_transcribedChunks.insert(m_current.chunkIndex, text);
    qDebug() << "[TranscriptionEngine] chunk" << m_current.chunkIndex << "transcribed, length" << text.length();

    emitAssembledText();

    if (m_current.isFinal) {
        QString fullText;
        for (auto it = m_transcribedChunks.constBegin(); it != m_transcribedChunks.constEnd(); ++it)
            fullText += it.value() + " ";
        fullText = fullText.trimmed();
        qDebug() << "[TranscriptionEngine] transcript finalized, total length" << fullText.length();
        emit transcriptFinalized(fullText);
    }

    processNext();
}

void TranscriptionEngine::emitAssembledText()
{
    QString assembled;
    while (m_transcribedChunks.contains(m_nextExpectedIndex)) {
        assembled += m_transcribedChunks.value(m_nextExpectedIndex) + " ";
        m_nextExpectedIndex++;
    }

    if (!assembled.isEmpty()) {
        QString runningText;
        for (int i = 0; i < m_nextExpectedIndex; ++i)
            runningText += m_transcribedChunks.value(i) + " ";
        emit transcriptProgress(runningText.trimmed());
    }
}
