#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTextStream>

#include <stdexcept>

#include "domain/models/launch_safety.hpp"
#include "domain/models/session_record.hpp"
#include "domain/models/status_provenance.hpp"
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
#include "infrastructure/process/process_runner.hpp"
#include "infrastructure/providers/claude_code_adapter.hpp"
#include "infrastructure/providers/codex_cli_adapter.hpp"
#include "infrastructure/providers/codex_session_resolver.hpp"
#include "infrastructure/providers/gemini_cli_adapter.hpp"
#include "infrastructure/providers/gemini_session_resolver.hpp"
#include "infrastructure/terminal/tmux_terminal_backend.hpp"
#include "ui/viewmodels/app_state.hpp"
#include "ui/viewmodels/session_list_model.hpp"

namespace {

using hydra::domain::Repository;
using hydra::domain::SessionRecord;
using hydra::domain::SessionState;
using hydra::domain::StatusProvenance;
using hydra::infrastructure::process::ProcessResult;

void require(const bool condition, const QString &message)
{
    if (!condition) {
        throw std::runtime_error(message.toStdString());
    }
}

ProcessResult runCommand(const QString &executable,
                         const QStringList &arguments,
                         const QString &workingDirectory = QString(),
                         const int finishTimeoutMs = 5000)
{
    return hydra::infrastructure::process::runProcess(
        executable,
        arguments,
        workingDirectory,
        3000,
        finishTimeoutMs);
}

void ensureGitRepo(const QString &path)
{
    QDir().mkpath(path);
    require(runCommand(QStringLiteral("git"), {QStringLiteral("init"), QStringLiteral("-q")}, path).ok,
            QStringLiteral("git init failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("config"), QStringLiteral("user.email"), QStringLiteral("hydra-test@example.com")},
                       path).ok,
            QStringLiteral("git config user.email failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("config"), QStringLiteral("user.name"), QStringLiteral("Hydra Test")},
                       path).ok,
            QStringLiteral("git config user.name failed"));

    QFile readme(QFileInfo(path, QStringLiteral("README.md")).absoluteFilePath());
    require(readme.open(QIODevice::WriteOnly | QIODevice::Text), QStringLiteral("failed to write README"));
    QTextStream(&readme) << "seed\n";
    readme.close();

    require(runCommand(QStringLiteral("git"), {QStringLiteral("add"), QStringLiteral("README.md")}, path).ok,
            QStringLiteral("git add failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("commit"), QStringLiteral("-q"), QStringLiteral("-m"), QStringLiteral("seed")},
                       path).ok,
            QStringLiteral("git commit failed"));
}

QStringList modelOrder(hydra::ui::AppState &appState)
{
    auto *model = qobject_cast<hydra::ui::SessionListModel *>(appState.sessionModel());
    require(model != nullptr, QStringLiteral("Session model cast failed"));

    QStringList order;
    for (int row = 0; row < model->rowCount(); ++row) {
        order.push_back(model->sessionIdAt(row));
    }
    return order;
}

SessionRecord makeSession(const QString &id,
                          const QString &repoId,
                          const QString &providerKey,
                          const QString &name,
                          const SessionState state,
                          const QDateTime &updatedAt)
{
    SessionRecord session;
    session.id = id;
    session.repoId = repoId;
    session.name = name;
    session.providerKey = providerKey;
    session.providerSessionId = QStringLiteral("token-%1").arg(id);
    session.launchSafetyKey = hydra::domain::launchSafetyKey(hydra::domain::LaunchSafety::WorkspaceSafe);
    session.state = state;
    session.workingDirectory = QStringLiteral("/tmp/%1").arg(id);
    session.createdAt = updatedAt.addSecs(-30);
    session.updatedAt = updatedAt;
    session.statusDetail = QStringLiteral("detail-%1").arg(id);
    session.statusProvenance = StatusProvenance::Derived;
    session.statusSourceRef = QStringLiteral("smoke");
    return session;
}

void seedSessions(hydra::infrastructure::SqliteSessionStore &sessionStore,
                  const QString &repoId)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    QString error;
    require(sessionStore.upsertSession(makeSession(QStringLiteral("thinking"),
                                                   repoId,
                                                   QStringLiteral("codex"),
                                                   QStringLiteral("Thinking"),
                                                   SessionState::Thinking,
                                                   now),
                                       &error),
            error);
    require(sessionStore.upsertSession(makeSession(QStringLiteral("idle"),
                                                   repoId,
                                                   QStringLiteral("gemini"),
                                                   QStringLiteral("Idle"),
                                                   SessionState::Idle,
                                                   now.addSecs(-10)),
                                       &error),
            error);
    require(sessionStore.upsertSession(makeSession(QStringLiteral("approval"),
                                                   repoId,
                                                   QStringLiteral("claude"),
                                                   QStringLiteral("Approval"),
                                                   SessionState::AwaitingApproval,
                                                   now.addSecs(-20)),
                                       &error),
            error);
}

}  // namespace

int main(int argc, char *argv[])
{
    QTemporaryDir tempRoot;
    require(tempRoot.isValid(), QStringLiteral("failed to create temporary root"));
    qputenv("XDG_CONFIG_HOME", QFileInfo(tempRoot.path(), QStringLiteral("config")).absoluteFilePath().toUtf8());

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra V2"));

    const QString repoPath = QFileInfo(tempRoot.path(), QStringLiteral("repo")).absoluteFilePath();
    const QString dbPath = QFileInfo(tempRoot.path(), QStringLiteral("hydra.sqlite3")).absoluteFilePath();
    ensureGitRepo(repoPath);

    hydra::infrastructure::DatabaseManager databaseManager(dbPath);
    hydra::infrastructure::SqliteRepoStore repoStore(databaseManager.database());
    hydra::infrastructure::SqliteSessionStore sessionStore(databaseManager.database());
    hydra::domain::RepoRegistry repoRegistry(repoStore);
    hydra::infrastructure::GitRepoWorkspace repoWorkspace;
    hydra::domain::WorktreeManager worktreeManager(repoStore, repoWorkspace);
    hydra::infrastructure::TmuxAdapter tmuxAdapter;
    hydra::infrastructure::CodexCliAdapter codexCliAdapter;
    hydra::infrastructure::CodexSessionResolver codexSessionResolver;
    hydra::infrastructure::GeminiCliAdapter geminiCliAdapter;
    hydra::infrastructure::GeminiSessionResolver geminiSessionResolver;
    hydra::infrastructure::ClaudeCodeAdapter claudeCodeAdapter;
    hydra::infrastructure::TmuxTerminalBackend terminalBackend;
    hydra::domain::ProviderCatalog providerCatalog({&codexCliAdapter,
                                                    &geminiCliAdapter,
                                                    &claudeCodeAdapter});
    hydra::domain::StatusAggregator statusAggregator;
    hydra::domain::SessionSupervisor sessionSupervisor(repoStore,
                                                       sessionStore,
                                                       tmuxAdapter,
                                                       terminalBackend,
                                                       QVector<hydra::domain::ports::ProviderSessionResolver *>{&codexSessionResolver,
                                                                                                                &geminiSessionResolver},
                                                       providerCatalog,
                                                       statusAggregator,
                                                       QStringLiteral("owner-session-order-smoke"));

    const auto repoOutcome = repoRegistry.ensureRepository(repoPath,
                                                           QStringLiteral("Hydra Smoke"),
                                                           QStringLiteral(""),
                                                           QStringLiteral("#B38A59"));
    require(repoOutcome.ok, repoOutcome.errorMessage);
    seedSessions(sessionStore, repoOutcome.repository.id);

    {
        hydra::ui::AppState appState(repoRegistry,
                                     worktreeManager,
                                     sessionSupervisor,
                                     providerCatalog,
                                     dbPath,
                                     false);
        appState.reload();
        require(!appState.sessionAutosortEnabled(),
                QStringLiteral("Autosort should default to disabled"));
        require(modelOrder(appState) == QStringList{QStringLiteral("thinking"),
                                                    QStringLiteral("idle"),
                                                    QStringLiteral("approval")},
                QStringLiteral("Initial manual order should preserve stored order"));
        require(appState.moveSessionToIndex(QStringLiteral("approval"), 0),
                QStringLiteral("Manual move should succeed"));
        require(modelOrder(appState) == QStringList{QStringLiteral("approval"),
                                                    QStringLiteral("thinking"),
                                                    QStringLiteral("idle")},
                QStringLiteral("Manual move should change presentation order"));
        appState.setSessionAutosortEnabled(true);
        require(modelOrder(appState) == QStringList{QStringLiteral("idle"),
                                                    QStringLiteral("approval"),
                                                    QStringLiteral("thinking")},
                QStringLiteral("Autosort should group idle, approval, then active"));
        appState.setSessionAutosortEnabled(false);
        require(modelOrder(appState) == QStringList{QStringLiteral("approval"),
                                                    QStringLiteral("thinking"),
                                                    QStringLiteral("idle")},
                QStringLiteral("Disabling autosort should restore persisted manual order"));
    }

    {
        hydra::ui::AppState appState(repoRegistry,
                                     worktreeManager,
                                     sessionSupervisor,
                                     providerCatalog,
                                     dbPath,
                                     false);
        appState.reload();
        require(!appState.sessionAutosortEnabled(),
                QStringLiteral("Autosort should remain disabled after reload"));
        require(modelOrder(appState) == QStringList{QStringLiteral("approval"),
                                                    QStringLiteral("thinking"),
                                                    QStringLiteral("idle")},
                QStringLiteral("Manual session order should persist across reload"));
    }

    QTextStream(stdout) << "session order smoke passed" << Qt::endl;
    return 0;
}
