#include "domain/services/worktree_manager.hpp"

#include <QDir>
#include <QFileInfo>

#include "domain/support/slug.hpp"

namespace hydra::domain {

WorktreeManager::WorktreeManager(ports::RepoStore &repoStore, ports::RepoWorkspace &repoWorkspace)
    : m_repoStore(repoStore),
      m_repoWorkspace(repoWorkspace)
{
}

RepoWorkspaceSnapshot WorktreeManager::loadWorkspace(const QString &repositoryId)
{
    const auto repository = repositoryById(repositoryId);
    if (!repository.has_value()) {
        return {.ok = false,
                .repositoryPath = QString(),
                .hydraDirectoryPath = QString(),
                .docsDirectoryPath = QString(),
                .localConfigPath = QString(),
                .gitRepository = false,
                .hydraDirectoryReady = false,
                .gitExcludeReady = false,
                .worktrees = {},
                .errorMessage = QStringLiteral("Selected repository could not be found.")};
    }

    const auto localState = m_repoWorkspace.ensureRepoLocalState(repository->path);
    if (!localState.ok) {
        return {.ok = false,
                .repositoryPath = repository->path,
                .hydraDirectoryPath = localState.hydraDirectoryPath,
                .docsDirectoryPath = localState.docsDirectoryPath,
                .localConfigPath = localState.localConfigPath,
                .gitRepository = localState.gitRepository,
                .hydraDirectoryReady = localState.hydraDirectoryReady,
                .gitExcludeReady = localState.gitExcludeReady,
                .worktrees = {},
                .errorMessage = localState.errorMessage};
    }

    const auto worktrees = m_repoWorkspace.listWorktrees(repository->path);
    if (!worktrees.ok) {
        return {.ok = false,
                .repositoryPath = localState.repositoryRootPath,
                .hydraDirectoryPath = localState.hydraDirectoryPath,
                .docsDirectoryPath = localState.docsDirectoryPath,
                .localConfigPath = localState.localConfigPath,
                .gitRepository = localState.gitRepository,
                .hydraDirectoryReady = localState.hydraDirectoryReady,
                .gitExcludeReady = localState.gitExcludeReady,
                .worktrees = {},
                .errorMessage = worktrees.errorMessage};
    }

    return {.ok = true,
            .repositoryPath = localState.repositoryRootPath,
            .hydraDirectoryPath = localState.hydraDirectoryPath,
            .docsDirectoryPath = localState.docsDirectoryPath,
            .localConfigPath = localState.localConfigPath,
            .gitRepository = localState.gitRepository,
            .hydraDirectoryReady = localState.hydraDirectoryReady,
            .gitExcludeReady = localState.gitExcludeReady,
            .worktrees = worktrees.worktrees,
            .errorMessage = QString()};
}

WorktreeCreationOutcome WorktreeManager::createWorktree(const QString &repositoryId,
                                                        const QString &branchName)
{
    const auto repository = repositoryById(repositoryId);
    if (!repository.has_value()) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Selected repository could not be found.")};
    }

    const QString trimmedBranchName = branchName.trimmed();
    if (trimmedBranchName.isEmpty()) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Branch name is required to create a worktree.")};
    }

    const auto localState = m_repoWorkspace.ensureRepoLocalState(repository->path);
    if (!localState.ok) {
        return {.ok = false, .worktree = {}, .errorMessage = localState.errorMessage};
    }
    if (!localState.gitRepository) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Worktree creation requires a Git repository.")};
    }

    ports::CreateWorktreeRequest request;
    request.repositoryPath = localState.repositoryRootPath;
    request.branchName = trimmedBranchName;
    request.worktreePath = defaultWorktreePath(localState.repositoryRootPath, trimmedBranchName);

    const auto result = m_repoWorkspace.createWorktree(request);
    if (!result.ok) {
        return {.ok = false, .worktree = {}, .errorMessage = result.errorMessage};
    }

    return {.ok = true, .worktree = result.worktree, .errorMessage = QString()};
}

std::optional<Repository> WorktreeManager::repositoryById(const QString &repositoryId) const
{
    return m_repoStore.repositoryById(repositoryId);
}

QString WorktreeManager::defaultWorktreePath(const QString &repositoryRootPath,
                                             const QString &branchName)
{
    const QFileInfo repositoryInfo(repositoryRootPath);
    const QString baseName = support::slugifyLowerDash(repositoryInfo.fileName().isEmpty()
                                                           ? QStringLiteral("repo")
                                                           : repositoryInfo.fileName());
    const QString branchSlug = support::slugifyLowerDash(branchName);
    const QString worktreeName = branchSlug.isEmpty() ? baseName : QStringLiteral("%1-%2").arg(baseName, branchSlug);

    QDir parentDirectory = repositoryInfo.dir();
    QString candidatePath = parentDirectory.filePath(worktreeName);
    if (!QFileInfo::exists(candidatePath)) {
        return candidatePath;
    }

    int suffix = 2;
    while (true) {
        const QString numberedPath = parentDirectory.filePath(
            QStringLiteral("%1-%2").arg(worktreeName).arg(suffix));
        if (!QFileInfo::exists(numberedPath)) {
            return numberedPath;
        }
        ++suffix;
    }
}

}  // namespace hydra::domain
