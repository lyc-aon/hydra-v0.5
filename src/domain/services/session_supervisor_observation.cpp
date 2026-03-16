#include "domain/services/session_supervisor.hpp"

#include <QDateTime>
#include <QDebug>

#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

namespace {

constexpr int kTimelineRetentionPerSession = 24;
constexpr int kIdleTailCaptureIntervalMs = 4000;
constexpr int kCodexIdleTailCaptureIntervalMs = 8000;
constexpr int kCodexIdleTailCaptureLineCount = 16;
constexpr int kIdleTailCaptureLineCount = 24;

int tailCaptureIntervalForSession(const SessionRecord &session)
{
    if (session.state != SessionState::Idle) {
        return 0;
    }
    if (session.providerKey == QStringLiteral("codex")) {
        return kCodexIdleTailCaptureIntervalMs;
    }
    return kIdleTailCaptureIntervalMs;
}

int tailCaptureLineCountForSession(const SessionRecord &session)
{
    if (session.state != SessionState::Idle) {
        return 64;
    }
    if (session.providerKey == QStringLiteral("codex")) {
        return kCodexIdleTailCaptureLineCount;
    }
    return kIdleTailCaptureLineCount;
}

bool providerNeedsPaneTitle(const ports::ProviderObservabilityHints &hints)
{
    return !hints.promptTitlePatterns.isEmpty() || !hints.approvalTitlePatterns.isEmpty()
           || !hints.waitingTitlePatterns.isEmpty() || !hints.activeTitlePatterns.isEmpty();
}

QVector<ports::MuxWatchRequest> watchRequestsForSessions(
    const QVector<SessionRecord> &sessions,
    const QHash<QString, ports::ProviderObservabilityHints> &hintsByProvider)
{
    QVector<ports::MuxWatchRequest> watchRequests;
    watchRequests.reserve(sessions.size());
    for (const SessionRecord &session : sessions) {
        if (session.tmuxSessionName.trimmed().isEmpty() || session.state == SessionState::Exited) {
            continue;
        }

        const ports::ProviderObservabilityHints hints = hintsByProvider.value(session.providerKey);
        watchRequests.push_back({
            .sessionId = session.id,
            .sessionName = session.tmuxSessionName,
            .paneId = session.tmuxPaneId,
            .capturePaneTail = true,
            .capturePaneTitle = providerNeedsPaneTitle(hints),
            .tailCaptureIntervalMs = tailCaptureIntervalForSession(session),
            .tailCaptureLineCount = tailCaptureLineCountForSession(session),
        });
    }

    return watchRequests;
}

QHash<QString, QVector<SessionSignal>> signalsBySessionFromEvents(
    const QHash<QString, QVector<ports::MuxEvent>> &pendingEventsBySession)
{
    QHash<QString, QVector<SessionSignal>> signalsBySession;
    for (auto it = pendingEventsBySession.cbegin(); it != pendingEventsBySession.cend(); ++it) {
        QVector<SessionSignal> sessionSignals;
        sessionSignals.reserve(it.value().size());
        for (const ports::MuxEvent &event : it.value()) {
            SessionSignal signal;
            signal.sessionId = event.sessionId;
            signal.sessionName = event.sessionName;
            signal.paneId = event.paneId;
            signal.payload = event.payload;
            signal.paneTail = event.paneTail;
            signal.paneTitle = event.paneTitle;
            signal.occurredAt =
                event.occurredAt.isValid() ? event.occurredAt : QDateTime::currentDateTimeUtc();
            signal.sourceRef = event.paneId.isEmpty() ? event.sessionName : event.paneId;

            switch (event.kind) {
            case ports::MuxEventKind::MonitorAttached:
                signal.kind = SessionSignalKind::MonitorAttached;
                signal.provenance = StatusProvenance::Tmux;
                break;
            case ports::MuxEventKind::PaneOutput:
                signal.kind = SessionSignalKind::PaneOutputObserved;
                signal.provenance = StatusProvenance::Tmux;
                break;
            case ports::MuxEventKind::SessionExited:
                signal.kind = SessionSignalKind::SessionExited;
                signal.provenance = StatusProvenance::Tmux;
                break;
            case ports::MuxEventKind::MonitorError:
                signal.kind = SessionSignalKind::MonitorError;
                signal.provenance = StatusProvenance::MonitorError;
                break;
            }

            sessionSignals.push_back(signal);
        }
        signalsBySession.insert(it.key(), sessionSignals);
    }

    return signalsBySession;
}

bool shouldCapturePaneTail(const SessionSignal &signal)
{
    return signal.kind != SessionSignalKind::SessionExited
           && signal.kind != SessionSignalKind::BootstrapExited
           && signal.kind != SessionSignalKind::MonitorError;
}

bool statusChanged(const SessionRecord &session, const SessionStatusUpdate &update)
{
    return session.state != update.state || session.statusDetail != update.detail
           || session.statusProvenance != update.provenance
           || session.statusSourceRef != update.sourceRef;
}

void markPersistenceFailure(bool *hadPersistenceFailure)
{
    if (hadPersistenceFailure != nullptr) {
        *hadPersistenceFailure = true;
    }
}

void refreshSessionFromStore(ports::SessionStore &sessionStore, SessionRecord &session)
{
    const auto refreshed = sessionStore.sessionById(session.id);
    if (!refreshed.has_value()) {
        return;
    }

    session = refreshed.value();
}

}  // namespace

QVector<SessionRecord> SessionSupervisor::refreshSessionStates()
{
    QVector<SessionRecord> currentSessions = m_sessionStore.listSessions();
    QSet<QString> claimedProviderSessionIds;
    for (const SessionRecord &session : currentSessions) {
        const QString providerSessionId = session.providerSessionId.trimmed();
        if (!providerSessionId.isEmpty()
            && !isPendingResumeToken(session.providerKey, providerSessionId))
        {
            claimedProviderSessionIds.insert(providerSessionId);
        }
    }

    for (SessionRecord &session : currentSessions) {
        if (resolvePendingResumeToken(session, claimedProviderSessionIds)) {
            claimedProviderSessionIds.insert(session.providerSessionId);
        }
    }

    QHash<QString, ports::ProviderObservabilityHints> hintsByProvider;
    for (const SessionRecord &session : currentSessions) {
        if (session.state == SessionState::Exited
            || hintsByProvider.contains(session.providerKey))
        {
            continue;
        }
        hintsByProvider.insert(session.providerKey, hintsForProvider(session.providerKey));
    }

    // Backfill aliases for live sessions that predate the alias feature.
    for (SessionRecord &session : currentSessions) {
        if (session.state == SessionState::Exited || !session.alias.trimmed().isEmpty()) {
            continue;
        }
        session.alias = generateNextAlias();
        m_sessionStore.upsertSession(session);
    }

    const QVector<ports::MuxWatchRequest> watchRequests =
        watchRequestsForSessions(currentSessions, hintsByProvider);

    QString monitorError;
    const bool watchOk = m_muxAdapter.syncWatchedSessions(watchRequests, &monitorError);
    if (!watchOk && !monitorError.trimmed().isEmpty()) {
        qWarning().noquote()
            << QStringLiteral("Hydra tmux monitor sync failed: %1").arg(monitorError);
    }

    const QHash<QString, QVector<SessionSignal>> signalsBySession =
        signalsBySessionFromEvents(m_muxAdapter.takePendingEvents());

    bool hadPersistenceFailure = false;
    for (SessionRecord &session : currentSessions) {
        if (session.state == SessionState::Exited) {
            continue;
        }

        const QVector<SessionSignal> sessionSignals = signalsBySession.value(session.id);
        if (sessionSignals.isEmpty()) {
            continue;
        }

        const ports::ProviderObservabilityHints hints = hintsByProvider.value(session.providerKey);
        for (const SessionSignal &signal : sessionSignals) {
            refreshSessionFromStore(m_sessionStore, session);
            if (session.state == SessionState::Exited) {
                m_muxAdapter.stopWatchingSession(session.id);
                break;
            }

            QString paneTail;
            QString paneTitle;
            if (shouldCapturePaneTail(signal)) {
                const QString paneId = signal.paneId.isEmpty() ? session.tmuxPaneId : signal.paneId;
                paneTail = signal.paneTail;
                if (paneTail.isEmpty()) {
                    paneTail = m_muxAdapter.capturePaneTail(paneId);
                }

                if (providerNeedsPaneTitle(hints)) {
                    paneTitle = signal.paneTitle;
                    if (paneTitle.isEmpty()) {
                        paneTitle = m_muxAdapter.capturePaneTitle(paneId);
                    }
                }
            }

            applyStatusSignal(session, signal, paneTail, paneTitle, hints, &hadPersistenceFailure);

            if (session.state == SessionState::Exited) {
                m_muxAdapter.stopWatchingSession(session.id);
            }
        }
    }

    return hadPersistenceFailure ? currentSessions : m_sessionStore.listSessions();
}

ports::ProviderObservabilityHints SessionSupervisor::hintsForProvider(
    const QString &providerKey) const
{
    const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(providerKey);
    if (adapter == nullptr) {
        return {};
    }

    return adapter->observabilityHints();
}

bool SessionSupervisor::applyStatusSignal(SessionRecord &session,
                                          const SessionSignal &signal,
                                          const QString &paneTail,
                                          const QString &paneTitle,
                                          const ports::ProviderObservabilityHints &hints,
                                          bool *hadPersistenceFailure)
{
    const SessionStatusUpdate update =
        m_statusAggregator.applySignal(session, signal, hints, paneTail, paneTitle);
    if (!statusChanged(session, update)) {
        return false;
    }

    session.state = update.state;
    session.statusDetail = update.detail;
    session.statusProvenance = update.provenance;
    session.statusSourceRef = update.sourceRef;
    session.updatedAt =
        update.timelineEvent.createdAt.isValid() ? update.timelineEvent.createdAt
                                                 : QDateTime::currentDateTimeUtc();
    if (signal.kind == SessionSignalKind::MonitorError) {
        session.lastError = update.detail;
    } else if (!session.lastError.isEmpty()) {
        session.lastError.clear();
    }

    QString timelineError;
    if (!m_sessionStore.appendTimelineEvent(update.timelineEvent,
                                            kTimelineRetentionPerSession,
                                            &timelineError))
    {
        markPersistenceFailure(hadPersistenceFailure);
        session.lastError = QStringLiteral("Failed to persist session timeline: %1")
                                .arg(timelineError);
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist session timeline event for %1: %2")
                   .arg(session.id, timelineError);
    }

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        markPersistenceFailure(hadPersistenceFailure);
        session.lastError = QStringLiteral("Failed to persist observed session state: %1")
                                .arg(errorMessage);
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist observed session state for %1: %2")
                   .arg(session.id, errorMessage);
    }

    return true;
}

}  // namespace hydra::domain
