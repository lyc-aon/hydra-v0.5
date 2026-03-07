#pragma once

#include <QString>
#include <QStringList>

#include "domain/ports/repo_workspace.hpp"
#include "infrastructure/process/process_runner.hpp"

namespace hydra::infrastructure {

class GitRepoWorkspace final : public domain::ports::RepoWorkspace {
public:
    GitRepoWorkspace();

    domain::ports::RepoLocalStateResult ensureRepoLocalState(const QString &repositoryPath) override;
    domain::ports::WorktreeListResult listWorktrees(const QString &repositoryPath) const override;
    domain::ports::CreateWorktreeResult createWorktree(
        const domain::ports::CreateWorktreeRequest &request) override;

private:
    struct GitContext {
        bool ok = false;
        bool gitRepository = false;
        QString repositoryRootPath;
        QString gitDirectoryPath;
        QString errorMessage;
    };

    using CommandResult = process::ProcessResult;

    CommandResult runGit(const QString &workingDirectory, const QStringList &arguments) const;
    GitContext resolveGitContext(const QString &repositoryPath) const;
    static QString toAbsolutePath(const QString &baseDirectory, const QString &path);
    static QString branchNameFromRef(const QString &ref);
    static QString normalizedPath(const QString &path);
    static domain::Worktree worktreeFromPorcelainBlock(const QStringList &blockLines,
                                                       const QString &repositoryRootPath);

    QString m_gitPath;
};

}  // namespace hydra::infrastructure
