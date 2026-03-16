#include "ui/viewmodels/app_state.hpp"

#include <algorithm>
#include <limits>
#include <memory>

#include <QSet>
#include <QtConcurrent>

#include "domain/models/provider_identity.hpp"
#include "domain/models/session_category.hpp"
#include "ui/viewmodels/app_state_worker_runtime.hpp"

namespace hydra::ui {

namespace {

constexpr int kFastSessionMonitorIntervalMs = 2500;
constexpr int kIdleSessionMonitorIntervalMs = 10000;

bool isHomeLaunchSession(const hydra::domain::SessionRecord &session)
{
    return session.name == QStringLiteral("Home shell")
           || session.name.startsWith(QStringLiteral("Home ["));
}

bool hasResolvedResumeToken(const hydra::domain::SessionRecord &session)
{
    const QString token = session.providerSessionId.trimmed();
    if (token.isEmpty()) {
        return false;
    }
    if (session.providerKey == QStringLiteral("codex")) {
        return !token.startsWith(QStringLiteral("codex-pending-"));
    }
    if (session.providerKey == QStringLiteral("gemini")) {
        return !token.startsWith(QStringLiteral("gemini-pending-"));
    }
    return true;
}

int autosortRankForSession(const hydra::domain::SessionRecord &session)
{
    switch (session.state) {
    case hydra::domain::SessionState::Idle:
        return 0;
    case hydra::domain::SessionState::WaitingForInput:
    case hydra::domain::SessionState::AwaitingApproval:
        return 1;
    case hydra::domain::SessionState::Starting:
    case hydra::domain::SessionState::Backgrounded:
    case hydra::domain::SessionState::Error:
        return 2;
    case hydra::domain::SessionState::Thinking:
    case hydra::domain::SessionState::RunningTool:
        return 3;
    case hydra::domain::SessionState::Exited:
        break;
    }

    return 4;
}

bool sessionNeedsFastMonitor(const hydra::domain::SessionRecord &session)
{
    switch (session.state) {
    case hydra::domain::SessionState::Starting:
    case hydra::domain::SessionState::Thinking:
    case hydra::domain::SessionState::RunningTool:
    case hydra::domain::SessionState::Backgrounded:
    case hydra::domain::SessionState::Error:
        return true;
    case hydra::domain::SessionState::Idle:
    case hydra::domain::SessionState::WaitingForInput:
    case hydra::domain::SessionState::AwaitingApproval:
    case hydra::domain::SessionState::Exited:
        break;
    }

    return false;
}

}

AppState::RefreshSnapshot AppState::loadRefreshSnapshot(const QString &databasePath,
                                                        const QString &selectedRepoId,
                                                        const bool preserveEmptyRepoSelection,
                                                        const quint64 requestVersion)
{
    AppState::RefreshSnapshot snapshot;
    snapshot.requestVersion = requestVersion;

    try {
        AppStateWorkerRuntime runtime(databasePath);
        snapshot.repositories = runtime.repoRegistry().repositories();
        bool repoStillExists = false;
        for (const domain::Repository &repository : snapshot.repositories) {
            if (repository.id == selectedRepoId) {
                repoStillExists = true;
                break;
            }
        }
        if (selectedRepoId.isEmpty()) {
            snapshot.resolvedRepoId = preserveEmptyRepoSelection
                                          ? QString()
                                          : (snapshot.repositories.isEmpty()
                                                 ? QString()
                                                 : snapshot.repositories.first().id);
        } else {
            snapshot.resolvedRepoId = repoStillExists
                                          ? selectedRepoId
                                          : (snapshot.repositories.isEmpty()
                                                 ? QString()
                                                 : snapshot.repositories.first().id);
        }

        if (!snapshot.resolvedRepoId.isEmpty()) {
            snapshot.workspaceSnapshot = runtime.worktreeManager().loadWorkspace(
                snapshot.resolvedRepoId);
        }

        snapshot.sessions = runtime.sessionSupervisor().refreshSessionStates();
        QStringList sessionIds;
        sessionIds.reserve(snapshot.sessions.size());
        for (const domain::SessionRecord &session : snapshot.sessions) {
            sessionIds.push_back(session.id);
        }
        snapshot.timelineBySession = runtime.sessionSupervisor().recentTimelineBySession(
            sessionIds,
            12);
    } catch (const std::exception &exception) {
        snapshot.errorMessage = QString::fromUtf8(exception.what());
    }

    return snapshot;
}

AppState::ProviderProbeSnapshot AppState::loadProviderProbeSnapshot(const QString &databasePath,
                                                                    const QString &selectedProviderKey)
{
    AppState::ProviderProbeSnapshot snapshot;

    try {
        AppStateWorkerRuntime runtime(databasePath);
        snapshot.providerProbes = runtime.providerCatalog().probes();

        bool providerStillExists = false;
        for (const domain::ports::ProviderProbe &probe : snapshot.providerProbes) {
            if (probe.providerKey == selectedProviderKey) {
                providerStillExists = true;
                break;
            }
        }

        snapshot.resolvedProviderKey = providerStillExists ? selectedProviderKey
                                                           : runtime.providerCatalog().defaultProviderKey();
    } catch (...) {
        snapshot.providerProbes.clear();
        snapshot.resolvedProviderKey = selectedProviderKey;
    }

    return snapshot;
}

AppState::SessionMonitorSnapshot AppState::loadSessionMonitorSnapshot(const QString &databasePath,
                                                                     const quint64 requestVersion)
{
    AppState::SessionMonitorSnapshot snapshot;
    snapshot.requestVersion = requestVersion;

    try {
        thread_local QString cachedDatabasePath;
        thread_local std::unique_ptr<AppStateWorkerRuntime> cachedRuntime;
        if (!cachedRuntime || cachedDatabasePath != databasePath) {
            cachedRuntime = std::make_unique<AppStateWorkerRuntime>(databasePath);
            cachedDatabasePath = databasePath;
        }

        snapshot.sessions = cachedRuntime->sessionSupervisor().refreshSessionStates();
        QStringList sessionIds;
        sessionIds.reserve(snapshot.sessions.size());
        for (const domain::SessionRecord &session : snapshot.sessions) {
            sessionIds.push_back(session.id);
        }
        snapshot.timelineBySession = cachedRuntime->sessionSupervisor().recentTimelineBySession(
            sessionIds,
            12);
    } catch (const std::exception &exception) {
        snapshot.errorMessage = QString::fromUtf8(exception.what());
    }

    return snapshot;
}

void AppState::refresh()
{
    if (m_refreshing) {
        return;
    }

    const quint64 requestVersion = m_refreshContextVersion;
    setRefreshing(true);
    recordActivity(QStringLiteral("refresh"));
    refreshProviderCatalogAsync();
    m_refreshWatcher.setFuture(QtConcurrent::run(&AppState::loadRefreshSnapshot,
                                                 m_databasePath,
                                                 m_selectedRepoId,
                                                 m_preserveEmptyRepoSelection,
                                                 requestVersion));
}

void AppState::reload()
{
    reloadProviderCatalog();

    const QString previousSelection = m_selectedRepoId;
    m_repositories = m_repoRegistry.repositories();
    m_repoModel.setRepositories(m_repositories);

    bool selectionStillExists = false;
    for (const domain::Repository &repository : m_repositories) {
        if (repository.id == previousSelection) {
            selectionStillExists = true;
            break;
        }
    }

    if (!selectionStillExists) {
        if (previousSelection.isEmpty() && m_preserveEmptyRepoSelection) {
            m_selectedRepoId.clear();
        } else {
            m_selectedRepoId = m_repositories.isEmpty() ? QString() : m_repositories.first().id;
        }
        emit selectedRepoIdChanged();
        emit selectedRepoNameChanged();
    }

    m_repoModel.setSelectedRepositoryId(m_selectedRepoId);
    reloadSelectedRepoWorkspace();
    refreshSessionsSnapshot();
    pollSessionMonitor();
    persistSelectionState();

    emit repoCountChanged();
}

QHash<QString, domain::Repository> AppState::repositoriesById() const
{
    QHash<QString, domain::Repository> repositories;
    repositories.reserve(m_repositories.size());
    for (const domain::Repository &repository : m_repositories) {
        repositories.insert(repository.id, repository);
    }
    return repositories;
}

QVector<domain::SessionRecord> AppState::refreshSessionsSnapshot()
{
    const QVector<domain::SessionRecord> sessions = m_sessionSupervisor.refreshSessionStates();
    m_sessions = sessions;
    normalizeSessionManualOrder();
    resolveSelectedSession();
    resolveMasterSession();
    resolveRouterSession();
    QStringList sessionIds;
    sessionIds.reserve(m_sessions.size());
    for (const domain::SessionRecord &session : m_sessions) {
        sessionIds.push_back(session.id);
    }

    m_timelineBySession = m_sessionSupervisor.recentTimelineBySession(sessionIds, 12);
    syncSessionModel();
    writeSessionManifest();
    updateSessionMonitorCadence(m_sessions);
    emit selectedSessionChanged();
    emit sessionCountChanged();
    persistSelectionState();
    return m_sessions;
}

void AppState::pollSessionMonitor()
{
    if (!m_tmuxAvailable || m_refreshing || m_sessionMonitorWatcher.isRunning() || m_sessions.isEmpty()) {
        return;
    }

    const quint64 requestVersion = m_refreshContextVersion;
    m_sessionMonitorWatcher.setFuture(
        QtConcurrent::run(&m_sessionMonitorThreadPool,
                          &AppState::loadSessionMonitorSnapshot,
                          m_databasePath,
                          requestVersion));
}

void AppState::updateSessionMonitorCadence(const QVector<domain::SessionRecord> &sessions)
{
    bool hasLiveSessions = false;
    bool requiresFastMonitor = false;

    for (const domain::SessionRecord &session : sessions) {
        if (session.state == hydra::domain::SessionState::Exited) {
            continue;
        }

        hasLiveSessions = true;
        if (sessionNeedsFastMonitor(session)) {
            requiresFastMonitor = true;
            break;
        }
    }

    const int nextIntervalMs =
        hasLiveSessions && !requiresFastMonitor ? kIdleSessionMonitorIntervalMs
                                                : kFastSessionMonitorIntervalMs;
    if (m_sessionMonitorTimer.interval() != nextIntervalMs) {
        m_sessionMonitorTimer.setInterval(nextIntervalMs);
    }
}

bool AppState::hasActiveSessionForRepository(const QString &repositoryId)
{
    for (const domain::SessionRecord &session : refreshSessionsSnapshot()) {
        if (session.repoId == repositoryId && session.state != domain::SessionState::Exited) {
            return true;
        }
    }

    return false;
}

bool AppState::hasActiveSessionForWorktree(const QString &worktreePath)
{
    for (const domain::SessionRecord &session : refreshSessionsSnapshot()) {
        if (session.workingDirectory == worktreePath
            && session.state != domain::SessionState::Exited)
        {
            return true;
        }
    }

    return false;
}

void AppState::reloadProviderCatalog()
{
    refreshProviderCatalogAsync(true);
}

void AppState::reloadSelectedRepoWorkspace()
{
    applyWorkspaceSnapshot(m_selectedRepoId.isEmpty()
                               ? domain::RepoWorkspaceSnapshot{}
                               : m_worktreeManager.loadWorkspace(m_selectedRepoId));
}

void AppState::applyWorkspaceSnapshot(const domain::RepoWorkspaceSnapshot &snapshot)
{
    m_repositoryRootPath = snapshot.repositoryPath;
    m_repositoryIsGit = snapshot.gitRepository;
    m_worktrees = snapshot.worktrees;
    m_worktreeModel.setWorktrees(m_worktrees);

    QString nextSelectedWorktreePath;
    const QString previousSelectedWorktreePath = m_selectedWorktreePath;
    for (const domain::Worktree &worktree : m_worktrees) {
        if (worktree.path == previousSelectedWorktreePath) {
            nextSelectedWorktreePath = worktree.path;
            break;
        }
    }
    if (nextSelectedWorktreePath.isEmpty()) {
        for (const domain::Worktree &worktree : m_worktrees) {
            if (worktree.isMain) {
                nextSelectedWorktreePath = worktree.path;
                break;
            }
        }
    }
    if (nextSelectedWorktreePath.isEmpty()) {
        nextSelectedWorktreePath = m_repositoryRootPath;
    }

    setSelectedWorktreePathInternal(nextSelectedWorktreePath);
    emit repositoryWorkspaceChanged();
    emit worktreeCountChanged();
}

void AppState::applyRefreshSnapshot(const RefreshSnapshot &snapshot)
{
    const bool selectedRepoChanged = m_selectedRepoId != snapshot.resolvedRepoId;
    m_repositories = snapshot.repositories;
    m_repoModel.setRepositories(m_repositories);
    m_selectedRepoId = snapshot.resolvedRepoId;
    m_repoModel.setSelectedRepositoryId(m_selectedRepoId);
    if (selectedRepoChanged) {
        emit selectedRepoIdChanged();
        emit selectedRepoNameChanged();
    }

    applyWorkspaceSnapshot(snapshot.workspaceSnapshot);
    m_sessions = snapshot.sessions;
    m_timelineBySession = snapshot.timelineBySession;
    normalizeSessionManualOrder();
    resolveSelectedSession();
    resolveMasterSession();
    resolveRouterSession();
    syncSessionModel();
    writeSessionManifest();
    updateSessionMonitorCadence(m_sessions);
    emit selectedSessionChanged();
    emit repoCountChanged();
    emit sessionCountChanged();
    persistSelectionState();
    pollSessionMonitor();
}

void AppState::scheduleRefresh(const int delayMs)
{
    const int effectiveDelayMs = std::max(0, delayMs);
    if (effectiveDelayMs == 0) {
        QMetaObject::invokeMethod(this, &AppState::refresh, Qt::QueuedConnection);
        return;
    }

    QTimer::singleShot(effectiveDelayMs, this, [this]() {
        refresh();
    });
}

void AppState::refreshProviderCatalogAsync(const bool force)
{
    if (m_providerProbeWatcher.isRunning()) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    if (!force && m_lastProviderProbeRefreshAt.isValid()
        && m_lastProviderProbeRefreshAt.msecsTo(now) < 30000)
    {
        return;
    }

    m_lastProviderProbeRefreshAt = now;
    m_providerProbeWatcher.setFuture(
        QtConcurrent::run(&AppState::loadProviderProbeSnapshot,
                          m_databasePath,
                          m_selectedProviderKey));
}

void AppState::applyProviderProbeSnapshot(const ProviderProbeSnapshot &snapshot)
{
    if (snapshot.providerProbes.isEmpty()) {
        return;
    }

    m_providerProbes = snapshot.providerProbes;
    const QString resolvedProviderKey = snapshot.resolvedProviderKey.trimmed();
    if (!resolvedProviderKey.isEmpty()) {
        m_selectedProviderKey = resolvedProviderKey;
    } else if (m_selectedProviderKey.trimmed().isEmpty()) {
        m_selectedProviderKey = m_providerCatalog.firstProviderKey();
    }
    m_providerModel.setSelectedProviderKey(m_selectedProviderKey);
    m_providerModel.setProviders(m_providerProbes);
    emit launchConfigurationChanged();
}

void AppState::applySessionMonitorSnapshot(const SessionMonitorSnapshot &snapshot)
{
    if (snapshot.sessions == m_sessions && snapshot.timelineBySession == m_timelineBySession) {
        return;
    }

    m_sessions = snapshot.sessions;
    m_timelineBySession = snapshot.timelineBySession;
    normalizeSessionManualOrder();
    resolveSelectedSession();
    resolveMasterSession();
    resolveRouterSession();
    syncSessionModel();
    writeSessionManifest();
    updateSessionMonitorCadence(m_sessions);
    emit selectedSessionChanged();
    emit sessionCountChanged();
    persistSelectionState();
}

QVector<domain::SessionRecord> AppState::presentedSessions() const
{
    QHash<QString, domain::SessionRecord> liveSessionsById;
    liveSessionsById.reserve(m_sessions.size());
    QHash<QString, int> manualOrderIndex;
    manualOrderIndex.reserve(m_sessionManualOrder.size());
    for (int index = 0; index < m_sessionManualOrder.size(); ++index) {
        manualOrderIndex.insert(m_sessionManualOrder.at(index), index);
    }

    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited
            && session.category != domain::SessionCategory::Master
            && session.category != domain::SessionCategory::Router) {
            liveSessionsById.insert(session.id, session);
        }
    }

    QVector<domain::SessionRecord> sessions;
    sessions.reserve(liveSessionsById.size());
    for (const QString &sessionId : m_sessionManualOrder) {
        const auto liveSession = liveSessionsById.constFind(sessionId);
        if (liveSession != liveSessionsById.cend()) {
            sessions.push_back(*liveSession);
        }
    }

    if (m_sessionAutosortEnabled) {
        std::stable_sort(sessions.begin(),
                         sessions.end(),
                         [&manualOrderIndex](const domain::SessionRecord &lhs,
                                             const domain::SessionRecord &rhs) {
                             const int lhsRank = autosortRankForSession(lhs);
                             const int rhsRank = autosortRankForSession(rhs);
                             if (lhsRank != rhsRank) {
                                 return lhsRank < rhsRank;
                             }

                             const int lhsManualIndex =
                                 manualOrderIndex.value(lhs.id, std::numeric_limits<int>::max());
                             const int rhsManualIndex =
                                 manualOrderIndex.value(rhs.id, std::numeric_limits<int>::max());
                             if (lhsManualIndex != rhsManualIndex) {
                                 return lhsManualIndex < rhsManualIndex;
                             }

                             return lhs.updatedAt > rhs.updatedAt;
                         });
    }

    return sessions;
}

void AppState::upsertSessionInSnapshot(const domain::SessionRecord &session)
{
    if (session.id.trimmed().isEmpty()) {
        return;
    }

    bool updated = false;
    for (domain::SessionRecord &existing : m_sessions) {
        if (existing.id == session.id) {
            existing = session;
            updated = true;
            break;
        }
    }

    if (!updated) {
        m_sessions.push_back(session);
    }

    normalizeSessionManualOrder();
    resolveSelectedSession();
    resolveMasterSession();
    resolveRouterSession();
    syncSessionModel();
    writeSessionManifest();
    updateSessionMonitorCadence(m_sessions);
    emit selectedSessionChanged();
    emit sessionCountChanged();
    persistSelectionState();
}

void AppState::normalizeSessionManualOrder()
{
    QStringList nextOrder;
    nextOrder.reserve(m_sessions.size());
    QSet<QString> liveSessionIds;
    liveSessionIds.reserve(m_sessions.size());
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited) {
            liveSessionIds.insert(session.id);
        }
    }

    for (const QString &sessionId : m_sessionManualOrder) {
        if (liveSessionIds.contains(sessionId) && !nextOrder.contains(sessionId)) {
            nextOrder.push_back(sessionId);
        }
    }

    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited && !nextOrder.contains(session.id)) {
            nextOrder.push_back(session.id);
        }
    }

    m_sessionManualOrder = nextOrder;
}

QVector<ResumeListModel::Entry> AppState::presentedResumableSessions() const
{
    QVector<ResumeListModel::Entry> entries;
    entries.reserve(m_sessions.size());

    const QHash<QString, domain::Repository> reposById = repositoriesById();
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited) {
            continue;
        }

        const domain::ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey);
        if (adapter == nullptr || !adapter->supportsResume()) {
            continue;
        }
        if (!hasResolvedResumeToken(session)) {
            continue;
        }

        const auto repository = reposById.constFind(session.repoId);
        ResumeListModel::Entry entry;
        entry.sessionId = session.id;
        entry.name = session.name;
        entry.repoName = isHomeLaunchSession(session)
                             ? QStringLiteral("HOME")
                             : (repository == reposById.cend() ? QStringLiteral("Unknown repo")
                                                               : repository->name);
        entry.providerName = domain::providerDisplayNameForKey(session.providerKey);
        entry.detailText = session.workingDirectory;
        if (!session.transcriptPath.trimmed().isEmpty()) {
            entry.detailText += QStringLiteral("  //  LOG SAVED");
        }
        entry.statusDetail = session.statusDetail;
        entry.updatedAtText =
            session.updatedAt.toLocalTime().toString(QStringLiteral("MMM d, h:mm ap"));
        entries.push_back(entry);
    }

    return entries;
}

void AppState::syncSessionModel()
{
    m_resumeModel.setEntries(presentedResumableSessions());
    m_sessionModel.setSessions(presentedSessions(),
                               repositoriesById(),
                               m_timelineBySession,
                               m_selectedSessionId);
}

void AppState::removeStoredSessionsFromSnapshot(const QStringList &sessionIds)
{
    if (sessionIds.isEmpty()) {
        return;
    }

    QSet<QString> deletedIds;
    deletedIds.reserve(sessionIds.size());
    for (const QString &sessionId : sessionIds) {
        const QString trimmedId = sessionId.trimmed();
        if (!trimmedId.isEmpty()) {
            deletedIds.insert(trimmedId);
        }
    }
    if (deletedIds.isEmpty()) {
        return;
    }

    QVector<domain::SessionRecord> nextSessions;
    nextSessions.reserve(m_sessions.size());
    for (const domain::SessionRecord &session : m_sessions) {
        if (!deletedIds.contains(session.id)) {
            nextSessions.push_back(session);
        }
    }

    if (nextSessions.size() == m_sessions.size()) {
        return;
    }

    m_sessions = nextSessions;
    for (const QString &sessionId : deletedIds) {
        m_timelineBySession.remove(sessionId);
    }

    normalizeSessionManualOrder();
    resolveSelectedSession();
    resolveMasterSession();
    resolveRouterSession();
    syncSessionModel();
    writeSessionManifest();
    updateSessionMonitorCadence(m_sessions);
    emit selectedSessionChanged();
    emit sessionCountChanged();
    persistSelectionState();
}

}  // namespace hydra::ui
