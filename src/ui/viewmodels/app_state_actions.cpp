#include "ui/viewmodels/app_state.hpp"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>

#include "domain/models/session_category.hpp"

namespace hydra::ui {

namespace {

bool writeMasterClaudeSettings(const QString &workspaceDir)
{
    QDir dir(workspaceDir);
    if (!dir.mkpath(QStringLiteral(".claude"))) {
        return false;
    }

    QJsonArray allow;
    allow.append(QStringLiteral("Bash(*)"));

    QJsonArray deny;
    deny.append(QStringLiteral("Read(*)"));
    deny.append(QStringLiteral("Edit(*)"));
    deny.append(QStringLiteral("Write(*)"));
    deny.append(QStringLiteral("Glob(*)"));
    deny.append(QStringLiteral("Grep(*)"));
    deny.append(QStringLiteral("WebFetch(*)"));
    deny.append(QStringLiteral("WebSearch(*)"));
    deny.append(QStringLiteral("NotebookEdit(*)"));
    deny.append(QStringLiteral("Task(*)"));

    QJsonObject permissions;
    permissions.insert(QStringLiteral("allow"), allow);
    permissions.insert(QStringLiteral("deny"), deny);

    QJsonObject root;
    root.insert(QStringLiteral("permissions"), permissions);
    root.insert(QStringLiteral("effortLevel"), QStringLiteral("low"));

    QFile file(dir.filePath(QStringLiteral(".claude/settings.local.json")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

}  // namespace

void AppState::ensureMasterSession()
{
    if (findMasterSession() != nullptr) {
        return;
    }

    QString launchRepoId = m_selectedRepoId;
    if (launchRepoId.isEmpty() && !m_repositories.isEmpty()) {
        launchRepoId = m_repositories.first().id;
    }
    if (launchRepoId.isEmpty()) {
        showWarningStatus(QStringLiteral("Add a repository before launching the master terminal."));
        return;
    }

    const QString effectiveProviderKey = m_masterProviderKey.isEmpty()
                                             ? m_selectedProviderKey
                                             : m_masterProviderKey;
    const domain::LaunchSafety effectiveSafety = m_masterProviderKey.isEmpty()
                                                     ? m_selectedLaunchSafety
                                                     : m_masterLaunchSafety;
    const QString effectiveModelId = m_masterModelId.trimmed();

    const QString effectiveSysprompt = m_masterSysprompt.trimmed().isEmpty()
                                           ? defaultMasterSysprompt()
                                           : m_masterSysprompt;

    const auto *adapter = m_providerCatalog.adapterFor(effectiveProviderKey);
    if (adapter == nullptr) {
        showWarningStatus(QStringLiteral("Selected master provider could not be resolved."));
        return;
    }

    const QString workingDirectory = masterWorkspacePath();
    if (!stageProviderControlWorkspace(workingDirectory)) {
        showWarningStatus(QStringLiteral("Hydra could not stage the master control workspace."));
        return;
    }
    if (!adapter->prepareSystemPromptWorkspace(workingDirectory, effectiveSysprompt)) {
        showWarningStatus(QStringLiteral("Hydra could not prepare the master prompt workspace for %1.")
                              .arg(adapter->displayName()));
        return;
    }
    if (effectiveProviderKey == QStringLiteral("claude")
        && !writeMasterClaudeSettings(workingDirectory))
    {
        showWarningStatus(QStringLiteral("Hydra could not apply Claude master control settings."));
        return;
    }

    QHash<QString, QString> launchEnvironment = buildProviderLaunchEnvironment(
        effectiveProviderKey,
        m_masterApiKeyEnvVarName,
        m_masterApiKeyValue);
    const QHash<QString, QString> promptEnvironment = adapter->promptEnvironment(effectiveSysprompt);
    for (auto it = promptEnvironment.cbegin(); it != promptEnvironment.cend(); ++it) {
        launchEnvironment.insert(it.key(), it.value());
    }
    const QString masterHermesProfileMode = providerUsesHermesProfiles(effectiveProviderKey)
                                                ? m_masterHermesProfileMode
                                                : QString();
    const QString masterHermesProfilePath = providerUsesHermesProfiles(effectiveProviderKey)
                                                ? resolvedHermesProfilePath(
                                                      launchRepoId,
                                                      m_masterHermesProfilePath)
                                                : QString();

    const domain::LaunchOutcome outcome = m_sessionSupervisor.launchSession(
        launchRepoId,
        effectiveProviderKey,
        effectiveSafety,
        workingDirectory,
        QStringLiteral("Master"),
        domain::SessionCategory::Master,
        effectiveModelId,
        launchEnvironment,
        masterHermesProfileMode,
        masterHermesProfilePath);

    if (outcome.ok) {
        ++m_refreshContextVersion;
        showInfoStatus(outcome.message);
        if (outcome.session.has_value()) {
            upsertSessionInSnapshot(*outcome.session);
        }
        recordActivity(QStringLiteral("master-launch"));
        scheduleRefresh(1500);
    } else {
        showWarningStatus(outcome.message);
    }
}

void AppState::launchSelectedRepoSession()
{
    QString launchOwnerRepoId = m_selectedRepoId;
    QString workingDirectory;
    QString sessionContextName;

    if (m_launchInHomeDirectory) {
        if (launchOwnerRepoId.isEmpty() && !m_repositories.isEmpty()) {
            launchOwnerRepoId = m_repositories.first().id;
        }
        if (launchOwnerRepoId.isEmpty()) {
            showWarningStatus(QStringLiteral("Add a repository before launching an unbound home session."));
            return;
        }
        workingDirectory = QDir::homePath();
        sessionContextName = QStringLiteral("Home");
    } else {
        if (!ensureRepositorySelected(QStringLiteral("Select a repository before launching a session."))) {
            return;
        }

        workingDirectory = !m_selectedWorktreePath.isEmpty()
                               ? m_selectedWorktreePath
                               : m_repositoryRootPath;
    }

    const domain::LaunchOutcome outcome = m_sessionSupervisor.launchSession(
        launchOwnerRepoId,
        m_selectedProviderKey,
        m_selectedLaunchSafety,
        workingDirectory,
        sessionContextName,
        domain::SessionCategory::Worker,
        m_selectedModelId,
        {},
        providerUsesHermesProfiles(m_selectedProviderKey) ? m_selectedHermesProfileMode : QString(),
        providerUsesHermesProfiles(m_selectedProviderKey)
            ? resolvedHermesProfilePath(launchOwnerRepoId, m_selectedHermesProfilePath)
            : QString());
    if (outcome.ok) {
        ++m_refreshContextVersion;
        showInfoStatus(outcome.message);
        if (outcome.session.has_value()) {
            upsertSessionInSnapshot(*outcome.session);
            setSelectedSessionId(outcome.session->id);
        }
    } else {
        showWarningStatus(outcome.message);
    }
    if (outcome.ok) {
        recordActivity(QStringLiteral("launch"));
        scheduleRefresh(1500);
    }
}

bool AppState::addRepositoryPath(const QString &repoPath)
{
    const QString trimmedRepoPath = repoPath.trimmed();
    if (trimmedRepoPath.isEmpty()) {
        showWarningStatus(QStringLiteral("Enter a folder path to add it to Hydra's registry."));
        return false;
    }

    const QFileInfo info(trimmedRepoPath);
    if (!info.exists() || !info.isDir()) {
        showWarningStatus(QStringLiteral("Repository folder could not be found."));
        return false;
    }

    const QString normalizedPath = QDir::cleanPath(info.absoluteFilePath());
    const QString repoName =
        info.fileName().trimmed().isEmpty() ? normalizedPath : info.fileName().trimmed();

    const domain::RepoRegistry::EnsureRepositoryOutcome outcome =
        m_repoRegistry.ensureRepository(normalizedPath,
                                       repoName,
                                       QStringLiteral(""),
                                       QStringLiteral("#B38A59"));
    if (!outcome.ok) {
        showWarningStatus(QStringLiteral("Failed to add repository to Hydra: %1")
                              .arg(outcome.errorMessage));
        return false;
    }

    reload();
    setSelectedRepoId(outcome.repository.id);
    showInfoStatus(outcome.created
                       ? QStringLiteral("Added %1 to the Hydra registry.")
                             .arg(outcome.repository.name)
                       : QStringLiteral("%1 is already tracked by Hydra.")
                             .arg(outcome.repository.name));
    return true;
}

void AppState::clearSelectedTarget()
{
    if (m_selectedRepoId.isEmpty() && m_selectedWorktreePath.isEmpty()) {
        setLaunchInHomeDirectory(true);
        return;
    }

    m_preserveEmptyRepoSelection = true;
    setLaunchInHomeDirectory(true);
    setSelectedRepoId(QString());
    showInfoStatus(QStringLiteral("Unbound the active target. Launches will now start in your home directory."));
}

bool AppState::createWorktree(const QString &branchName)
{
    if (!ensureRepositorySelected(QStringLiteral("Select a repository before creating a worktree."))) {
        return false;
    }

    const domain::WorktreeCreationOutcome outcome =
        m_worktreeManager.createWorktree(m_selectedRepoId, branchName);
    if (!outcome.ok) {
        showWarningStatus(outcome.errorMessage);
        return false;
    }

    reloadSelectedRepoWorkspace();
    setSelectedWorktreePathInternal(outcome.worktree.path);
    ++m_refreshContextVersion;
    showInfoStatus(QStringLiteral("Created worktree %1 for branch %2.")
                       .arg(outcome.worktree.path, outcome.worktree.branchName));
    recordActivity(QStringLiteral("worktree"));
    return true;
}

bool AppState::deleteRepository(const QString &repositoryId)
{
    const QString trimmedRepositoryId = repositoryId.trimmed();
    const domain::Repository *repository = findRepository(trimmedRepositoryId);
    if (trimmedRepositoryId.isEmpty() || repository == nullptr) {
        showWarningStatus(QStringLiteral("Repository could not be found."));
        return false;
    }
    const QString repositoryName = repository->name;

    if (hasActiveSessionForRepository(trimmedRepositoryId)) {
        showWarningStatus(
            QStringLiteral("End active tmux sessions for %1 before removing it from Hydra.")
                .arg(repositoryName));
        return false;
    }

    QString errorMessage;
    if (!m_repoRegistry.deleteRepository(trimmedRepositoryId, &errorMessage)) {
        showWarningStatus(QStringLiteral("Failed to remove repository from Hydra: %1")
                              .arg(errorMessage));
        return false;
    }

    reload();
    ++m_refreshContextVersion;
    showInfoStatus(QStringLiteral("Removed %1 from the Hydra registry.")
                       .arg(repositoryName));
    recordActivity(QStringLiteral("target"));
    return true;
}

bool AppState::deleteWorktree(const QString &worktreePath)
{
    const QString trimmedWorktreePath = worktreePath.trimmed();
    if (!ensureRepositorySelected(QStringLiteral("Select a repository before removing a worktree."))) {
        return false;
    }
    if (trimmedWorktreePath.isEmpty()) {
        showWarningStatus(QStringLiteral("Worktree could not be found."));
        return false;
    }

    if (hasActiveSessionForWorktree(trimmedWorktreePath)) {
        showWarningStatus(
            QStringLiteral("End active tmux sessions in this worktree before removing it."));
        return false;
    }

    const domain::WorktreeRemovalOutcome outcome =
        m_worktreeManager.removeWorktree(m_selectedRepoId, trimmedWorktreePath);
    if (!outcome.ok) {
        showWarningStatus(outcome.errorMessage);
        return false;
    }

    reloadSelectedRepoWorkspace();
    ++m_refreshContextVersion;
    showInfoStatus(QStringLiteral("Removed linked worktree %1.")
                       .arg(outcome.worktree.branchName));
    recordActivity(QStringLiteral("worktree"));
    return true;
}

void AppState::resumeSession(const QString &sessionId)
{
    const domain::LaunchOutcome outcome =
        m_sessionSupervisor.resumeSession(sessionId, m_selectedLaunchSafety);
    if (outcome.ok) {
        ++m_refreshContextVersion;
        showInfoStatus(outcome.message);
        if (outcome.session.has_value()) {
            upsertSessionInSnapshot(*outcome.session);
            setSelectedSessionId(outcome.session->id);
        }
        recordActivity(QStringLiteral("resume"));
        scheduleRefresh(1500);
        return;
    }

    showWarningStatus(outcome.message);
}

int AppState::deleteResumeSessions(const QVariantList &sessionIds)
{
    QStringList normalizedIds;
    normalizedIds.reserve(sessionIds.size());
    for (const QVariant &sessionId : sessionIds) {
        const QString trimmedId = sessionId.toString().trimmed();
        if (!trimmedId.isEmpty()) {
            normalizedIds.push_back(trimmedId);
        }
    }

    const domain::SessionDeletionOutcome outcome = m_sessionSupervisor.deleteStoredSessions(normalizedIds);
    if (!outcome.ok) {
        showWarningStatus(outcome.message);
        return 0;
    }

    if (outcome.deletedCount > 0) {
        ++m_refreshContextVersion;
        recordActivity(QStringLiteral("resume-delete"));
        removeStoredSessionsFromSnapshot(normalizedIds);
    }

    showInfoStatus(outcome.message);
    return outcome.deletedCount;
}

int AppState::clearAllResumableSessions()
{
    const domain::SessionDeletionOutcome outcome = m_sessionSupervisor.clearClosedResumableSessions();
    if (!outcome.ok) {
        showWarningStatus(outcome.message);
        return 0;
    }

    if (outcome.deletedCount > 0) {
        ++m_refreshContextVersion;
        recordActivity(QStringLiteral("resume-delete"));
        QStringList deletedIds;
        deletedIds.reserve(m_sessions.size());
        for (const domain::SessionRecord &session : m_sessions) {
            if (session.state != domain::SessionState::Exited) {
                continue;
            }
            const domain::ports::ProviderAdapter *adapter =
                m_providerCatalog.adapterFor(session.providerKey);
            if (adapter != nullptr && adapter->supportsResume()) {
                deletedIds.push_back(session.id);
            }
        }
        removeStoredSessionsFromSnapshot(deletedIds);
    }

    showInfoStatus(outcome.message);
    return outcome.deletedCount;
}

void AppState::terminateSession(const QString &sessionId)
{
    const domain::SessionTerminationOutcome outcome =
        m_sessionSupervisor.terminateSession(sessionId);
    if (outcome.ok) {
        ++m_refreshContextVersion;
        showInfoStatus(outcome.message);
        if (outcome.session.has_value()) {
            upsertSessionInSnapshot(*outcome.session);
        }
    } else {
        showWarningStatus(outcome.message);
    }
    if (outcome.ok) {
        recordActivity(QStringLiteral("terminate"));
        scheduleRefresh();
    }
}

bool AppState::shutdownOwnedSessionsForAppExit()
{
    const domain::SessionShutdownOutcome outcome = m_sessionSupervisor.shutdownOwnedSessionsForAppExit();
    if (!outcome.ok) {
        ++m_refreshContextVersion;
        refreshSessionsSnapshot();
        showWarningStatus(QStringLiteral("Hydra could not close all active sessions cleanly: %1")
                              .arg(outcome.errors.join(QStringLiteral(" | "))),
                          5200);
        return false;
    }

    ++m_refreshContextVersion;
    refreshSessionsSnapshot();
    return true;
}

bool AppState::ensureRepositorySelected(const QString &warningMessage)
{
    if (!m_selectedRepoId.isEmpty()) {
        return true;
    }

    showWarningStatus(warningMessage);
    return false;
}

}  // namespace hydra::ui
