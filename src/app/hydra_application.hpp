#pragma once

#include <QObject>

#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/worktree_manager.hpp"
#include "infrastructure/git/git_repo_workspace.hpp"
#include "infrastructure/mux/tmux_adapter.hpp"
#include "infrastructure/persistence/database_manager.hpp"
#include "infrastructure/persistence/sqlite_repo_store.hpp"
#include "infrastructure/persistence/sqlite_session_store.hpp"
#include "ui/viewmodels/app_state.hpp"

namespace hydra::app {

class HydraApplication final : public QObject {
    Q_OBJECT

public:
    explicit HydraApplication(QObject *parent = nullptr);

    ui::AppState *appState();

private:
    static QString databasePath();

    infrastructure::DatabaseManager m_databaseManager;
    infrastructure::SqliteRepoStore m_repoStore;
    infrastructure::SqliteSessionStore m_sessionStore;
    domain::RepoRegistry m_repoRegistry;
    infrastructure::GitRepoWorkspace m_repoWorkspace;
    domain::WorktreeManager m_worktreeManager;
    infrastructure::TmuxAdapter m_tmuxAdapter;
    domain::SessionSupervisor m_sessionSupervisor;
    ui::AppState m_appState;
};

}  // namespace hydra::app
