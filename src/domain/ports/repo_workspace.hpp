#pragma once

#include <QString>
#include <QVector>

#include "domain/models/worktree.hpp"

namespace hydra::domain::ports {

struct RepoLocalStateResult {
    bool ok = false;
    bool gitRepository = false;
    QString repositoryRootPath;
    QString hydraDirectoryPath;
    QString docsDirectoryPath;
    QString localConfigPath;
    bool hydraDirectoryReady = false;
    bool gitExcludeReady = false;
    QString errorMessage;
};

struct WorktreeListResult {
    bool ok = false;
    QVector<domain::Worktree> worktrees;
    QString errorMessage;
};

struct CreateWorktreeRequest {
    QString repositoryPath;
    QString branchName;
    QString worktreePath;
};

struct CreateWorktreeResult {
    bool ok = false;
    domain::Worktree worktree;
    QString errorMessage;
};

class RepoWorkspace {
public:
    virtual ~RepoWorkspace() = default;

    virtual RepoLocalStateResult ensureRepoLocalState(const QString &repositoryPath) = 0;
    virtual WorktreeListResult listWorktrees(const QString &repositoryPath) const = 0;
    virtual CreateWorktreeResult createWorktree(const CreateWorktreeRequest &request) = 0;
};

}  // namespace hydra::domain::ports
