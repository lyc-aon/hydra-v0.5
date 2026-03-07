#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/models/worktree.hpp"
#include "domain/ports/repo_store.hpp"
#include "domain/ports/repo_workspace.hpp"

namespace hydra::domain {

struct RepoWorkspaceSnapshot {
    bool ok = false;
    QString repositoryPath;
    QString hydraDirectoryPath;
    QString docsDirectoryPath;
    QString localConfigPath;
    bool gitRepository = false;
    bool hydraDirectoryReady = false;
    bool gitExcludeReady = false;
    QVector<Worktree> worktrees;
    QString errorMessage;
};

struct WorktreeCreationOutcome {
    bool ok = false;
    Worktree worktree;
    QString errorMessage;
};

class WorktreeManager final {
public:
    WorktreeManager(ports::RepoStore &repoStore, ports::RepoWorkspace &repoWorkspace);

    RepoWorkspaceSnapshot loadWorkspace(const QString &repositoryId);
    WorktreeCreationOutcome createWorktree(const QString &repositoryId,
                                           const QString &branchName);

private:
    std::optional<Repository> repositoryById(const QString &repositoryId) const;
    static QString defaultWorktreePath(const QString &repositoryRootPath,
                                       const QString &branchName);

    ports::RepoStore &m_repoStore;
    ports::RepoWorkspace &m_repoWorkspace;
};

}  // namespace hydra::domain
