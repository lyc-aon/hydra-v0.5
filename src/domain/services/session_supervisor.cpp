#include "domain/services/session_supervisor.hpp"

#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

#include "domain/models/status_provenance.hpp"
#include "domain/support/slug.hpp"

namespace hydra::domain {

namespace {

constexpr int kTimelineRetentionPerSession = 24;
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

SessionSupervisor::SessionSupervisor(ports::RepoStore &repoStore,
                                     ports::SessionStore &sessionStore,
                                     ports::MuxAdapter &muxAdapter,
                                     ports::TerminalBackend &terminalBackend,
                                     const QVector<ports::ProviderSessionResolver *> &sessionResolvers,
                                     const ProviderCatalog &providerCatalog,
                                     const StatusAggregator &statusAggregator,
                                     QString runtimeOwnerInstanceId)
    : m_repoStore(repoStore),
      m_sessionStore(sessionStore),
      m_muxAdapter(muxAdapter),
      m_terminalBackend(terminalBackend),
      m_sessionResolvers(sessionResolvers),
      m_providerCatalog(providerCatalog),
      m_statusAggregator(statusAggregator),
      m_runtimeOwnerInstanceId(std::move(runtimeOwnerInstanceId))
{
}

QVector<SessionRecord> SessionSupervisor::sessions() const
{
    return m_sessionStore.listSessions();
}

int SessionSupervisor::ownedLiveSessionCount() const
{
    int count = 0;
    for (const SessionRecord &session : m_sessionStore.listSessions()) {
        if (shouldShutdownOwnedSession(session)) {
            ++count;
        }
    }
    return count;
}

bool SessionSupervisor::shouldShutdownOwnedSession(const SessionRecord &session) const
{
    if (session.ownerInstanceId != m_runtimeOwnerInstanceId) {
        return false;
    }

    const QString liveTmuxSessionName = session.tmuxSessionName.trimmed();
    if (!liveTmuxSessionName.isEmpty() && m_muxAdapter.hasLiveSession(liveTmuxSessionName)) {
        return true;
    }

    return session.state != SessionState::Exited;
}

QHash<QString, QVector<SessionTimelineEvent>> SessionSupervisor::recentTimelineBySession(
    const QStringList &sessionIds,
    const int limitPerSession) const
{
    return m_sessionStore.recentTimelineBySession(sessionIds, limitPerSession);
}

LaunchOutcome SessionSupervisor::launchSession(const QString &repositoryId,
                                               const QString &providerKey,
                                               const LaunchSafety safety,
                                               const QString &workingDirectory,
                                               const QString &sessionContextName,
                                               const SessionCategory category,
                                               const QString &modelOverride,
                                               const QHash<QString, QString> &launchEnvironment,
                                               const QString &providerProfileMode,
                                               const QString &providerProfilePath)
{
    const auto repository = m_repoStore.repositoryById(repositoryId);
    if (!repository.has_value()) {
        return {.ok = false,
                .message = QStringLiteral("Selected repository could not be found."),
                .session = std::nullopt};
    }

    const auto probe = m_muxAdapter.probeAvailability();
    if (!probe.available) {
        return {.ok = false,
                .message = QStringLiteral("tmux is required for the first Hydra slice. %1")
                               .arg(probe.message),
                .session = std::nullopt};
    }

    const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(providerKey);
    if (adapter == nullptr) {
        return {.ok = false,
                .message = QStringLiteral("Selected provider could not be resolved."),
                .session = std::nullopt};
    }

    const ports::ProviderProbe providerProbe = adapter->probe();
    if (!providerProbe.available) {
        return {.ok = false,
                .message = providerProbe.statusMessage,
                .session = std::nullopt};
    }

    const QString contextName = sessionContextName.trimmed().isEmpty()
                                    ? repository->name
                                    : sessionContextName.trimmed();
    const QString effectiveWorkingDirectory =
        workingDirectory.trimmed().isEmpty() ? repository->path : workingDirectory.trimmed();
    const QString initialResumeToken = adapter->initialResumeToken();
    const ports::ProviderInvocationOptions invocationOptions{
        .resumeToken = initialResumeToken,
        .modelOverride = modelOverride.trimmed(),
        .sessionCategory = category,
        .providerProfileMode = providerProfileMode.trimmed(),
        .providerProfilePath = providerProfilePath.trimmed(),
        .workingDirectory = effectiveWorkingDirectory,
    };

    ports::MuxLaunchRequest launchRequest;
    launchRequest.sessionName = buildSessionName(contextName);
    launchRequest.workingDirectory = effectiveWorkingDirectory;
    const ports::ProviderLaunchPlan launchPlan = adapter->buildLaunchPlan(safety, invocationOptions);
    launchRequest.executable = launchPlan.executable;
    launchRequest.arguments = launchPlan.arguments;
    launchRequest.environment = launchEnvironment;
    for (auto it = launchPlan.environment.cbegin(); it != launchPlan.environment.cend(); ++it) {
        launchRequest.environment.insert(it.key(), it.value());
    }

    const auto launchResult = m_muxAdapter.launchDetachedSession(launchRequest);
    if (!launchResult.ok) {
        return {.ok = false, .message = launchResult.errorMessage, .session = std::nullopt};
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();

    SessionRecord session;
    session.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.repoId = repository->id;
    session.name = QStringLiteral("%1 [%2]").arg(contextName, adapter->displayName());
    session.providerKey = adapter->providerKey();
    session.providerSessionId = initialResumeToken;
    session.ownerInstanceId = m_runtimeOwnerInstanceId;
    session.launchSafetyKey = launchSafetyKey(safety);
    session.state = SessionState::Starting;
    session.tmuxSessionName = launchResult.sessionName;
    session.tmuxPaneId = launchResult.paneId;
    session.workingDirectory = launchRequest.workingDirectory;
    session.modelOverride = invocationOptions.modelOverride;
    session.launchEnvironment = launchEnvironment;
    session.providerProfileMode = invocationOptions.providerProfileMode;
    session.providerProfilePath = invocationOptions.providerProfilePath;
    session.alias = generateNextAlias();
    session.category = category;
    session.statusDetail = QStringLiteral("Hydra launched the detached tmux session.");
    session.statusProvenance = StatusProvenance::UiLocal;
    session.statusSourceRef = launchResult.sessionName;
    session.createdAt = now;
    session.updatedAt = now;

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        QString cleanupSuffix;
        if (!launchResult.sessionName.isEmpty()) {
            const auto cleanup = m_muxAdapter.terminateSession(launchResult.sessionName);
            if (!cleanup.ok) {
                cleanupSuffix =
                    QStringLiteral(" tmux cleanup also failed: %1").arg(cleanup.errorMessage);
            }
        }

        return {.ok = false,
                .message = QStringLiteral("tmux session launched but persistence failed: %1")
                               .arg(errorMessage) +
                               cleanupSuffix,
                .session = std::nullopt};
    }

    if (const ports::ProviderSessionResolver *resolver = resolverForProvider(session.providerKey)) {
        m_pendingResumeResolutions.insert(
            session.id,
            PendingResumeResolution{
                .providerKey = session.providerKey,
                .workingDirectory = session.workingDirectory,
                .knownSessionIds = resolver->knownSessionIdsForWorkingDirectory(session.workingDirectory),
            });
    }

    const QString postLaunchBootstrapText = adapter->postLaunchBootstrapText(invocationOptions).trimmed();
    if (!postLaunchBootstrapText.isEmpty()) {
        QString bootstrapError;
        const bool bootstrapOk = m_terminalBackend.pasteTextAndSubmit(session.tmuxPaneId,
                                                                      postLaunchBootstrapText,
                                                                      &bootstrapError);
        if (!bootstrapOk) {
            qWarning().noquote()
                << QStringLiteral("Hydra failed to bootstrap %1 resume metadata for %2: %3")
                       .arg(session.providerKey, session.id, bootstrapError);
        }
    }

    QString timelineError;
    const SessionTimelineEvent event = makeActionTimelineEvent(
        session,
        SessionSignalKind::LaunchRequested,
        QStringLiteral("Launch"),
        session.statusDetail,
        session.statusProvenance,
        session.statusSourceRef,
        now);
    if (!m_sessionStore.appendTimelineEvent(event, kTimelineRetentionPerSession, &timelineError)) {
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist launch timeline event for %1: %2")
                   .arg(session.id, timelineError);
    }

    if (!session.tmuxSessionName.trimmed().isEmpty()) {
        const QString paneTail =
            session.tmuxPaneId.trimmed().isEmpty() ? QString()
                                                   : m_muxAdapter.capturePaneTail(session.tmuxPaneId);
        const QString paneTitle =
            session.tmuxPaneId.trimmed().isEmpty() ? QString()
                                                   : m_muxAdapter.capturePaneTitle(session.tmuxPaneId);
        const SessionSignal signal{
            .sessionId = session.id,
            .sessionName = session.tmuxSessionName,
            .paneId = session.tmuxPaneId,
            .kind = SessionSignalKind::MonitorAttached,
            .payload = paneTail,
            .paneTail = paneTail,
            .paneTitle = paneTitle,
            .occurredAt = QDateTime::currentDateTimeUtc(),
            .provenance = StatusProvenance::Tmux,
            .sourceRef = session.tmuxPaneId.trimmed().isEmpty() ? session.tmuxSessionName
                                                                : session.tmuxPaneId,
        };
        bool hadPersistenceFailure = false;
        applyStatusSignal(session,
                          signal,
                          paneTail,
                          paneTitle,
                          adapter->observabilityHints(),
                          &hadPersistenceFailure);
    }

    return {.ok = true,
            .message = QStringLiteral("Launched %1 for %2 in tmux session %3.")
                           .arg(adapter->displayName(), contextName, launchResult.sessionName),
            .session = session};
}

LaunchOutcome SessionSupervisor::resumeSession(const QString &sessionId, const LaunchSafety safety)
{
    const auto storedSession = m_sessionStore.sessionById(sessionId.trimmed());
    if (!storedSession.has_value()) {
        return {.ok = false,
                .message = QStringLiteral("Stored session could not be found."),
                .session = std::nullopt};
    }

    SessionRecord session = storedSession.value();
    if (session.state != SessionState::Exited) {
        return {.ok = false,
                .message = QStringLiteral("Only closed sessions can be resumed."),
                .session = std::nullopt};
    }

    const QFileInfo workingDirectoryInfo(session.workingDirectory);
    if (!workingDirectoryInfo.exists() || !workingDirectoryInfo.isDir()) {
        return {.ok = false,
                .message = QStringLiteral("Resume target could not be found on disk."),
                .session = std::nullopt};
    }

    const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey);
    if (adapter == nullptr || !adapter->supportsResume()) {
        return {.ok = false,
                .message = QStringLiteral("This provider does not support session resume."),
                .session = std::nullopt};
    }

    const ports::ProviderProbe providerProbe = adapter->probe();
    if (!providerProbe.available) {
        return {.ok = false,
                .message = providerProbe.statusMessage,
                .session = std::nullopt};
    }

    const QString resumeToken = resolveResumeToken(session);
    if (resumeToken.isEmpty()) {
        return {.ok = false,
                .message = QStringLiteral("Provider resume metadata is not available for this session yet."),
                .session = std::nullopt};
    }

    ports::MuxLaunchRequest launchRequest;
    launchRequest.sessionName = buildSessionName(session.name);
    launchRequest.workingDirectory = session.workingDirectory;
    const ports::ProviderLaunchPlan launchPlan = adapter->buildResumePlan(
        safety,
        ports::ProviderInvocationOptions{
            .resumeToken = resumeToken,
            .modelOverride = session.modelOverride,
            .sessionCategory = session.category,
            .providerProfileMode = session.providerProfileMode,
            .providerProfilePath = session.providerProfilePath,
            .workingDirectory = session.workingDirectory,
        });
    launchRequest.executable = launchPlan.executable;
    launchRequest.arguments = launchPlan.arguments;
    launchRequest.environment = session.launchEnvironment;
    for (auto it = launchPlan.environment.cbegin(); it != launchPlan.environment.cend(); ++it) {
        launchRequest.environment.insert(it.key(), it.value());
    }

    const auto launchResult = m_muxAdapter.launchDetachedSession(launchRequest);
    if (!launchResult.ok) {
        return {.ok = false,
                .message = launchResult.errorMessage,
                .session = std::nullopt};
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    session.providerSessionId = resumeToken;
    session.ownerInstanceId = m_runtimeOwnerInstanceId;
    session.launchSafetyKey = launchSafetyKey(safety);
    session.state = SessionState::Starting;
    session.tmuxSessionName = launchResult.sessionName;
    session.tmuxPaneId = launchResult.paneId;
    session.lastError.clear();
    session.transcriptPath.clear();
    session.auditPath.clear();
    session.statusDetail = QStringLiteral("Hydra resumed the provider conversation in a new tmux session.");
    session.statusProvenance = StatusProvenance::UiLocal;
    session.statusSourceRef = resumeToken;
    session.updatedAt = now;

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        const auto cleanup = m_muxAdapter.terminateSession(launchResult.sessionName);
        const QString cleanupSuffix = cleanup.ok
                                          ? QString()
                                          : QStringLiteral(" tmux cleanup also failed: %1")
                                                .arg(cleanup.errorMessage);
        return {.ok = false,
                .message = QStringLiteral("tmux session launched but resume persistence failed: %1")
                               .arg(errorMessage) +
                               cleanupSuffix,
                .session = std::nullopt};
    }

    QString timelineError;
    const SessionTimelineEvent event = makeActionTimelineEvent(
        session,
        SessionSignalKind::LaunchRequested,
        QStringLiteral("Resume"),
        session.statusDetail,
        session.statusProvenance,
        session.statusSourceRef,
        now);
    if (!m_sessionStore.appendTimelineEvent(event, kTimelineRetentionPerSession, &timelineError)) {
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist resume timeline event for %1: %2")
                   .arg(session.id, timelineError);
    }

    if (!session.tmuxSessionName.trimmed().isEmpty()) {
        const QString paneTail =
            session.tmuxPaneId.trimmed().isEmpty() ? QString()
                                                   : m_muxAdapter.capturePaneTail(session.tmuxPaneId);
        const QString paneTitle =
            session.tmuxPaneId.trimmed().isEmpty() ? QString()
                                                   : m_muxAdapter.capturePaneTitle(session.tmuxPaneId);
        const SessionSignal signal{
            .sessionId = session.id,
            .sessionName = session.tmuxSessionName,
            .paneId = session.tmuxPaneId,
            .kind = SessionSignalKind::MonitorAttached,
            .payload = paneTail,
            .paneTail = paneTail,
            .paneTitle = paneTitle,
            .occurredAt = QDateTime::currentDateTimeUtc(),
            .provenance = StatusProvenance::Tmux,
            .sourceRef = session.tmuxPaneId.trimmed().isEmpty() ? session.tmuxSessionName
                                                                : session.tmuxPaneId,
        };
        bool hadPersistenceFailure = false;
        applyStatusSignal(session,
                          signal,
                          paneTail,
                          paneTitle,
                          adapter->observabilityHints(),
                          &hadPersistenceFailure);
    }

    m_pendingResumeResolutions.remove(session.id);
    return {.ok = true,
            .message = QStringLiteral("Resumed %1 in tmux session %2.")
                           .arg(session.name, launchResult.sessionName),
            .session = session};
}

QString SessionSupervisor::buildSessionName(const QString &baseName)
{
    QString slug = support::slugifyLowerDash(baseName);
    if (slug.isEmpty()) {
        slug = QStringLiteral("repo");
    }

    const QString suffix = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    return QStringLiteral("hydra-%1-%2").arg(slug, suffix);
}

}  // namespace hydra::domain
