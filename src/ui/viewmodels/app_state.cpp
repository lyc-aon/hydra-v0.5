#include "ui/viewmodels/app_state.hpp"

#include <QHash>

namespace hydra::ui {

namespace {

constexpr int kInfoStatusDurationMs = 2600;
constexpr int kWarningStatusDurationMs = 3600;

}

AppState::AppState(domain::RepoRegistry &repoRegistry,
                   domain::WorktreeManager &worktreeManager,
                   domain::SessionSupervisor &sessionSupervisor,
                   const bool tmuxAvailable,
                   QObject *parent)
    : QObject(parent),
      m_repoRegistry(repoRegistry),
      m_worktreeManager(worktreeManager),
      m_sessionSupervisor(sessionSupervisor),
      m_tmuxAvailable(tmuxAvailable),
      m_repoModel(this),
      m_sessionModel(this),
      m_worktreeModel(this)
{
    m_statusMessageTimer.setParent(this);
    m_statusMessageTimer.setSingleShot(true);
    QObject::connect(&m_statusMessageTimer, &QTimer::timeout, this, [this]() {
        clearStatusMessage();
    });
}

QAbstractItemModel *AppState::repoModel()
{
    return &m_repoModel;
}

QAbstractItemModel *AppState::sessionModel()
{
    return &m_sessionModel;
}

QAbstractItemModel *AppState::worktreeModel()
{
    return &m_worktreeModel;
}

QString AppState::selectedRepoId() const
{
    return m_selectedRepoId;
}

void AppState::setSelectedRepoId(const QString &selectedRepoId)
{
    if (m_selectedRepoId == selectedRepoId) {
        return;
    }

    m_selectedRepoId = selectedRepoId;
    m_repoModel.setSelectedRepositoryId(m_selectedRepoId);
    emit selectedRepoIdChanged();
    emit selectedRepoNameChanged();
    reloadSelectedRepoWorkspace();
}

QString AppState::selectedRepoName() const
{
    for (const domain::Repository &repository : m_repositories) {
        if (repository.id == m_selectedRepoId) {
            return repository.name;
        }
    }

    return QStringLiteral("No repository selected");
}

QString AppState::selectedWorktreePath() const
{
    return m_selectedWorktreePath;
}

void AppState::setSelectedWorktreePath(const QString &selectedWorktreePath)
{
    setSelectedWorktreePathInternal(selectedWorktreePath);
}

QString AppState::selectedWorktreeBranch() const
{
    for (const domain::Worktree &worktree : m_worktrees) {
        if (worktree.path == m_selectedWorktreePath) {
            return worktree.branchName;
        }
    }

    return QString();
}

QString AppState::statusMessage() const
{
    return m_statusMessage;
}

bool AppState::statusIsWarning() const
{
    return m_statusIsWarning;
}

QString AppState::repositoryRootPath() const
{
    return m_repositoryRootPath;
}

QString AppState::hydraDirectoryPath() const
{
    return m_hydraDirectoryPath;
}

QString AppState::docsDirectoryPath() const
{
    return m_docsDirectoryPath;
}

QString AppState::repositoryWorkspaceMessage() const
{
    return m_repositoryWorkspaceMessage;
}

bool AppState::repositoryIsGit() const
{
    return m_repositoryIsGit;
}

bool AppState::repoLocalStateReady() const
{
    return m_repoLocalStateReady;
}

bool AppState::gitExcludeReady() const
{
    return m_gitExcludeReady;
}

bool AppState::tmuxAvailable() const
{
    return m_tmuxAvailable;
}

int AppState::repoCount() const
{
    return m_repositories.size();
}

int AppState::sessionCount() const
{
    return m_sessionModel.rowCount();
}

int AppState::worktreeCount() const
{
    return m_worktreeModel.rowCount();
}

void AppState::launchSelectedRepoSession()
{
    if (m_selectedRepoId.isEmpty()) {
        showTransientStatus(QStringLiteral("Select a repository before launching a session."),
                            kWarningStatusDurationMs,
                            true);
        return;
    }

    const QString workingDirectory = !m_selectedWorktreePath.isEmpty()
                                         ? m_selectedWorktreePath
                                         : m_repositoryRootPath;
    const domain::LaunchOutcome outcome =
        m_sessionSupervisor.launchGenericShell(m_selectedRepoId, workingDirectory);
    showTransientStatus(outcome.message,
                        outcome.ok ? kInfoStatusDurationMs : kWarningStatusDurationMs,
                        !outcome.ok);
    if (outcome.ok) {
        reload();
    }
}

bool AppState::createWorktree(const QString &branchName)
{
    if (m_selectedRepoId.isEmpty()) {
        showTransientStatus(QStringLiteral("Select a repository before creating a worktree."),
                            kWarningStatusDurationMs,
                            true);
        return false;
    }

    const domain::WorktreeCreationOutcome outcome =
        m_worktreeManager.createWorktree(m_selectedRepoId, branchName);
    if (!outcome.ok) {
        showTransientStatus(outcome.errorMessage, kWarningStatusDurationMs, true);
        return false;
    }

    reloadSelectedRepoWorkspace();
    setSelectedWorktreePathInternal(outcome.worktree.path);
    showTransientStatus(QStringLiteral("Created worktree %1 for branch %2.")
                            .arg(outcome.worktree.path, outcome.worktree.branchName),
                        kInfoStatusDurationMs,
                        false);
    return true;
}

void AppState::terminateSession(const QString &sessionId)
{
    const domain::SessionTerminationOutcome outcome =
        m_sessionSupervisor.terminateSession(sessionId);
    showTransientStatus(outcome.message,
                        outcome.ok ? kInfoStatusDurationMs : kWarningStatusDurationMs,
                        !outcome.ok);
    if (outcome.ok) {
        reload();
    }
}

void AppState::refresh()
{
    reload();
    showTransientStatus(QStringLiteral("Hydra state refreshed from SQLite, tmux, and Git."),
                        kInfoStatusDurationMs,
                        false);
}

void AppState::clearStatusMessage()
{
    if (m_statusMessage.isEmpty()) {
        return;
    }

    m_statusMessage.clear();
    m_statusIsWarning = false;
    emit statusMessageChanged();
}

void AppState::showTransientStatus(const QString &statusMessage,
                                   const int durationMs,
                                   const bool warning)
{
    setStatusMessage(statusMessage, warning);
    if (!statusMessage.isEmpty()) {
        m_statusMessageTimer.start(durationMs);
    }
}

void AppState::reload()
{
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
        m_selectedRepoId = m_repositories.isEmpty() ? QString() : m_repositories.first().id;
        emit selectedRepoIdChanged();
        emit selectedRepoNameChanged();
    }

    m_repoModel.setSelectedRepositoryId(m_selectedRepoId);
    reloadSelectedRepoWorkspace();

    QHash<QString, domain::Repository> repositoriesById;
    for (const domain::Repository &repository : m_repositories) {
        repositoriesById.insert(repository.id, repository);
    }

    m_sessionModel.setSessions(m_sessionSupervisor.refreshSessionStates(), repositoriesById);

    emit repoCountChanged();
    emit sessionCountChanged();
}

void AppState::reloadSelectedRepoWorkspace()
{
    QString nextRepositoryRootPath;
    QString nextHydraDirectoryPath;
    QString nextDocsDirectoryPath;
    QString nextRepositoryWorkspaceMessage;
    bool nextRepositoryIsGit = false;
    bool nextRepoLocalStateReady = false;
    bool nextGitExcludeReady = false;
    QVector<domain::Worktree> nextWorktrees;

    if (!m_selectedRepoId.isEmpty()) {
        const domain::RepoWorkspaceSnapshot snapshot =
            m_worktreeManager.loadWorkspace(m_selectedRepoId);
        nextRepositoryRootPath = snapshot.repositoryPath;
        nextHydraDirectoryPath = snapshot.hydraDirectoryPath;
        nextDocsDirectoryPath = snapshot.docsDirectoryPath;
        nextRepositoryIsGit = snapshot.gitRepository;
        nextRepoLocalStateReady = snapshot.hydraDirectoryReady;
        nextGitExcludeReady = snapshot.gitExcludeReady;
        nextWorktrees = snapshot.worktrees;

        if (snapshot.ok) {
            nextRepositoryWorkspaceMessage = snapshot.gitRepository
                                                ? QStringLiteral("Repo-local state is ready. .hydra is provisioned and excluded from Git.")
                                                : QStringLiteral("Repo-local state is ready. Git worktree features are unavailable for this path.");
        } else {
            nextRepositoryWorkspaceMessage = snapshot.errorMessage;
        }
    }

    m_repositoryRootPath = nextRepositoryRootPath;
    m_hydraDirectoryPath = nextHydraDirectoryPath;
    m_docsDirectoryPath = nextDocsDirectoryPath;
    m_repositoryWorkspaceMessage = nextRepositoryWorkspaceMessage;
    m_repositoryIsGit = nextRepositoryIsGit;
    m_repoLocalStateReady = nextRepoLocalStateReady;
    m_gitExcludeReady = nextGitExcludeReady;
    m_worktrees = nextWorktrees;
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

void AppState::setStatusMessage(const QString &statusMessage, const bool warning)
{
    m_statusMessageTimer.stop();
    if (m_statusMessage == statusMessage && m_statusIsWarning == warning) {
        return;
    }

    m_statusMessage = statusMessage;
    m_statusIsWarning = !m_statusMessage.isEmpty() && warning;
    emit statusMessageChanged();
}

void AppState::setSelectedWorktreePathInternal(const QString &selectedWorktreePath)
{
    if (m_selectedWorktreePath == selectedWorktreePath) {
        m_worktreeModel.setSelectedWorktreePath(m_selectedWorktreePath);
        return;
    }

    m_selectedWorktreePath = selectedWorktreePath;
    m_worktreeModel.setSelectedWorktreePath(m_selectedWorktreePath);
    emit selectedWorktreePathChanged();
    emit selectedWorktreeBranchChanged();
}

}  // namespace hydra::ui
