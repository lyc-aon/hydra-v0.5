#include "infrastructure/git/git_repo_workspace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTextStream>

namespace hydra::infrastructure {
namespace {

QString trimmedOutput(const QString &value)
{
    return value.trimmed();
}

domain::ports::RepoLocalStateResult repoLocalStateResult(
    const bool ok,
    const bool gitRepository,
    const QString &repositoryRootPath,
    const QString &hydraDirectoryPath,
    const QString &docsDirectoryPath,
    const QString &localConfigPath,
    const bool hydraDirectoryReady,
    const bool gitExcludeReady,
    const QString &errorMessage)
{
    return {
        .ok = ok,
        .gitRepository = gitRepository,
        .repositoryRootPath = repositoryRootPath,
        .hydraDirectoryPath = hydraDirectoryPath,
        .docsDirectoryPath = docsDirectoryPath,
        .localConfigPath = localConfigPath,
        .hydraDirectoryReady = hydraDirectoryReady,
        .gitExcludeReady = gitExcludeReady,
        .errorMessage = errorMessage,
    };
}

QStringList hydraDirectories(const QString &hydraDirectoryPath, const QString &docsDirectoryPath)
{
    return {
        hydraDirectoryPath,
        QDir(hydraDirectoryPath).filePath(QStringLiteral("providers")),
        docsDirectoryPath,
        QDir(hydraDirectoryPath).filePath(QStringLiteral("heartbeats")),
        QDir(hydraDirectoryPath).filePath(QStringLiteral("session-templates")),
    };
}

bool ensureDirectories(const QStringList &directoryPaths, QString *failedPath)
{
    for (const QString &directoryPath : directoryPaths) {
        if (QDir().mkpath(directoryPath)) {
            continue;
        }

        if (failedPath != nullptr) {
            *failedPath = directoryPath;
        }
        return false;
    }

    return true;
}

bool ensureLocalConfigFile(const QString &localConfigPath, QString *errorMessage)
{
    if (QFileInfo::exists(localConfigPath)) {
        return true;
    }

    QFile localConfigFile(localConfigPath);
    if (!localConfigFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to create Hydra local config: %1")
                                .arg(localConfigPath);
        }
        return false;
    }

    const QJsonObject payload{{QStringLiteral("version"), 1}};
    localConfigFile.write(QJsonDocument(payload).toJson(QJsonDocument::Indented));
    localConfigFile.close();
    return true;
}

bool hasExcludeEntry(const QString &existingText, const QString &excludeEntry)
{
    const QStringList existingLines = existingText.split('\n');
    for (const QString &line : existingLines) {
        if (line.trimmed() == excludeEntry) {
            return true;
        }
    }

    return false;
}

bool ensureExcludeEntry(const QString &excludePath, QString *errorMessage)
{
    QFileInfo excludeInfo(excludePath);
    if (!QDir().mkpath(excludeInfo.absolutePath())) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to prepare Git exclude path: %1").arg(excludePath);
        }
        return false;
    }

    QFile excludeFile(excludePath);
    QString existingText;
    if (excludeFile.exists()) {
        if (!excludeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to read Git exclude file: %1").arg(excludePath);
            }
            return false;
        }
        existingText = QString::fromUtf8(excludeFile.readAll());
        excludeFile.close();
    }

    const QString excludeEntry = QStringLiteral(".hydra/");
    if (hasExcludeEntry(existingText, excludeEntry)) {
        return true;
    }

    if (!excludeFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to update Git exclude file: %1").arg(excludePath);
        }
        return false;
    }

    QTextStream stream(&excludeFile);
    if (!existingText.isEmpty() && !existingText.endsWith('\n')) {
        stream << '\n';
    }
    stream << excludeEntry << '\n';
    excludeFile.close();
    return true;
}

QStringList createWorktreeArguments(const QString &branchName,
                                    const QString &worktreePath,
                                    const bool branchExists,
                                    const bool hasHead)
{
    QStringList arguments = {QStringLiteral("worktree"), QStringLiteral("add")};
    if (branchExists) {
        arguments << worktreePath << branchName;
        return arguments;
    }

    if (hasHead) {
        arguments << QStringLiteral("-b") << branchName << worktreePath << QStringLiteral("HEAD");
        return arguments;
    }

    arguments << QStringLiteral("--orphan") << QStringLiteral("-b") << branchName << worktreePath;
    return arguments;
}

}  // namespace

GitRepoWorkspace::GitRepoWorkspace()
    : m_gitPath(QStandardPaths::findExecutable(QStringLiteral("git")))
{
}

domain::ports::RepoLocalStateResult GitRepoWorkspace::ensureRepoLocalState(
    const QString &repositoryPath)
{
    const QFileInfo repositoryInfo(repositoryPath);
    if (!repositoryInfo.exists() || !repositoryInfo.isDir()) {
        return repoLocalStateResult(false,
                                    false,
                                    repositoryPath,
                                    QString(),
                                    QString(),
                                    QString(),
                                    false,
                                    false,
                                    QStringLiteral("Repository path does not exist: %1")
                                        .arg(repositoryPath));
    }

    const GitContext gitContext = resolveGitContext(repositoryPath);
    const QString repositoryRootPath = gitContext.ok && gitContext.gitRepository
                                           ? gitContext.repositoryRootPath
                                           : QDir(repositoryPath).absolutePath();
    const QString hydraDirectoryPath = QDir(repositoryRootPath).filePath(QStringLiteral(".hydra"));
    const QString docsDirectoryPath = QDir(hydraDirectoryPath).filePath(QStringLiteral("docs"));
    const QString localConfigPath = QDir(hydraDirectoryPath).filePath(QStringLiteral("local.json"));

    QString failedDirectoryPath;
    if (!ensureDirectories(hydraDirectories(hydraDirectoryPath, docsDirectoryPath),
                           &failedDirectoryPath)) {
        return repoLocalStateResult(false,
                                    gitContext.gitRepository,
                                    repositoryRootPath,
                                    hydraDirectoryPath,
                                    docsDirectoryPath,
                                    localConfigPath,
                                    false,
                                    false,
                                    QStringLiteral("Failed to create repo-local directory: %1")
                                        .arg(failedDirectoryPath));
    }

    QString errorMessage;
    if (!ensureLocalConfigFile(localConfigPath, &errorMessage)) {
        return repoLocalStateResult(false,
                                    gitContext.gitRepository,
                                    repositoryRootPath,
                                    hydraDirectoryPath,
                                    docsDirectoryPath,
                                    localConfigPath,
                                    true,
                                    false,
                                    errorMessage);
    }

    if (!gitContext.ok) {
        return repoLocalStateResult(true,
                                    false,
                                    repositoryRootPath,
                                    hydraDirectoryPath,
                                    docsDirectoryPath,
                                    localConfigPath,
                                    true,
                                    false,
                                    QString());
    }

    const QString excludePath =
        QDir(gitContext.gitDirectoryPath).filePath(QStringLiteral("info/exclude"));
    if (!ensureExcludeEntry(excludePath, &errorMessage)) {
        return repoLocalStateResult(false,
                                    true,
                                    repositoryRootPath,
                                    hydraDirectoryPath,
                                    docsDirectoryPath,
                                    localConfigPath,
                                    true,
                                    false,
                                    errorMessage);
    }

    return repoLocalStateResult(true,
                                true,
                                repositoryRootPath,
                                hydraDirectoryPath,
                                docsDirectoryPath,
                                localConfigPath,
                                true,
                                true,
                                QString());
}

domain::ports::WorktreeListResult GitRepoWorkspace::listWorktrees(const QString &repositoryPath) const
{
    const GitContext gitContext = resolveGitContext(repositoryPath);
    if (!gitContext.ok) {
        return {.ok = false, .worktrees = {}, .errorMessage = gitContext.errorMessage};
    }

    const CommandResult result = runGit(gitContext.repositoryRootPath,
                                        {QStringLiteral("worktree"),
                                         QStringLiteral("list"),
                                         QStringLiteral("--porcelain")});
    if (!result.ok) {
        return {.ok = false,
                .worktrees = {},
                .errorMessage = QStringLiteral("git worktree list failed: %1")
                                    .arg(trimmedOutput(result.standardError))};
    }

    QVector<domain::Worktree> worktrees;
    QStringList currentBlock;
    const QStringList lines = result.standardOutput.split('\n');
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) {
            if (!currentBlock.isEmpty()) {
                worktrees.push_back(worktreeFromPorcelainBlock(currentBlock,
                                                               gitContext.repositoryRootPath));
                currentBlock.clear();
            }
            continue;
        }
        currentBlock.push_back(line);
    }

    if (!currentBlock.isEmpty()) {
        worktrees.push_back(worktreeFromPorcelainBlock(currentBlock,
                                                       gitContext.repositoryRootPath));
    }

    return {.ok = true, .worktrees = worktrees, .errorMessage = QString()};
}

domain::ports::CreateWorktreeResult GitRepoWorkspace::createWorktree(
    const domain::ports::CreateWorktreeRequest &request)
{
    const GitContext gitContext = resolveGitContext(request.repositoryPath);
    if (!gitContext.ok) {
        return {.ok = false, .worktree = {}, .errorMessage = gitContext.errorMessage};
    }

    const QString trimmedBranchName = request.branchName.trimmed();
    if (trimmedBranchName.isEmpty()) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Branch name is required to create a worktree.")};
    }

    const QString trimmedWorktreePath = request.worktreePath.trimmed();
    if (trimmedWorktreePath.isEmpty()) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Worktree path is required.")};
    }

    if (QFileInfo::exists(trimmedWorktreePath)) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Worktree path already exists: %1")
                                    .arg(trimmedWorktreePath)};
    }

    const CommandResult branchCheck = runGit(
        gitContext.repositoryRootPath,
        {QStringLiteral("show-ref"),
         QStringLiteral("--verify"),
         QStringLiteral("--quiet"),
         QStringLiteral("refs/heads/%1").arg(trimmedBranchName)});
    const CommandResult headCheck = runGit(gitContext.repositoryRootPath,
                                           {QStringLiteral("rev-parse"),
                                            QStringLiteral("--verify"),
                                            QStringLiteral("HEAD")});

    const CommandResult create = runGit(
        gitContext.repositoryRootPath,
        createWorktreeArguments(trimmedBranchName,
                                trimmedWorktreePath,
                                branchCheck.ok,
                                headCheck.ok));
    if (!create.ok) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("git worktree add failed: %1")
                                    .arg(trimmedOutput(create.standardError))};
    }

    const auto worktrees = listWorktrees(gitContext.repositoryRootPath);
    if (!worktrees.ok) {
        return {.ok = false,
                .worktree = {},
                .errorMessage = QStringLiteral("Worktree created but refresh failed: %1")
                                    .arg(worktrees.errorMessage)};
    }

    const QString requestedPath = normalizedPath(trimmedWorktreePath);
    for (const domain::Worktree &worktree : worktrees.worktrees) {
        if (normalizedPath(worktree.path) == requestedPath) {
            return {.ok = true, .worktree = worktree, .errorMessage = QString()};
        }
    }

    return {.ok = true,
            .worktree = {.path = QDir(trimmedWorktreePath).absolutePath(),
                         .branchName = trimmedBranchName,
                         .headCommit = QString(),
                         .isMain = false},
            .errorMessage = QString()};
}

GitRepoWorkspace::CommandResult GitRepoWorkspace::runGit(const QString &workingDirectory,
                                                         const QStringList &arguments) const
{
    CommandResult result = process::runProcess(m_gitPath, arguments, workingDirectory);
    if (!result.ok && result.standardError == QStringLiteral("process timed out")) {
        result.standardError = QStringLiteral("git command timed out");
    }

    return result;
}

GitRepoWorkspace::GitContext GitRepoWorkspace::resolveGitContext(const QString &repositoryPath) const
{
    const QFileInfo repositoryInfo(repositoryPath);
    if (!repositoryInfo.exists() || !repositoryInfo.isDir()) {
        return {.ok = false,
                .gitRepository = false,
                .repositoryRootPath = QString(),
                .gitDirectoryPath = QString(),
                .errorMessage = QStringLiteral("Repository path does not exist: %1").arg(repositoryPath)};
    }

    const QString absoluteRepositoryPath = QDir(repositoryPath).absolutePath();
    const CommandResult rootResult =
        runGit(absoluteRepositoryPath, {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")});
    if (!rootResult.ok) {
        return {.ok = false,
                .gitRepository = false,
                .repositoryRootPath = absoluteRepositoryPath,
                .gitDirectoryPath = QString(),
                .errorMessage = QStringLiteral("Git repository detection failed: %1")
                                    .arg(trimmedOutput(rootResult.standardError))};
    }

    const CommandResult gitDirectoryResult =
        runGit(absoluteRepositoryPath, {QStringLiteral("rev-parse"), QStringLiteral("--git-dir")});
    if (!gitDirectoryResult.ok) {
        return {.ok = false,
                .gitRepository = false,
                .repositoryRootPath = trimmedOutput(rootResult.standardOutput),
                .gitDirectoryPath = QString(),
                .errorMessage = QStringLiteral("Git metadata lookup failed: %1")
                                    .arg(trimmedOutput(gitDirectoryResult.standardError))};
    }

    const QString repositoryRootPath = trimmedOutput(rootResult.standardOutput);
    const QString gitDirectoryPath =
        toAbsolutePath(repositoryRootPath, trimmedOutput(gitDirectoryResult.standardOutput));

    return {.ok = true,
            .gitRepository = true,
            .repositoryRootPath = repositoryRootPath,
            .gitDirectoryPath = gitDirectoryPath,
            .errorMessage = QString()};
}

QString GitRepoWorkspace::toAbsolutePath(const QString &baseDirectory, const QString &path)
{
    if (QDir::isAbsolutePath(path)) {
        return QDir(path).absolutePath();
    }

    return QDir(baseDirectory).absoluteFilePath(path);
}

QString GitRepoWorkspace::branchNameFromRef(const QString &ref)
{
    static const QString prefix = QStringLiteral("refs/heads/");
    return ref.startsWith(prefix) ? ref.mid(prefix.size()) : ref;
}

QString GitRepoWorkspace::normalizedPath(const QString &path)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QFileInfo info(cleanPath);
    const QString canonicalPath = info.canonicalFilePath();
    return canonicalPath.isEmpty() ? QDir(cleanPath).absolutePath() : canonicalPath;
}

domain::Worktree GitRepoWorkspace::worktreeFromPorcelainBlock(const QStringList &blockLines,
                                                              const QString &repositoryRootPath)
{
    domain::Worktree worktree;
    for (const QString &line : blockLines) {
        if (line.startsWith(QStringLiteral("worktree "))) {
            worktree.path = line.mid(QStringLiteral("worktree ").size()).trimmed();
        } else if (line.startsWith(QStringLiteral("HEAD "))) {
            worktree.headCommit = line.mid(QStringLiteral("HEAD ").size()).trimmed();
        } else if (line.startsWith(QStringLiteral("branch "))) {
            worktree.branchName =
                branchNameFromRef(line.mid(QStringLiteral("branch ").size()).trimmed());
        } else if (line == QStringLiteral("detached")) {
            worktree.branchName = QStringLiteral("(detached)");
        }
    }

    worktree.path = normalizedPath(worktree.path);
    worktree.isMain = worktree.path == normalizedPath(repositoryRootPath);
    if (worktree.isMain && worktree.branchName.isEmpty()) {
        worktree.branchName = QStringLiteral("main");
    }

    return worktree;
}

}  // namespace hydra::infrastructure
