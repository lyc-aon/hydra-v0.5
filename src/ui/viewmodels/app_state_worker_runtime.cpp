#include "ui/viewmodels/app_state_worker_runtime.hpp"

namespace hydra::ui {

AppStateWorkerRuntime::AppStateWorkerRuntime(const QString &databasePath)
    : m_databaseManager(databasePath),
      m_repoStore(m_databaseManager.database()),
      m_sessionStore(m_databaseManager.database()),
      m_repoRegistry(m_repoStore),
      m_repoWorkspace(),
      m_worktreeManager(m_repoStore, m_repoWorkspace),
      m_tmuxAdapter(),
      m_codexCliAdapter(),
      m_codexSessionResolver(),
      m_geminiCliAdapter(),
      m_geminiSessionResolver(),
      m_claudeCodeAdapter(),
      m_hermesCliAdapter(),
      m_hermesSessionResolver(),
      m_openCodeCliAdapter(),
      m_openCodeSessionResolver(),
      m_terminalBackend(),
      m_providerCatalog(QVector<domain::ports::ProviderAdapter *>{
          &m_codexCliAdapter,
          &m_geminiCliAdapter,
          &m_claudeCodeAdapter,
          &m_hermesCliAdapter,
          &m_openCodeCliAdapter,
      }),
      m_statusAggregator(),
      m_sessionSupervisor(m_repoStore,
                          m_sessionStore,
                          m_tmuxAdapter,
                          m_terminalBackend,
                          QVector<domain::ports::ProviderSessionResolver *>{&m_codexSessionResolver,
                                                                           &m_geminiSessionResolver,
                                                                           &m_hermesSessionResolver,
                                                                           &m_openCodeSessionResolver},
                          m_providerCatalog,
                          m_statusAggregator,
                          QString())
{
}

domain::RepoRegistry &AppStateWorkerRuntime::repoRegistry()
{
    return m_repoRegistry;
}

domain::WorktreeManager &AppStateWorkerRuntime::worktreeManager()
{
    return m_worktreeManager;
}

domain::SessionSupervisor &AppStateWorkerRuntime::sessionSupervisor()
{
    return m_sessionSupervisor;
}

const domain::ProviderCatalog &AppStateWorkerRuntime::providerCatalog() const
{
    return m_providerCatalog;
}

}  // namespace hydra::ui
