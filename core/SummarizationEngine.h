#pragma once

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QTimer>

class SummarizationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SummarizationEngine(QObject *parent = nullptr);

public slots:
    void summarize(const QString &noteId, const QString &transcript);

signals:
    void summaryReady(const QString &noteId, const QString &summary);
    void summaryFailed(const QString &noteId, const QString &reason);

private slots:
    void processNext();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onTimeout();

private:
    QString extractSummaryText(const QString &rawOutput) const;

    struct PendingSummary {
        QString noteId;
        QString transcript;
    };

    QQueue<PendingSummary> m_queue;
    QProcess *m_process;
    QTimer m_timeoutTimer;
    PendingSummary m_current;
    QString m_currentPrompt;
    bool m_busy;
};
