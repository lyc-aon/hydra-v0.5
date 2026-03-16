#pragma once

#include <optional>

#include <QSet>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include "domain/models/launch_safety.hpp"
#include "domain/models/session_category.hpp"
#include "domain/models/session_record.hpp"
#include "domain/models/session_signal.hpp"
#include "domain/models/session_timeline_event.hpp"
#include "domain/ports/mux_adapter.hpp"
#include "domain/ports/terminal_backend.hpp"
#include "domain/ports/repo_store.hpp"
#include "domain/ports/provider_session_resolver.hpp"
#include "domain/ports/session_store.hpp"
#include "domain/services/provider_catalog.hpp"
#include "domain/services/status_aggregator.hpp"

namespace hydra::domain {

struct LaunchOutcome {
    bool ok = false;
    QString message;
    std::optional<SessionRecord> session;
};

struct SessionTerminationOutcome {
    bool ok = false;
    QString message;
    std::optional<SessionRecord> session;
};

struct SessionShutdownOutcome {
    bool ok = true;
    int terminatedCount = 0;
    int resumedReadyCount = 0;
    QStringList errors;
};

struct SessionDeletionOutcome {
    bool ok = false;
    int deletedCount = 0;
    QString message;
};

class SessionSupervisor final {
public:
    SessionSupervisor(ports::RepoStore &repoStore,
                      ports::SessionStore &sessionStore,
                      ports::MuxAdapter &muxAdapter,
                      ports::TerminalBackend &terminalBackend,
                      const QVector<ports::ProviderSessionResolver *> &sessionResolvers,
                      const ProviderCatalog &providerCatalog,
                      const StatusAggregator &statusAggregator,
                      QString runtimeOwnerInstanceId);

    QVector<SessionRecord> sessions() const;
    QVector<SessionRecord> refreshSessionStates();
    QHash<QString, QVector<SessionTimelineEvent>> recentTimelineBySession(
        const QStringList &sessionIds,
        int limitPerSession) const;
    LaunchOutcome launchSession(const QString &repositoryId,
                                const QString &providerKey,
                                LaunchSafety safety,
                                const QString &workingDirectory = QString(),
                                const QString &sessionContextName = QString(),
                                SessionCategory category = SessionCategory::Worker,
                                const QString &modelOverride = QString(),
                                const QHash<QString, QString> &launchEnvironment = {},
                                const QString &providerProfileMode = QString(),
                                const QString &providerProfilePath = QString());
    LaunchOutcome resumeSession(const QString &sessionId, LaunchSafety safety);
    SessionDeletionOutcome deleteStoredSessions(const QStringList &sessionIds);
    SessionDeletionOutcome clearClosedResumableSessions();
    SessionTerminationOutcome terminateSession(const QString &sessionId);
    SessionShutdownOutcome shutdownOwnedSessionsForAppExit();
    SessionShutdownOutcome cleanupForeignOwnedSessionsForStartup();
    int ownedLiveSessionCount() const;
    bool setSessionAlias(const QString &sessionId, const QString &newAlias);
    std::optional<SessionRecord> resolveAlias(const QString &alias) const;

private:
    struct PendingResumeResolution {
        QString providerKey;
        QString workingDirectory;
        QSet<QString> knownSessionIds;
    };

    struct SessionArtifactExportOutcome {
        QString transcriptPath;
        QString auditPath;
        QStringList warnings;
    };

    static QString buildSessionName(const QString &baseName);
    static bool isPendingResumeToken(const QString &providerKey, const QString &resumeToken);
    QString generateNextAlias() const;
    bool shouldShutdownOwnedSession(const SessionRecord &session) const;

    ports::ProviderObservabilityHints hintsForProvider(const QString &providerKey) const;
    const ports::ProviderSessionResolver *resolverForProvider(const QString &providerKey) const;
    QSet<QString> claimedProviderSessionIdsExcluding(const QString &sessionId) const;
    QString resolveResumeToken(const SessionRecord &session,
                               const QSet<QString> &excludedSessionIds = {});
    bool resolvePendingResumeToken(SessionRecord &session,
                                   const QSet<QString> &excludedSessionIds = {});
    QString transcriptCaptureTarget(const SessionRecord &session) const;
    QString captureSessionTranscript(const SessionRecord &session) const;
    SessionArtifactExportOutcome exportSessionArtifacts(const SessionRecord &session,
                                                        const QString &capturedTranscript) const;
    bool applyStatusSignal(SessionRecord &session,
                           const SessionSignal &signal,
                           const QString &paneTail,
                           const QString &paneTitle,
                           const ports::ProviderObservabilityHints &hints,
                           bool *hadPersistenceFailure = nullptr);

    ports::RepoStore &m_repoStore;
    ports::SessionStore &m_sessionStore;
    ports::MuxAdapter &m_muxAdapter;
    ports::TerminalBackend &m_terminalBackend;
    QVector<ports::ProviderSessionResolver *> m_sessionResolvers;
    const ProviderCatalog &m_providerCatalog;
    const StatusAggregator &m_statusAggregator;
    QString m_runtimeOwnerInstanceId;
    QHash<QString, PendingResumeResolution> m_pendingResumeResolutions;
};

}  // namespace hydra::domain
