#include "domain/services/session_supervisor.hpp"

#include <QDateTime>
#include <QDebug>
#include <QSet>
#include <QUuid>

#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

namespace {

constexpr int kTimelineRetentionPerSession = 24;

const QStringList kNatoAlphabet = {
    QStringLiteral("Alpha"), QStringLiteral("Bravo"), QStringLiteral("Charlie"),
    QStringLiteral("Delta"), QStringLiteral("Echo"), QStringLiteral("Foxtrot"),
    QStringLiteral("Golf"), QStringLiteral("Hotel"), QStringLiteral("India"),
    QStringLiteral("Juliet"), QStringLiteral("Kilo"), QStringLiteral("Lima"),
    QStringLiteral("Mike"), QStringLiteral("November"), QStringLiteral("Oscar"),
    QStringLiteral("Papa"), QStringLiteral("Quebec"), QStringLiteral("Romeo"),
    QStringLiteral("Sierra"), QStringLiteral("Tango"), QStringLiteral("Uniform"),
    QStringLiteral("Victor"), QStringLiteral("Whiskey"), QStringLiteral("Xray"),
    QStringLiteral("Yankee"), QStringLiteral("Zulu"),
};

SessionTimelineEvent makeActionTimelineEvent(const SessionRecord &session,
                                             const SessionSignalKind signalKind,
                                             const QString &summary,
                                             const QString &detail,
                                             const StatusProvenance provenance,
                                             const QString &sourceRef,
                                             const QDateTime &createdAt)
{
    return {
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .sessionId = session.id,
        .signalKind = signalKind,
        .state = session.state,
        .summary = summary,
        .detail = detail,
        .provenance = provenance,
        .sourceRef = sourceRef,
        .createdAt = createdAt,
    };
}

}  // namespace

SessionDeletionOutcome SessionSupervisor::deleteStoredSessions(const QStringList &sessionIds)
{
    QStringList normalizedIds;
    normalizedIds.reserve(sessionIds.size());
    QSet<QString> seenIds;
    for (const QString &sessionId : sessionIds) {
        const QString trimmedId = sessionId.trimmed();
        if (trimmedId.isEmpty() || seenIds.contains(trimmedId)) {
            continue;
        }
        seenIds.insert(trimmedId);
        normalizedIds.push_back(trimmedId);
    }

    if (normalizedIds.isEmpty()) {
        return {.ok = true,
                .deletedCount = 0,
                .message = QStringLiteral("No stored sessions were selected.")};
    }

    QStringList deletableIds;
    deletableIds.reserve(normalizedIds.size());
    for (const QString &sessionId : normalizedIds) {
        const auto session = m_sessionStore.sessionById(sessionId);
        if (!session.has_value()) {
            continue;
        }
        if (session->state != SessionState::Exited) {
            return {.ok = false,
                    .deletedCount = 0,
                    .message = QStringLiteral("Only closed stored sessions can be deleted.")};
        }
        deletableIds.push_back(sessionId);
    }

    if (deletableIds.isEmpty()) {
        return {.ok = true,
                .deletedCount = 0,
                .message = QStringLiteral("No stored sessions were found to delete.")};
    }

    QString errorMessage;
    if (!m_sessionStore.deleteSessions(deletableIds, &errorMessage)) {
        return {.ok = false,
                .deletedCount = 0,
                .message = QStringLiteral("Hydra could not delete the stored sessions: %1")
                               .arg(errorMessage)};
    }

    for (const QString &sessionId : deletableIds) {
        m_pendingResumeResolutions.remove(sessionId);
    }

    return {.ok = true,
            .deletedCount = static_cast<int>(deletableIds.size()),
            .message = deletableIds.size() == 1
                           ? QStringLiteral("Deleted 1 stored session.")
                           : QStringLiteral("Deleted %1 stored sessions.")
                                 .arg(deletableIds.size())};
}

SessionDeletionOutcome SessionSupervisor::clearClosedResumableSessions()
{
    QStringList storedIds;
    const QVector<SessionRecord> storedSessions = m_sessionStore.listSessions();
    storedIds.reserve(storedSessions.size());

    for (const SessionRecord &session : storedSessions) {
        if (session.state != SessionState::Exited) {
            continue;
        }

        const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey);
        if (adapter == nullptr || !adapter->supportsResume()) {
            continue;
        }

        storedIds.push_back(session.id);
    }

    return deleteStoredSessions(storedIds);
}

SessionTerminationOutcome SessionSupervisor::terminateSession(const QString &sessionId)
{
    const auto sessionRecord = m_sessionStore.sessionById(sessionId);
    if (!sessionRecord.has_value()) {
        return {.ok = false,
                .message = QStringLiteral("Session could not be found."),
                .session = std::nullopt};
    }

    SessionRecord session = sessionRecord.value();
    const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey);
    const bool supportsResume = adapter != nullptr && adapter->supportsResume();
    const QString liveTmuxSessionName = session.tmuxSessionName;
    const bool isLive = !liveTmuxSessionName.isEmpty()
                        && m_muxAdapter.hasLiveSession(liveTmuxSessionName);
    const bool foreignOwnedSession = !session.ownerInstanceId.trimmed().isEmpty()
                                     && session.ownerInstanceId != m_runtimeOwnerInstanceId;
    const QSet<QString> excludedSessionIds = claimedProviderSessionIdsExcluding(session.id);
    resolvePendingResumeToken(session, excludedSessionIds);
    bool resumeReady = !isPendingResumeToken(session.providerKey, session.providerSessionId);

    QString capturedTranscript;
    if (isLive) {
        capturedTranscript = captureSessionTranscript(session);
    }

    if (isLive) {
        const auto terminate = m_muxAdapter.terminateSession(liveTmuxSessionName);
        if (!terminate.ok) {
            return {.ok = false,
                    .message = QStringLiteral("Failed to end tmux session %1: %2")
                                   .arg(liveTmuxSessionName, terminate.errorMessage),
                    .session = std::nullopt};
        }
    }

    if (!resumeReady) {
        resolvePendingResumeToken(session, excludedSessionIds);
        resumeReady = !isPendingResumeToken(session.providerKey, session.providerSessionId);
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    session.state = SessionState::Exited;
    session.lastError.clear();
    const bool pendingResumeMetadata =
        supportsResume && isPendingResumeToken(session.providerKey, session.providerSessionId)
        && !resumeReady;
    if (foreignOwnedSession && pendingResumeMetadata) {
        session.statusDetail = QStringLiteral(
            "Hydra cleaned up a provider session left behind by an earlier app instance before provider resume metadata was resolved. Resume will unlock automatically once the provider metadata becomes visible.");
    } else if (foreignOwnedSession) {
        session.statusDetail = QStringLiteral(
            "Hydra cleaned up a provider session left behind by an earlier app instance.");
    } else if (pendingResumeMetadata) {
        session.statusDetail = QStringLiteral(
            "Hydra ended the detached tmux session before provider resume metadata was resolved. Resume will unlock automatically when the provider metadata becomes visible.");
    } else {
        session.statusDetail = QStringLiteral("Hydra ended the detached tmux session.");
    }
    session.statusProvenance = StatusProvenance::UiLocal;
    session.statusSourceRef = liveTmuxSessionName;
    session.tmuxSessionName.clear();
    session.tmuxPaneId.clear();
    session.ownerInstanceId.clear();
    session.transcriptPath.clear();
    session.auditPath.clear();
    session.updatedAt = now;

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        return {.ok = false,
                .message = QStringLiteral("tmux session ended but persistence failed: %1")
                               .arg(errorMessage),
                .session = std::nullopt};
    }

    QString timelineError;
    const SessionTimelineEvent event = makeActionTimelineEvent(
        session,
        SessionSignalKind::TerminateRequested,
        QStringLiteral("Terminate"),
        session.statusDetail,
        session.statusProvenance,
        session.statusSourceRef,
        now);
    if (!m_sessionStore.appendTimelineEvent(event, kTimelineRetentionPerSession, &timelineError)) {
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist termination timeline event for %1: %2")
                   .arg(session.id, timelineError);
    }

    const SessionArtifactExportOutcome artifactExport =
        exportSessionArtifacts(session, capturedTranscript);
    if (!artifactExport.transcriptPath.isEmpty() || !artifactExport.auditPath.isEmpty()) {
        session.transcriptPath = artifactExport.transcriptPath;
        session.auditPath = artifactExport.auditPath;

        QString artifactPersistError;
        if (!m_sessionStore.upsertSession(session, &artifactPersistError)) {
            qWarning().noquote()
                << QStringLiteral("Hydra failed to persist exit artifact paths for %1: %2")
                       .arg(session.id, artifactPersistError);
        }
    }
    for (const QString &warning : artifactExport.warnings) {
        qWarning().noquote() << QStringLiteral("Hydra %1").arg(warning);
    }

    m_muxAdapter.stopWatchingSession(session.id);

    const auto persistedSession = m_sessionStore.sessionById(session.id);
    const QString baseMessage = isLive
                                    ? QStringLiteral("Ended tmux session %1.")
                                          .arg(liveTmuxSessionName)
                                    : QStringLiteral(
                                          "Session was already inactive; metadata marked exited.");
    return {.ok = true,
            .message = artifactExport.warnings.isEmpty()
                           ? baseMessage
                           : QStringLiteral("%1 Artifact export warning: %2")
                                 .arg(baseMessage,
                                      artifactExport.warnings.join(QStringLiteral(" | "))),
            .session = persistedSession};
}

SessionShutdownOutcome SessionSupervisor::shutdownOwnedSessionsForAppExit()
{
    SessionShutdownOutcome outcome;
    QVector<SessionRecord> sessions = m_sessionStore.listSessions();
    QSet<QString> claimedProviderSessionIds;
    for (const SessionRecord &session : sessions) {
        const QString providerSessionId = session.providerSessionId.trimmed();
        if (!providerSessionId.isEmpty()
            && !isPendingResumeToken(session.providerKey, providerSessionId))
        {
            claimedProviderSessionIds.insert(providerSessionId);
        }
    }

    for (SessionRecord &session : sessions) {
        if (!shouldShutdownOwnedSession(session)) {
            continue;
        }

        const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey);
        const bool supportsResume = adapter != nullptr && adapter->supportsResume();
        const QString liveTmuxSessionName = session.tmuxSessionName;
        const bool isLive = !liveTmuxSessionName.isEmpty()
                            && m_muxAdapter.hasLiveSession(liveTmuxSessionName);
        resolvePendingResumeToken(session, claimedProviderSessionIds);
        bool resumeReady = !isPendingResumeToken(session.providerKey, session.providerSessionId);
        QString capturedTranscript;
        if (isLive) {
            capturedTranscript = captureSessionTranscript(session);
        }

        if (isLive) {
            const auto terminate = m_muxAdapter.terminateSession(liveTmuxSessionName);
            if (!terminate.ok) {
                outcome.ok = false;
                outcome.errors.push_back(
                    QStringLiteral("%1: %2").arg(liveTmuxSessionName, terminate.errorMessage));
                continue;
            }
            ++outcome.terminatedCount;
        }

        if (!resumeReady) {
            resolvePendingResumeToken(session, claimedProviderSessionIds);
            resumeReady = !isPendingResumeToken(session.providerKey, session.providerSessionId);
        }

        const bool pendingResumeMetadata =
            supportsResume && isPendingResumeToken(session.providerKey, session.providerSessionId)
            && !resumeReady;

        session.state = SessionState::Exited;
        session.lastError.clear();
        if (pendingResumeMetadata) {
            session.statusDetail = QStringLiteral(
                "Hydra closed this provider session when the app exited before provider resume metadata was resolved. Resume will unlock automatically when the provider metadata becomes visible.");
        } else if (supportsResume) {
            session.statusDetail = QStringLiteral(
                "Hydra closed this provider session when the app exited. Use Resume to reopen it.");
        } else {
            session.statusDetail = QStringLiteral("Hydra closed this session when the app exited.");
        }
        session.statusProvenance = StatusProvenance::UiLocal;
        session.statusSourceRef = liveTmuxSessionName;
        session.tmuxSessionName.clear();
        session.tmuxPaneId.clear();
        session.ownerInstanceId.clear();
        session.transcriptPath.clear();
        session.auditPath.clear();
        session.updatedAt = QDateTime::currentDateTimeUtc();

        QString errorMessage;
        if (!m_sessionStore.upsertSession(session, &errorMessage)) {
            outcome.ok = false;
            outcome.errors.push_back(QStringLiteral("%1: %2").arg(session.name, errorMessage));
            continue;
        }

        if (supportsResume && !session.providerSessionId.trimmed().isEmpty()) {
            claimedProviderSessionIds.insert(session.providerSessionId.trimmed());
        }
        if (!pendingResumeMetadata) {
            ++outcome.resumedReadyCount;
        }

        QString timelineError;
        const SessionTimelineEvent event = makeActionTimelineEvent(
            session,
            SessionSignalKind::TerminateRequested,
            QStringLiteral("App close"),
            session.statusDetail,
            session.statusProvenance,
            session.statusSourceRef,
            session.updatedAt);
        if (!m_sessionStore.appendTimelineEvent(event, kTimelineRetentionPerSession, &timelineError)) {
            outcome.ok = false;
            outcome.errors.push_back(
                QStringLiteral("%1 timeline: %2").arg(session.name, timelineError));
        }

        m_muxAdapter.stopWatchingSession(session.id);
        m_pendingResumeResolutions.remove(session.id);

        const SessionArtifactExportOutcome artifactExport =
            exportSessionArtifacts(session, capturedTranscript);
        if (!artifactExport.transcriptPath.isEmpty() || !artifactExport.auditPath.isEmpty()) {
            session.transcriptPath = artifactExport.transcriptPath;
            session.auditPath = artifactExport.auditPath;

            QString artifactPersistError;
            if (!m_sessionStore.upsertSession(session, &artifactPersistError)) {
                qWarning().noquote()
                    << QStringLiteral("Hydra failed to persist shutdown artifact paths for %1: %2")
                           .arg(session.id, artifactPersistError);
            }
        }
        for (const QString &warning : artifactExport.warnings) {
            qWarning().noquote() << QStringLiteral("Hydra %1").arg(warning);
        }
    }

    return outcome;
}

SessionShutdownOutcome SessionSupervisor::cleanupForeignOwnedSessionsForStartup()
{
    SessionShutdownOutcome outcome;
    const QVector<SessionRecord> sessions = m_sessionStore.listSessions();
    for (const SessionRecord &session : sessions) {
        const QString ownerInstanceId = session.ownerInstanceId.trimmed();
        if (ownerInstanceId.isEmpty() || ownerInstanceId == m_runtimeOwnerInstanceId) {
            continue;
        }

        const QString liveTmuxSessionName = session.tmuxSessionName.trimmed();
        const bool wasLive =
            !liveTmuxSessionName.isEmpty() && m_muxAdapter.hasLiveSession(liveTmuxSessionName);

        const SessionTerminationOutcome terminateOutcome = terminateSession(session.id);
        if (!terminateOutcome.ok) {
            outcome.ok = false;
            outcome.errors.push_back(
                QStringLiteral("%1: %2").arg(session.name, terminateOutcome.message));
            continue;
        }

        if (wasLive) {
            ++outcome.terminatedCount;
        }

        if (terminateOutcome.session.has_value()
            && !isPendingResumeToken(terminateOutcome.session->providerKey,
                                     terminateOutcome.session->providerSessionId))
        {
            ++outcome.resumedReadyCount;
        }
    }

    return outcome;
}

QString SessionSupervisor::generateNextAlias() const
{
    QSet<QString> usedAliases;
    const QVector<SessionRecord> sessions = m_sessionStore.listSessions();
    for (const auto &session : sessions) {
        if (session.state != SessionState::Exited && !session.alias.isEmpty()) {
            usedAliases.insert(session.alias.toLower());
        }
    }

    for (int round = 0; round < 10; ++round) {
        const QString suffix = round == 0 ? QString() : QStringLiteral("-%1").arg(round + 1);
        for (const QString &name : kNatoAlphabet) {
            const QString candidate = name + suffix;
            if (!usedAliases.contains(candidate.toLower())) {
                return candidate;
            }
        }
    }

    return QStringLiteral("Session-%1")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(4));
}

bool SessionSupervisor::setSessionAlias(const QString &sessionId, const QString &newAlias)
{
    const QString normalized = newAlias.trimmed();
    if (normalized.isEmpty()) {
        return false;
    }

    const QVector<SessionRecord> sessions = m_sessionStore.listSessions();
    for (const auto &session : sessions) {
        if (session.id != sessionId && session.state != SessionState::Exited
            && session.alias.toLower() == normalized.toLower())
        {
            return false;
        }
    }

    for (auto session : sessions) {
        if (session.id == sessionId) {
            session.alias = normalized;
            return m_sessionStore.upsertSession(session);
        }
    }
    return false;
}

std::optional<SessionRecord> SessionSupervisor::resolveAlias(const QString &alias) const
{
    const QString normalized = alias.trimmed().toLower();
    const QVector<SessionRecord> sessions = m_sessionStore.listSessions();
    for (const auto &session : sessions) {
        if (session.state != SessionState::Exited && session.alias.toLower() == normalized) {
            return session;
        }
    }
    return std::nullopt;
}

}  // namespace hydra::domain
