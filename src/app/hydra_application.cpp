#include "app/hydra_application.hpp"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace hydra::app {

HydraApplication::HydraApplication(QObject *parent)
    : QObject(parent),
      m_databaseManager(databasePath()),
      m_repoStore(m_databaseManager.database()),
      m_sessionStore(m_databaseManager.database()),
      m_repoRegistry(m_repoStore),
      m_repoWorkspace(),
      m_worktreeManager(m_repoStore, m_repoWorkspace),
      m_tmuxAdapter(),
      m_sessionSupervisor(m_repoStore, m_sessionStore, m_tmuxAdapter),
      m_appState(m_repoRegistry,
                 m_worktreeManager,
                 m_sessionSupervisor,
                 m_tmuxAdapter.probeAvailability().available)
{
    m_repoRegistry.ensureBootstrapRepository(QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH),
                                             QStringLiteral("Hydra V2"),
                                             QStringLiteral("Self-hosting workspace for the native Hydra scaffold."),
                                             QStringLiteral("#b76e3c"));
    m_appState.reload();
}

ui::AppState *HydraApplication::appState()
{
    return &m_appState;
}

QString HydraApplication::databasePath()
{
    const QString appDataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDirectory);
    return QFileInfo(appDataDirectory, QStringLiteral("hydra.sqlite3")).absoluteFilePath();
}

}  // namespace hydra::app
