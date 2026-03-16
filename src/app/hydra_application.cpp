#include "app/hydra_application.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>

namespace hydra::app {

HydraApplication::HydraApplication(QObject *parent)
    : QObject(parent),
      m_databaseBootstrapState(databaseBootstrapState()),
      m_databaseManager(m_databaseBootstrapState.path),
      m_repoStore(m_databaseManager.database()),
      m_sessionStore(m_databaseManager.database()),
      m_repoRegistry(m_repoStore),
      m_repoWorkspace(),
      m_worktreeManager(m_repoStore, m_repoWorkspace),
      m_tmuxAdapter(),
      m_codexCliAdapter(),
      m_geminiCliAdapter(),
      m_claudeCodeAdapter(),
      m_hermesCliAdapter(),
      m_hermesSessionResolver(),
      m_openCodeCliAdapter(),
      m_openCodeSessionResolver(),
      m_terminalBackend(std::make_shared<infrastructure::TmuxTerminalBackend>()),
      m_providerCatalog(QVector<domain::ports::ProviderAdapter *>{
          &m_codexCliAdapter,
          &m_geminiCliAdapter,
          &m_claudeCodeAdapter,
          &m_hermesCliAdapter,
          &m_openCodeCliAdapter,
      }),
      m_statusAggregator(),
      m_runtimeOwnerInstanceId(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_sessionSupervisor(m_repoStore,
                          m_sessionStore,
                          m_tmuxAdapter,
                          *m_terminalBackend,
                          QVector<domain::ports::ProviderSessionResolver *>{&m_codexSessionResolver,
                                                                           &m_geminiSessionResolver,
                                                                           &m_hermesSessionResolver,
                                                                           &m_openCodeSessionResolver},
                          m_providerCatalog,
                          m_statusAggregator,
                          m_runtimeOwnerInstanceId),
      m_appState(std::make_unique<ui::AppState>(m_repoRegistry,
                                                m_worktreeManager,
                                                m_sessionSupervisor,
                                                m_providerCatalog,
                                                m_databaseBootstrapState.path,
                                                m_tmuxAdapter.probeAvailability().available)),
      m_shellState(std::make_unique<ui::ShellState>()),
      m_terminalController(std::make_unique<ui::TerminalSurfaceController>(m_terminalBackend)),
      m_masterTerminalController(std::make_unique<ui::TerminalSurfaceController>(m_terminalBackend)),
      m_routerTerminalController(std::make_unique<ui::TerminalSurfaceController>(m_terminalBackend)),
      m_themeState(std::make_unique<ui::ThemeState>())
{
    if (QCoreApplication::instance() != nullptr) {
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [this]() {
            shutdownOwnedSessions();
        });
    }

    const QString bootstrapPath = QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH);
    if (!m_databaseBootstrapState.existed
        || !m_repoRegistry.repositoryByPath(bootstrapPath).has_value())
    {
        m_repoRegistry.ensureBootstrapRepository(
            QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH),
            QStringLiteral("Hydra V2"),
            QStringLiteral("Self-hosting workspace for the native Hydra scaffold."),
            QStringLiteral("#B38A59"));
    }
}

ui::AppState *HydraApplication::appState()
{
    return m_appState.get();
}

ui::ShellState *HydraApplication::shellState()
{
    return m_shellState.get();
}

ui::TerminalSurfaceController *HydraApplication::terminalController()
{
    return m_terminalController.get();
}

ui::TerminalSurfaceController *HydraApplication::masterTerminalController()
{
    return m_masterTerminalController.get();
}

ui::TerminalSurfaceController *HydraApplication::routerTerminalController()
{
    return m_routerTerminalController.get();
}

ui::ThemeState *HydraApplication::themeState()
{
    return m_themeState.get();
}

void HydraApplication::startUiLifecycle()
{
    m_appState->startLifecycle();
}

void HydraApplication::shutdownOwnedSessions()
{
    if (m_shutdownHandled) {
        return;
    }

    m_shutdownHandled = true;
    const domain::SessionShutdownOutcome outcome = m_sessionSupervisor.shutdownOwnedSessionsForAppExit();
    if (!outcome.ok) {
        qWarning().noquote()
            << QStringLiteral("Hydra app shutdown had session cleanup errors: %1")
                   .arg(outcome.errors.join(QStringLiteral(" | ")));
    }
}

HydraApplication::DatabaseBootstrapState HydraApplication::databaseBootstrapState()
{
    const QString path = databasePath();
    return {.path = path, .existed = QFileInfo::exists(path)};
}

QString HydraApplication::databasePath()
{
    const QString appDataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDirectory);
    return QFileInfo(appDataDirectory, QStringLiteral("hydra.sqlite3")).absoluteFilePath();
}

}  // namespace hydra::app
