#pragma once

#include <optional>

#include <QHash>
#include <QString>

#include "domain/ports/mux_adapter.hpp"
#include "infrastructure/process/process_runner.hpp"

namespace hydra::infrastructure {

class TmuxAdapter final : public domain::ports::MuxAdapter {
public:
    TmuxAdapter();
    ~TmuxAdapter() override;

    domain::ports::MuxProbeResult probeAvailability() const override;
    domain::ports::MuxLaunchResult launchDetachedSession(
        const domain::ports::MuxLaunchRequest &request) override;
    domain::ports::MuxTerminateResult terminateSession(const QString &sessionName) const override;
    bool hasLiveSession(const QString &sessionName) const override;
    bool syncWatchedSessions(const QVector<domain::ports::MuxWatchRequest> &requests,
                             QString *errorMessage = nullptr) override;
    QHash<QString, QVector<domain::ports::MuxEvent>> takePendingEvents() override;
    void stopWatchingSession(const QString &sessionId) override;
    QString capturePaneTail(const QString &paneId, int lineCount = 64) const override;
    QString capturePaneTitle(const QString &paneId) const override;

private:
    struct ObservedSession {
        QString sessionName;
        QString paneId;
        QString paneTail;
        QString paneTitle;
        QDateTime lastTailCaptureAt;
    };

    struct LivePaneSnapshot {
        QHash<QString, ObservedSession> byPaneId;
        QHash<QString, ObservedSession> bySessionName;
    };

    using CommandResult = process::ProcessResult;

    CommandResult runTmuxCommand(const QStringList &arguments) const;
    LivePaneSnapshot captureLivePaneSnapshot(QString *errorMessage = nullptr) const;
    std::optional<ObservedSession> captureSessionTarget(const QString &sessionName) const;
    void queueEvent(const domain::ports::MuxEvent &event);

    QString m_tmuxPath;
    QHash<QString, ObservedSession> m_observedSessions;
    QHash<QString, QVector<domain::ports::MuxEvent>> m_pendingEventsBySession;
};

}  // namespace hydra::infrastructure
