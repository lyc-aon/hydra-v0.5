#pragma once

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

namespace hydra::ui {

class AppStateWorkerRuntime final {
public:
    explicit AppStateWorkerRuntime(const QString &databasePath);

    domain::RepoRegistry &repoRegistry();
    domain::WorktreeManager &worktreeManager();
    domain::SessionSupervisor &sessionSupervisor();
    const domain::ProviderCatalog &providerCatalog() const;

private:
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
    infrastructure::TmuxTerminalBackend m_terminalBackend;
    domain::ProviderCatalog m_providerCatalog;
    domain::StatusAggregator m_statusAggregator;
    domain::SessionSupervisor m_sessionSupervisor;
};

}  // namespace hydra::ui
