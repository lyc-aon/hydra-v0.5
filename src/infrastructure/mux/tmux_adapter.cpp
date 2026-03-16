#include "infrastructure/mux/tmux_adapter.hpp"

#include <QDateTime>
#include <QSet>
#include <QStandardPaths>

namespace hydra::infrastructure {

namespace {

constexpr QChar kPaneFieldSeparator(0x1f);

QStringList tmuxClientArguments(const QStringList &arguments)
{
    QStringList effectiveArguments;

    const QString socketPath = qEnvironmentVariable("HYDRA_TMUX_SOCKET_PATH").trimmed();
    if (!socketPath.isEmpty()) {
        effectiveArguments << QStringLiteral("-S") << socketPath;
    } else {
        const QString socketName = qEnvironmentVariable("HYDRA_TMUX_SOCKET_NAME").trimmed();
        if (!socketName.isEmpty()) {
            effectiveArguments << QStringLiteral("-L") << socketName;
        }
    }

    effectiveArguments << arguments;
    return effectiveArguments;
}

}

TmuxAdapter::TmuxAdapter()
    : m_tmuxPath(QStandardPaths::findExecutable(QStringLiteral("tmux")))
{
}

TmuxAdapter::~TmuxAdapter() = default;

domain::ports::MuxProbeResult TmuxAdapter::probeAvailability() const
{
    if (m_tmuxPath.isEmpty()) {
        return {.available = false,
                .executablePath = QString(),
                .message = QStringLiteral("tmux is not on PATH.")};
    }

    return {.available = true,
            .executablePath = m_tmuxPath,
            .message = QStringLiteral("tmux is available.")};
}

domain::ports::MuxLaunchResult TmuxAdapter::launchDetachedSession(
    const domain::ports::MuxLaunchRequest &request)
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return {.ok = false,
                .sessionName = QString(),
                .paneId = QString(),
                .errorMessage = probe.message};
    }

    QStringList launchArguments = {
        QStringLiteral("new-session"),
        QStringLiteral("-d"),
        QStringLiteral("-s"),
        request.sessionName,
        QStringLiteral("-c"),
        request.workingDirectory,
    };

    if (!request.environment.isEmpty()) {
        launchArguments.append(QStringLiteral("env"));
        QStringList envKeys = request.environment.keys();
        envKeys.sort();
        for (const QString &key : envKeys) {
            launchArguments.append(QStringLiteral("%1=%2").arg(key, request.environment.value(key)));
        }
    }

    if (!request.executable.isEmpty()) {
        launchArguments.append(request.executable);
        launchArguments.append(request.arguments);
    }

    const CommandResult launch = runTmuxCommand(launchArguments);
    if (!launch.ok) {
        return {.ok = false,
                .sessionName = QString(),
                .paneId = QString(),
                .errorMessage = QStringLiteral("tmux new-session failed: %1")
                                    .arg(launch.standardError.trimmed())};
    }

    const CommandResult inspect = runTmuxCommand(
        {QStringLiteral("display-message"),
         QStringLiteral("-p"),
         QStringLiteral("-t"),
         request.sessionName,
         QStringLiteral("#{pane_id}")});

    if (!inspect.ok) {
        return {.ok = true,
                .sessionName = request.sessionName,
                .paneId = QString(),
                .errorMessage = QString()};
    }

    return {.ok = true,
            .sessionName = request.sessionName,
            .paneId = inspect.standardOutput.trimmed(),
            .errorMessage = QString()};
}

domain::ports::MuxTerminateResult TmuxAdapter::terminateSession(const QString &sessionName) const
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return {.ok = false, .errorMessage = probe.message};
    }

    const CommandResult result =
        runTmuxCommand({QStringLiteral("kill-session"), QStringLiteral("-t"), sessionName});
    if (result.ok) {
        return {.ok = true, .errorMessage = QString()};
    }

    const QString detail = result.standardError.trimmed().isEmpty()
                               ? QStringLiteral("tmux kill-session failed")
                               : result.standardError.trimmed();
    return {.ok = false, .errorMessage = detail};
}

bool TmuxAdapter::hasLiveSession(const QString &sessionName) const
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return false;
    }

    const CommandResult result =
        runTmuxCommand({QStringLiteral("has-session"), QStringLiteral("-t"), sessionName});
    return result.ok && result.exitCode == 0;
}

bool TmuxAdapter::syncWatchedSessions(const QVector<domain::ports::MuxWatchRequest> &requests,
                                      QString *errorMessage)
{
    if (requests.isEmpty()) {
        m_observedSessions.clear();
        return true;
    }

    const auto probe = probeAvailability();
    if (!probe.available) {
        if (errorMessage != nullptr) {
            *errorMessage = probe.message;
        }
        return false;
    }

    QString paneSnapshotError;
    const LivePaneSnapshot livePanes = captureLivePaneSnapshot(&paneSnapshotError);
    if (!paneSnapshotError.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = paneSnapshotError;
        }
        return false;
    }

    QSet<QString> requestedIds;
    for (const domain::ports::MuxWatchRequest &request : requests) {
        requestedIds.insert(request.sessionId);

        ObservedSession livePane;
        bool foundLivePane = false;
        if (!request.paneId.trimmed().isEmpty()) {
            const auto paneIt = livePanes.byPaneId.constFind(request.paneId.trimmed());
            if (paneIt != livePanes.byPaneId.cend()) {
                livePane = paneIt.value();
                foundLivePane = true;
            }
        }
        if (!foundLivePane) {
            const auto sessionIt = livePanes.bySessionName.constFind(request.sessionName.trimmed());
            if (sessionIt != livePanes.bySessionName.cend()) {
                livePane = sessionIt.value();
                foundLivePane = true;
            }
        }

        if (!foundLivePane) {
            const auto targetedPane = captureSessionTarget(request.sessionName.trimmed());
            if (targetedPane.has_value()) {
                livePane = targetedPane.value();
                foundLivePane = true;
            }
        }

        if (!foundLivePane && hasLiveSession(request.sessionName.trimmed())) {
            auto existing = m_observedSessions.find(request.sessionId);
            if (existing != m_observedSessions.end()) {
                livePane = existing.value();
            } else {
                livePane = {
                    .sessionName = request.sessionName.trimmed(),
                    .paneId = request.paneId.trimmed(),
                    .paneTail = QString(),
                    .paneTitle = QString(),
                    .lastTailCaptureAt = QDateTime(),
                };
            }
            foundLivePane = true;
        }

        if (!foundLivePane) {
            queueEvent({
                .sessionId = request.sessionId,
                .sessionName = request.sessionName,
                .paneId = request.paneId,
                .kind = domain::ports::MuxEventKind::SessionExited,
                .payload = QStringLiteral("tmux no longer reports the session as live."),
                .paneTail = QString(),
                .paneTitle = QString(),
                .occurredAt = QDateTime::currentDateTimeUtc(),
            });
            m_observedSessions.remove(request.sessionId);
            continue;
        }

        const QString effectivePaneId =
            livePane.paneId.trimmed().isEmpty() ? request.paneId : livePane.paneId;
        auto existing = m_observedSessions.find(request.sessionId);
        const bool isNewObservation = existing == m_observedSessions.end()
                                      || existing->sessionName != livePane.sessionName
                                      || existing->paneId != effectivePaneId;
        const QDateTime now = QDateTime::currentDateTimeUtc();
        bool shouldCaptureTail = request.capturePaneTail;
        if (shouldCaptureTail && !isNewObservation && existing != m_observedSessions.end()
            && request.tailCaptureIntervalMs > 0 && existing->lastTailCaptureAt.isValid()
            && existing->lastTailCaptureAt.msecsTo(now) < request.tailCaptureIntervalMs)
        {
            shouldCaptureTail = false;
        }
        const QString paneTail = shouldCaptureTail
                                     ? capturePaneTail(effectivePaneId, request.tailCaptureLineCount)
                                     : (existing != m_observedSessions.end()
                                            ? existing->paneTail
                                            : QString());
        const QString paneTitle = request.capturePaneTitle
                                      ? livePane.paneTitle
                                      : (existing != m_observedSessions.end()
                                             ? existing->paneTitle
                                             : QString());

        if (isNewObservation) {
            queueEvent({
                .sessionId = request.sessionId,
                .sessionName = livePane.sessionName,
                .paneId = effectivePaneId,
                .kind = domain::ports::MuxEventKind::MonitorAttached,
                .payload = paneTail,
                .paneTail = paneTail,
                .paneTitle = paneTitle,
                .occurredAt = QDateTime::currentDateTimeUtc(),
            });
        } else if (existing->paneTail != paneTail || existing->paneTitle != paneTitle) {
            queueEvent({
                .sessionId = request.sessionId,
                .sessionName = livePane.sessionName,
                .paneId = effectivePaneId,
                .kind = domain::ports::MuxEventKind::PaneOutput,
                .payload = paneTail,
                .paneTail = paneTail,
                .paneTitle = paneTitle,
                .occurredAt = QDateTime::currentDateTimeUtc(),
            });
        }

        m_observedSessions.insert(request.sessionId,
                                  ObservedSession{
                                      .sessionName = livePane.sessionName,
                                      .paneId = effectivePaneId,
                                      .paneTail = paneTail,
                                      .paneTitle = paneTitle,
                                      .lastTailCaptureAt = shouldCaptureTail
                                                               ? now
                                                               : (existing != m_observedSessions.end()
                                                                      ? existing->lastTailCaptureAt
                                                                      : QDateTime()),
                                  });
    }

    const QStringList currentKeys = m_observedSessions.keys();
    for (const QString &sessionId : currentKeys) {
        if (!requestedIds.contains(sessionId)) {
            stopWatchingSession(sessionId);
        }
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

QHash<QString, QVector<domain::ports::MuxEvent>> TmuxAdapter::takePendingEvents()
{
    QHash<QString, QVector<domain::ports::MuxEvent>> events;
    events.swap(m_pendingEventsBySession);
    return events;
}

void TmuxAdapter::stopWatchingSession(const QString &sessionId)
{
    m_observedSessions.remove(sessionId);
    m_pendingEventsBySession.remove(sessionId);
}

QString TmuxAdapter::capturePaneTail(const QString &paneId, const int lineCount) const
{
    if (paneId.trimmed().isEmpty()) {
        return QString();
    }

    const int effectiveLineCount = lineCount <= 0 ? 64 : lineCount;
    const CommandResult capture = runTmuxCommand({QStringLiteral("capture-pane"),
                                                  QStringLiteral("-p"),
                                                  QStringLiteral("-J"),
                                                  QStringLiteral("-S"),
                                                  QStringLiteral("-%1").arg(effectiveLineCount),
                                                  QStringLiteral("-t"),
                                                  paneId});
    return capture.ok ? capture.standardOutput : QString();
}

QString TmuxAdapter::capturePaneTitle(const QString &paneId) const
{
    if (paneId.trimmed().isEmpty()) {
        return QString();
    }

    const CommandResult capture = runTmuxCommand({QStringLiteral("display-message"),
                                                  QStringLiteral("-p"),
                                                  QStringLiteral("-t"),
                                                  paneId,
                                                  QStringLiteral("#{pane_title}")});
    return capture.ok ? capture.standardOutput.trimmed() : QString();
}

TmuxAdapter::LivePaneSnapshot TmuxAdapter::captureLivePaneSnapshot(QString *errorMessage) const
{
    LivePaneSnapshot snapshot;

    const CommandResult result = runTmuxCommand({QStringLiteral("list-panes"),
                                                 QStringLiteral("-a"),
                                                 QStringLiteral("-F"),
                                                 QStringLiteral("#{session_name}%1#{pane_id}%1#{pane_title}")
                                                     .arg(kPaneFieldSeparator)});
    if (!result.ok) {
        const QString detail = result.standardError.trimmed();
        if (detail.contains(QStringLiteral("no server running"), Qt::CaseInsensitive)
            || detail.contains(QStringLiteral("failed to connect to server"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr) {
                errorMessage->clear();
            }
            return snapshot;
        }
        if (errorMessage != nullptr) {
            *errorMessage = detail.isEmpty() ? QStringLiteral("tmux list-panes failed") : detail;
        }
        return snapshot;
    }

    const QStringList lines =
        result.standardOutput.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList parts = line.split(kPaneFieldSeparator, Qt::KeepEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        ObservedSession pane;
        pane.sessionName = parts.at(0).trimmed();
        pane.paneId = parts.at(1).trimmed();
        pane.paneTitle = parts.size() >= 3 ? parts.at(2).trimmed() : QString();
        if (pane.sessionName.isEmpty() || pane.paneId.isEmpty()) {
            continue;
        }

        snapshot.byPaneId.insert(pane.paneId, pane);
        if (!snapshot.bySessionName.contains(pane.sessionName)) {
            snapshot.bySessionName.insert(pane.sessionName, pane);
        }
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return snapshot;
}

std::optional<TmuxAdapter::ObservedSession> TmuxAdapter::captureSessionTarget(
    const QString &sessionName) const
{
    const QString trimmedSessionName = sessionName.trimmed();
    if (trimmedSessionName.isEmpty()) {
        return std::nullopt;
    }

    const CommandResult result = runTmuxCommand({QStringLiteral("display-message"),
                                                 QStringLiteral("-p"),
                                                 QStringLiteral("-t"),
                                                 trimmedSessionName,
                                                 QStringLiteral("#{session_name}%1#{pane_id}%1#{pane_title}")
                                                     .arg(kPaneFieldSeparator)});
    if (!result.ok) {
        return std::nullopt;
    }

    const QStringList parts =
        result.standardOutput.trimmed().split(kPaneFieldSeparator, Qt::KeepEmptyParts);
    if (parts.size() < 2) {
        return std::nullopt;
    }

    ObservedSession pane;
    pane.sessionName = parts.at(0).trimmed();
    pane.paneId = parts.at(1).trimmed();
    pane.paneTitle = parts.size() >= 3 ? parts.at(2).trimmed() : QString();
    if (pane.sessionName.isEmpty() || pane.paneId.isEmpty()) {
        return std::nullopt;
    }

    return pane;
}

TmuxAdapter::CommandResult TmuxAdapter::runTmuxCommand(const QStringList &arguments) const
{
    if (m_tmuxPath.isEmpty()) {
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = QStringLiteral("tmux is unavailable")};
    }

    CommandResult result = process::runProcess(m_tmuxPath, tmuxClientArguments(arguments));
    if (!result.ok && result.standardError == QStringLiteral("process timed out")) {
        result.standardError = QStringLiteral("tmux command timed out");
    }

    return result;
}

void TmuxAdapter::queueEvent(const domain::ports::MuxEvent &event)
{
    m_pendingEventsBySession[event.sessionId].push_back(event);
}

}  // namespace hydra::infrastructure
