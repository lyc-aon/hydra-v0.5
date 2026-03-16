#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

namespace hydra::domain::ports {

struct MuxProbeResult {
    bool available = false;
    QString executablePath;
    QString message;
};

struct MuxLaunchRequest {
    QString sessionName;
    QString workingDirectory;
    QString executable;
    QStringList arguments;
    QHash<QString, QString> environment;
};

struct MuxLaunchResult {
    bool ok = false;
    QString sessionName;
    QString paneId;
    QString errorMessage;
};

struct MuxTerminateResult {
    bool ok = false;
    QString errorMessage;
};

enum class MuxEventKind {
    MonitorAttached,
    PaneOutput,
    SessionExited,
    MonitorError,
};

struct MuxWatchRequest {
    QString sessionId;
    QString sessionName;
    QString paneId;
    bool capturePaneTail = true;
    bool capturePaneTitle = true;
    int tailCaptureIntervalMs = 0;
    int tailCaptureLineCount = 64;
};

struct MuxEvent {
    QString sessionId;
    QString sessionName;
    QString paneId;
    MuxEventKind kind = MuxEventKind::PaneOutput;
    QString payload;
    QString paneTail;
    QString paneTitle;
    QDateTime occurredAt;
};

class MuxAdapter {
public:
    virtual ~MuxAdapter() = default;

    virtual MuxProbeResult probeAvailability() const = 0;
    virtual MuxLaunchResult launchDetachedSession(const MuxLaunchRequest &request) = 0;
    virtual MuxTerminateResult terminateSession(const QString &sessionName) const = 0;
    virtual bool hasLiveSession(const QString &sessionName) const = 0;
    virtual bool syncWatchedSessions(const QVector<MuxWatchRequest> &requests,
                                     QString *errorMessage = nullptr) = 0;
    virtual QHash<QString, QVector<MuxEvent>> takePendingEvents() = 0;
    virtual void stopWatchingSession(const QString &sessionId) = 0;
    virtual QString capturePaneTail(const QString &paneId, int lineCount = 64) const = 0;
    virtual QString capturePaneTitle(const QString &paneId) const = 0;
};

}  // namespace hydra::domain::ports
