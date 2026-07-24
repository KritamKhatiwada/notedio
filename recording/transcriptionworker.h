#ifndef TRANSCRIPTIONWORKER_H
#define TRANSCRIPTIONWORKER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QElapsedTimer>

class TranscriptionWorker : public QObject
{
    Q_OBJECT
public:
    explicit TranscriptionWorker(int workerId, QObject *parent = nullptr);

    void configure(const QString &whisperExePath, const QString &modelPath, int threadCount);
    void start(int chunkIndex, const QString &audioFilePath, const QString &outputBasePath);
    bool isRunning() const;

signals:
    void chunkFinished(int chunkIndex, const QString &transcribedText);
    void chunkFailed(int chunkIndex, const QString &errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onReadyReadStdout();
    void onReadyReadStderr();

private:
    void launchProcess();
    void retryOrFail(const QString &reason);

    int m_workerId;
    QString m_whisperExePath;
    QString m_modelPath;
    int m_threadCount = 8;
    QProcess *m_process = nullptr;

    int m_currentChunkIndex = -1;
    QString m_currentAudioFilePath;
    QString m_currentOutputBasePath;
    QElapsedTimer m_jobTimer;

    // A single whisper-cli invocation can crash for environment-specific
    // reasons we can't fully control from here (e.g. hybrid P-core/E-core
    // CPUs dispatching SIMD instructions to a core that doesn't support
    // them). Rather than surface a hard failure on the very first crash,
    // we retry a couple of times with progressively safer settings
    // (currently: dropping to a single thread) before giving up for real.
    static constexpr int kMaxAttempts = 3;
    int m_attempt = 0;

    // A crashed QProcess emits BOTH errorOccurred(Crashed) and
    // finished(exitCode, CrashExit). This guards against handling the same
    // attempt twice (which would double-trigger a retry or double-report).
    bool m_attemptHandled = false;
};

#endif