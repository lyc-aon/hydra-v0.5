#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "domain/services/provider_catalog.hpp"
#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/status_aggregator.hpp"
#include "domain/services/worktree_manager.hpp"
#include "infrastructure/git/git_repo_workspace.hpp"
#include "infrastructure/mux/tmux_adapter.hpp"
#include "infrastructure/persistence/database_manager.hpp"
#include "infrastructure/persistence/sqlite_repo_store.hpp"
#include "infrastructure/persistence/sqlite_session_store.hpp"
#include "infrastructure/providers/claude_code_adapter.hpp"
#include "infrastructure/providers/codex_cli_adapter.hpp"
#include "infrastructure/providers/codex_session_resolver.hpp"
#include "infrastructure/providers/gemini_cli_adapter.hpp"
#include "infrastructure/providers/gemini_session_resolver.hpp"
#include "infrastructure/providers/hermes_cli_adapter.hpp"
#include "infrastructure/providers/hermes_session_resolver.hpp"
#include "infrastructure/providers/opencode_cli_adapter.hpp"
#include "infrastructure/providers/opencode_session_resolver.hpp"
#include "infrastructure/terminal/tmux_terminal_backend.hpp"
#include "ui/viewmodels/app_state.hpp"
#include "ui/viewmodels/shell_state.hpp"
#include "ui/viewmodels/terminal_surface_controller.hpp"
#include "ui/viewmodels/theme_state.hpp"

namespace hydra::app {

class HydraApplication final : public QObject {
    Q_OBJECT

public:
    explicit HydraApplication(QObject *parent = nullptr);

    ui::AppState *appState();
    ui::ShellState *shellState();
    ui::TerminalSurfaceController *terminalController();
    ui::TerminalSurfaceController *masterTerminalController();
    ui::TerminalSurfaceController *routerTerminalController();
    ui::ThemeState *themeState();
    void startUiLifecycle();
    void shutdownOwnedSessions();

private:
    struct DatabaseBootstrapState {
        QString path;
        bool existed = false;
    };

    static DatabaseBootstrapState databaseBootstrapState();
    static QString databasePath();

    DatabaseBootstrapState m_databaseBootstrapState;
    infrastructure::DatabaseManager m_databaseManager;
    infrastructure::SqliteRepoStore m_repoStore;
    infrastructure::SqliteSessionStore m_sessionStore;
    domain::RepoRegistry m_repoRegistry;
    infrastructure::GitRepoWorkspace m_repoWorkspace;
    domain::WorktreeManager m_worktreeManager;
    infrastructure::TmuxAdapter m_tmuxAdapter;
    infrastructure::CodexCliAdapter m_codexCliAdapter;
    infrastructure::CodexSessionResolver m_codexSessionResolver;
    infrastructure::GeminiCliAdapter m_geminiCliAdapter;
    infrastructure::GeminiSessionResolver m_geminiSessionResolver;
    infrastructure::ClaudeCodeAdapter m_claudeCodeAdapter;
    infrastructure::HermesCliAdapter m_hermesCliAdapter;
    infrastructure::HermesSessionResolver m_hermesSessionResolver;
    infrastructure::OpenCodeCliAdapter m_openCodeCliAdapter;
    infrastructure::OpenCodeSessionResolver m_openCodeSessionResolver;
    std::shared_ptr<domain::ports::TerminalBackend> m_terminalBackend;
    domain::ProviderCatalog m_providerCatalog;
    domain::StatusAggregator m_statusAggregator;
    QString m_runtimeOwnerInstanceId;
    domain::SessionSupervisor m_sessionSupervisor;
    std::unique_ptr<ui::AppState> m_appState;
    std::unique_ptr<ui::ShellState> m_shellState;
    std::unique_ptr<ui::TerminalSurfaceController> m_terminalController;
    std::unique_ptr<ui::TerminalSurfaceController> m_masterTerminalController;
    std::unique_ptr<ui::TerminalSurfaceController> m_routerTerminalController;
    std::unique_ptr<ui::ThemeState> m_themeState;
    bool m_shutdownHandled = false;
};

}  // namespace hydra::app
