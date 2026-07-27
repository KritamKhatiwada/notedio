#include "SummarizationEngine.h"
#include "AppConfig.h"

#include <QDebug>

SummarizationEngine::SummarizationEngine(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_busy(false)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SummarizationEngine::onProcessFinished);

    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.setInterval(90000);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &SummarizationEngine::onTimeout);
}

void SummarizationEngine::onTimeout()
{
    qDebug() << "[SummarizationEngine] note" << m_current.noteId << "timed out, killing process";
    m_process->kill();
}

void SummarizationEngine::summarize(const QString &noteId, const QString &transcript)
{
    m_queue.enqueue({ noteId, transcript });
    qDebug() << "[SummarizationEngine] queued summary request for note" << noteId;
    if (!m_busy)
        processNext();
}

void SummarizationEngine::processNext()
{
    if (m_queue.isEmpty()) {
        m_busy = false;
        return;
    }

    m_busy = true;
    m_current = m_queue.dequeue();

    const AppConfig &config = AppConfig::instance();
    m_currentPrompt = config.summarizePromptTemplate().arg(m_current.transcript);

    QStringList arguments;
    arguments << "-m" << config.llamaModel()
              << "-t" << QString::number(config.llamaThreads())
              << "-p" << m_currentPrompt
              << config.llamaExtraArgs();

    qDebug() << "[SummarizationEngine] summarizing note" << m_current.noteId << "via" << config.llamaExecutable();

    m_process->start(config.llamaExecutable(), arguments);
    m_process->closeWriteChannel();
    m_timeoutTimer.start();
}

void SummarizationEngine::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    m_timeoutTimer.stop();

    if (status != QProcess::NormalExit || exitCode != 0) {
        qDebug() << "[SummarizationEngine] summary failed for note" << m_current.noteId << "exit code" << exitCode;
        emit summaryFailed(m_current.noteId, QString("llama-cli exit code %1").arg(exitCode));
        processNext();
        return;
    }

    const QString rawOutput = QString::fromUtf8(m_process->readAllStandardOutput());
    const QString output = extractSummaryText(rawOutput);
    qDebug() << "[SummarizationEngine] summary ready for note" << m_current.noteId << "length" << output.length();
    emit summaryReady(m_current.noteId, output);

    processNext();
}

QString SummarizationEngine::extractSummaryText(const QString &rawOutput) const
{
    QString remaining = rawOutput;

    const int promptIndex = remaining.indexOf(m_currentPrompt);
    if (promptIndex >= 0)
        remaining = remaining.mid(promptIndex + m_currentPrompt.length());

    const int timingIndex = remaining.indexOf("[ Prompt");
    if (timingIndex >= 0)
        remaining = remaining.left(timingIndex);

    static const QStringList noisePrefixes = {
        "build", "model", "ftype", "modalities", "available commands",
        "Loading model", "Exiting", "main:", "system_info",
        "/exit", "/regen", "/clear", "/read", "/glob", ">"
    };

    QStringList keptLines;
    for (const QString &line : remaining.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        bool isNoise = false;
        for (const QString &prefix : noisePrefixes) {
            if (trimmed.startsWith(prefix)) { isNoise = true; break; }
        }
        if (isNoise)
            continue;

        bool isBoxArt = true;
        for (const QChar &ch : trimmed) {
            if (ch.isLetterOrNumber()) { isBoxArt = false; break; }
        }
        if (isBoxArt)
            continue;

        keptLines << trimmed;
    }

    return keptLines.join('\n').trimmed();
}
