#include "transcriptionworker.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>

TranscriptionWorker::TranscriptionWorker(int workerId, QObject *parent)
    : QObject(parent), m_workerId(workerId)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TranscriptionWorker::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &TranscriptionWorker::onProcessErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &TranscriptionWorker::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &TranscriptionWorker::onReadyReadStderr);
}

void TranscriptionWorker::configure(const QString &whisperExePath, const QString &modelPath, int threadCount)
{
    m_whisperExePath = whisperExePath;
    m_modelPath = modelPath;
    m_threadCount = threadCount;

    if (!QFileInfo::exists(m_whisperExePath))
        qWarning() << "[Worker" << m_workerId << "] whisper-cli NOT FOUND at" << m_whisperExePath;
    if (!QFileInfo::exists(m_modelPath))
        qWarning() << "[Worker" << m_workerId << "] model NOT FOUND at" << m_modelPath;
}

bool TranscriptionWorker::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

void TranscriptionWorker::start(int chunkIndex, const QString &audioFilePath, const QString &outputBasePath)
{
    m_currentChunkIndex = chunkIndex;
    m_currentAudioFilePath = audioFilePath;
    m_currentOutputBasePath = outputBasePath;
    m_attempt = 0;
    m_jobTimer.start();

    // A chunk file interrupted mid-write by stopSession() (e.g. the very
    // last chunk when the user hits stop right after a rotation) can be a
    // few hundred bytes of WAV header with no audio frames. whisper-cli
    // fails to parse these ("failed to read the frames of the audio data")
    // and retrying doesn't help since the file itself never gets any more
    // complete. Skip straight to a clean failure instead of burning three
    // retries launching a process against a file that can't succeed.
    static constexpr qint64 kMinValidWavBytes = 4096;

    QFileInfo info(audioFilePath);
    if (!info.exists()) {
        qWarning() << "[Worker" << m_workerId << "] chunk" << chunkIndex << "audio file missing:" << audioFilePath;
        emit chunkFailed(chunkIndex, "audio file missing: " + audioFilePath);
        return;
    }

    if (info.size() < kMinValidWavBytes) {
        qWarning() << "[Worker" << m_workerId << "] chunk" << chunkIndex
                   << "too small/corrupt (" << info.size() << "bytes), likely truncated on stop, skipping:" << audioFilePath;
        emit chunkFailed(chunkIndex, QString("chunk too small (%1 bytes), likely truncated on stop").arg(info.size()));
        return;
    }

    launchProcess();
}

void TranscriptionWorker::launchProcess()
{
    m_attemptHandled = false;

    // Attempt 0 uses the normal configured thread count. Every retry after
    // that drops to a single thread - if the crash is caused by a worker
    // thread landing on a CPU core that doesn't support whatever SIMD
    // instructions whisper-cli was compiled with (common on hybrid
    // P-core/E-core CPUs), running single-threaded is far less likely to
    // hit an unsupported core mid-computation.
    const int threads = (m_attempt == 0) ? m_threadCount : 1;

    QStringList args;
    args << "-m" << m_modelPath
         << "-f" << m_currentAudioFilePath
         << "--translate"
         << "-t" << QString::number(threads)
         << "--max-context" << "0"
         << "--no-gpu"
         << "--no-flash-attn"
         << "-otxt"
         << "-of" << m_currentOutputBasePath;

    qInfo() << "[Worker" << m_workerId << "] START chunk" << m_currentChunkIndex
            << "attempt" << (m_attempt + 1) << "/" << kMaxAttempts
            << "threads=" << threads
            << "file:" << m_currentAudioFilePath
            << "cmd:" << m_whisperExePath << args.join(' ');

    m_process->start(m_whisperExePath, args);
}

void TranscriptionWorker::onReadyReadStdout()
{
    const QString out = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    if (!out.isEmpty())
        qDebug().noquote() << "[Worker" << m_workerId << "][stdout]" << out;
}

void TranscriptionWorker::onReadyReadStderr()
{
    const QString err = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    if (!err.isEmpty())
        qDebug().noquote() << "[Worker" << m_workerId << "][stderr]" << err;
}

void TranscriptionWorker::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if (m_attemptHandled)
        return; // already reported via onProcessErrorOccurred (e.g. a crash)

    const qint64 elapsedMs = m_jobTimer.elapsed();

    if (status != QProcess::NormalExit || exitCode != 0) {
        m_attemptHandled = true;
        qWarning() << "[Worker" << m_workerId << "] chunk" << m_currentChunkIndex
                   << "attempt" << (m_attempt + 1) << "FAILED exitCode=" << exitCode
                   << "after" << elapsedMs << "ms";
        retryOrFail(QString("whisper-cli exited with code %1").arg(exitCode));
        return;
    }

    QFile txtFile(m_currentOutputBasePath + ".txt");
    if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_attemptHandled = true;
        qWarning() << "[Worker" << m_workerId << "] chunk" << m_currentChunkIndex
                   << "output file missing:" << txtFile.fileName();
        retryOrFail("output file missing: " + txtFile.fileName());
        return;
    }

    QTextStream stream(&txtFile);
    const QString text = stream.readAll().trimmed();
    txtFile.close();

    m_attemptHandled = true;
    qInfo() << "[Worker" << m_workerId << "] DONE chunk" << m_currentChunkIndex
            << "in" << elapsedMs << "ms, chars:" << text.length();

    emit chunkFinished(m_currentChunkIndex, text);
}

void TranscriptionWorker::onProcessErrorOccurred(QProcess::ProcessError error)
{
    if (m_attemptHandled)
        return; // already reported (e.g. finished() beat us to it)

    m_attemptHandled = true;
    qWarning() << "[Worker" << m_workerId << "] chunk" << m_currentChunkIndex
               << "PROCESS ERROR:" << error << m_process->errorString();
    retryOrFail(QString("process error: %1 (%2)")
                    .arg(static_cast<int>(error)).arg(m_process->errorString()));
}

void TranscriptionWorker::retryOrFail(const QString &reason)
{
    if (m_attempt + 1 < kMaxAttempts) {
        m_attempt++;
        qWarning() << "[Worker" << m_workerId << "] chunk" << m_currentChunkIndex
                   << "retrying (attempt" << (m_attempt + 1) << "/" << kMaxAttempts
                   << ", single-threaded) after failure:" << reason;
        QTimer::singleShot(0, this, &TranscriptionWorker::launchProcess);
        return;
    }

    qWarning() << "[Worker" << m_workerId << "] chunk" << m_currentChunkIndex
               << "giving up after" << kMaxAttempts << "attempts. Last error:" << reason;
    emit chunkFailed(m_currentChunkIndex, reason);
}