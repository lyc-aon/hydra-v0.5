#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/models/worktree.hpp"
#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/worktree_manager.hpp"
#include "ui/viewmodels/repo_list_model.hpp"
#include "ui/viewmodels/session_list_model.hpp"
#include "ui/viewmodels/worktree_list_model.hpp"

namespace hydra::ui {

class AppState final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *repoModel READ repoModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *sessionModel READ sessionModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *worktreeModel READ worktreeModel CONSTANT)
    Q_PROPERTY(QString selectedRepoId READ selectedRepoId WRITE setSelectedRepoId NOTIFY selectedRepoIdChanged)
    Q_PROPERTY(QString selectedRepoName READ selectedRepoName NOTIFY selectedRepoNameChanged)
    Q_PROPERTY(QString selectedWorktreePath READ selectedWorktreePath WRITE setSelectedWorktreePath NOTIFY selectedWorktreePathChanged)
    Q_PROPERTY(QString selectedWorktreeBranch READ selectedWorktreeBranch NOTIFY selectedWorktreeBranchChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString repositoryRootPath READ repositoryRootPath NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(QString hydraDirectoryPath READ hydraDirectoryPath NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(QString docsDirectoryPath READ docsDirectoryPath NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(QString repositoryWorkspaceMessage READ repositoryWorkspaceMessage NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool repositoryIsGit READ repositoryIsGit NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool repoLocalStateReady READ repoLocalStateReady NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool gitExcludeReady READ gitExcludeReady NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool tmuxAvailable READ tmuxAvailable CONSTANT)
    Q_PROPERTY(int repoCount READ repoCount NOTIFY repoCountChanged)
    Q_PROPERTY(int sessionCount READ sessionCount NOTIFY sessionCountChanged)
    Q_PROPERTY(int worktreeCount READ worktreeCount NOTIFY worktreeCountChanged)

public:
    AppState(domain::RepoRegistry &repoRegistry,
             domain::WorktreeManager &worktreeManager,
             domain::SessionSupervisor &sessionSupervisor,
             bool tmuxAvailable,
             QObject *parent = nullptr);

    QAbstractItemModel *repoModel();
    QAbstractItemModel *sessionModel();
    QAbstractItemModel *worktreeModel();

    QString selectedRepoId() const;
    void setSelectedRepoId(const QString &selectedRepoId);
    QString selectedRepoName() const;
    QString selectedWorktreePath() const;
    void setSelectedWorktreePath(const QString &selectedWorktreePath);
    QString selectedWorktreeBranch() const;
    QString statusMessage() const;
    QString repositoryRootPath() const;
    QString hydraDirectoryPath() const;
    QString docsDirectoryPath() const;
    QString repositoryWorkspaceMessage() const;
    bool repositoryIsGit() const;
    bool repoLocalStateReady() const;
    bool gitExcludeReady() const;
    bool tmuxAvailable() const;
    int repoCount() const;
    int sessionCount() const;
    int worktreeCount() const;

    Q_INVOKABLE void launchSelectedRepoSession();
    Q_INVOKABLE bool createWorktree(const QString &branchName);
    Q_INVOKABLE void terminateSession(const QString &sessionId);
    Q_INVOKABLE void refresh();

    void reload();

signals:
    void selectedRepoIdChanged();
    void selectedRepoNameChanged();
    void selectedWorktreePathChanged();
    void selectedWorktreeBranchChanged();
    void statusMessageChanged();
    void repositoryWorkspaceChanged();
    void repoCountChanged();
    void sessionCountChanged();
    void worktreeCountChanged();

private:
    void clearStatusMessage();
    void showTransientStatus(const QString &statusMessage, int durationMs = 2600);
    void reloadSelectedRepoWorkspace();
    void setStatusMessage(const QString &statusMessage);
    void setSelectedWorktreePathInternal(const QString &selectedWorktreePath);

    domain::RepoRegistry &m_repoRegistry;
    domain::WorktreeManager &m_worktreeManager;
    domain::SessionSupervisor &m_sessionSupervisor;
    bool m_tmuxAvailable = false;
    QTimer m_statusMessageTimer;
    QString m_selectedRepoId;
    QString m_selectedWorktreePath;
    QString m_statusMessage;
    QString m_repositoryRootPath;
    QString m_hydraDirectoryPath;
    QString m_docsDirectoryPath;
    QString m_repositoryWorkspaceMessage;
    bool m_repositoryIsGit = false;
    bool m_repoLocalStateReady = false;
    bool m_gitExcludeReady = false;
    QVector<domain::Repository> m_repositories;
    QVector<domain::Worktree> m_worktrees;
    RepoListModel m_repoModel;
    SessionListModel m_sessionModel;
    WorktreeListModel m_worktreeModel;
};

}  // namespace hydra::ui
