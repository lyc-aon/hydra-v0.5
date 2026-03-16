#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTextStream>

#include <stdexcept>

#include "domain/models/status_provenance.hpp"
#include "domain/ports/terminal_backend.hpp"
#include "domain/services/provider_catalog.hpp"
#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/status_aggregator.hpp"
#include "infrastructure/persistence/sqlite_repo_store.hpp"
#include "infrastructure/persistence/sqlite_session_store.hpp"
#include "infrastructure/persistence/database_manager.hpp"
#include "infrastructure/providers/claude_code_adapter.hpp"
#include "infrastructure/providers/codex_cli_adapter.hpp"
#include "infrastructure/providers/gemini_cli_adapter.hpp"
#include "infrastructure/providers/hermes_cli_adapter.hpp"
#include "infrastructure/providers/hermes_session_resolver.hpp"
#include "infrastructure/providers/opencode_cli_adapter.hpp"
#include "infrastructure/providers/opencode_session_resolver.hpp"

namespace {

void require(bool condition, const QString &message)
{
    if (!condition) {
        throw std::runtime_error(message.toStdString());
    }
}

void writeTextFile(const QString &path, const QByteArray &content, const QString &errorMessage)
{
    QFile file(path);
    require(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text), errorMessage);
    require(file.write(content) == content.size(), errorMessage);
}

void writeExecutableFile(const QString &path,
                         const QByteArray &content,
                         const QString &errorMessage)
{
    writeTextFile(path, content, errorMessage);
    QFile file(path);
    require(file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                    | QFileDevice::ExeOwner | QFileDevice::ReadGroup
                                    | QFileDevice::ExeGroup | QFileDevice::ReadOther
                                    | QFileDevice::ExeOther),
            errorMessage);
}

void requireArgumentsContain(const QStringList &arguments,
                             const QStringList &expectedTokens,
                             const QString &message)
{
    for (const QString &token : expectedTokens) {
        if (!arguments.contains(token)) {
            throw std::runtime_error(message.toStdString());
        }
    }
}

void requireEnvironmentExcludes(const QHash<QString, QString> &environment,
                                const QStringList &keys,
                                const QString &message)
{
    for (const QString &key : keys) {
        if (environment.contains(key)) {
            throw std::runtime_error(message.toStdString());
        }
    }
}

void requireEnvironmentPathContains(const QHash<QString, QString> &environment,
                                    const QString &key,
                                    const QString &fragment,
                                    const QString &message)
{
    const QString path = environment.value(key).trimmed();
    require(!path.isEmpty() && path.contains(fragment), message);
}

void requireSymlinkTarget(const QString &path,
                          const QString &expectedTarget,
                          const QString &message)
{
    const QFileInfo info(path);
    require(info.isSymLink()
                && QDir::cleanPath(info.symLinkTarget()) == QDir::cleanPath(expectedTarget),
            message);
}

class CaptureMuxAdapter final : public hydra::domain::ports::MuxAdapter {
public:
    hydra::domain::ports::MuxProbeResult probeAvailability() const override
    {
        return {.available = true,
                .executablePath = QStringLiteral("/usr/bin/tmux"),
                .message = QStringLiteral("ok")};
    }

    hydra::domain::ports::MuxLaunchResult launchDetachedSession(
        const hydra::domain::ports::MuxLaunchRequest &request) override
    {
        launchRequests.push_back(request);
        return {.ok = true,
                .sessionName = request.sessionName,
                .paneId = QStringLiteral("%%launch-override-pane"),
                .errorMessage = {}};
    }

    hydra::domain::ports::MuxTerminateResult terminateSession(const QString &) const override
    {
        return {.ok = true, .errorMessage = {}};
    }

    bool hasLiveSession(const QString &) const override { return true; }

    bool syncWatchedSessions(const QVector<hydra::domain::ports::MuxWatchRequest> &,
                             QString *errorMessage = nullptr) override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    QHash<QString, QVector<hydra::domain::ports::MuxEvent>> takePendingEvents() override
    {
        return {};
    }

    void stopWatchingSession(const QString &) override {}

    QString capturePaneTail(const QString &, int) const override { return {}; }

    QString capturePaneTitle(const QString &) const override { return {}; }

    QVector<hydra::domain::ports::MuxLaunchRequest> launchRequests;
};

class CaptureProviderAdapter final : public hydra::domain::ports::ProviderAdapter {
public:
    QString providerKey() const override { return QStringLiteral("capture-provider"); }
    QString displayName() const override { return QStringLiteral("Capture Provider"); }

    hydra::domain::ports::ProviderProbe probe() const override
    {
        return {.providerKey = providerKey(),
                .displayName = displayName(),
                .available = true,
                .executablePath = QStringLiteral("/usr/bin/capture-provider"),
                .versionString = QStringLiteral("smoke"),
                .statusMessage = QStringLiteral("ok")};
    }

    bool supportsModelOverride() const override { return true; }
    bool supportsResume() const override { return true; }

    QString initialResumeToken() const override
    {
        return QStringLiteral("capture-resume-token");
    }

    QString defaultApiKeyEnvironmentVariable() const override
    {
        return QStringLiteral("CAPTURE_API_KEY");
    }

    hydra::domain::ports::ProviderLaunchPlan buildLaunchPlan(
        hydra::domain::LaunchSafety,
        hydra::domain::ports::ProviderInvocationOptions options) const override
    {
        lastLaunchOptions = options;

        QStringList arguments = {
            QStringLiteral("--mode"),
            QStringLiteral("interactive"),
        };
        if (!options.modelOverride.trimmed().isEmpty()) {
            arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
        }

        return {.executable = QStringLiteral("capture-provider"),
                .arguments = arguments,
                .environment = {}};
    }

    hydra::domain::ports::ProviderLaunchPlan buildResumePlan(
        hydra::domain::LaunchSafety,
        hydra::domain::ports::ProviderInvocationOptions options) const override
    {
        lastResumeOptions = options;

        QStringList arguments = {
            QStringLiteral("--mode"),
            QStringLiteral("resume"),
            QStringLiteral("--resume-token"),
            options.resumeToken.trimmed(),
        };
        if (!options.modelOverride.trimmed().isEmpty()) {
            arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
        }
        if (!options.providerProfileMode.trimmed().isEmpty()) {
            arguments << QStringLiteral("--profile-mode") << options.providerProfileMode.trimmed();
        }
        if (!options.providerProfilePath.trimmed().isEmpty()) {
            arguments << QStringLiteral("--profile-path") << options.providerProfilePath.trimmed();
        }

        return {.executable = QStringLiteral("capture-provider"),
                .arguments = arguments,
                .environment = {}};
    }

    QString launchSafetySummary(hydra::domain::LaunchSafety) const override
    {
        return QStringLiteral("smoke");
    }

    hydra::domain::ports::ProviderObservabilityHints observabilityHints() const override
    {
        return {};
    }

    mutable hydra::domain::ports::ProviderInvocationOptions lastLaunchOptions;
    mutable hydra::domain::ports::ProviderInvocationOptions lastResumeOptions;
};

class CaptureTerminalBackend final : public hydra::domain::ports::TerminalBackend {
public:
    bool pasteText(const QString &target,
                   const QString &text,
                   QString *errorMessage = nullptr) const override
    {
        pasteTargets.push_back(target);
        pastedTexts.push_back(text);
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool sendSpecialKey(const QString &target,
                        const QString &keySequence,
                        int repeatCount = 1,
                        QString *errorMessage = nullptr) const override
    {
        Q_UNUSED(repeatCount);

        keyTargets.push_back(target);
        sentKeys.push_back(keySequence);
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool prepareInteractiveAttach(const QString &, bool, QString *errorMessage = nullptr) const override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool scrollHistory(const QString &, int, QString *errorMessage = nullptr) const override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    bool openExternalAttach(const QString &,
                            const QString &,
                            QString *errorMessage = nullptr) const override
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    mutable QStringList pasteTargets;
    mutable QStringList pastedTexts;
    mutable QStringList keyTargets;
    mutable QStringList sentKeys;
};

}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra Provider Launch Overrides Smoke"));

    try {
        QTemporaryDir tempRoot;
        require(tempRoot.isValid(), QStringLiteral("Failed to create temp root."));

        const QString rootPath = tempRoot.path();
        qputenv("XDG_DATA_HOME", QDir(rootPath).filePath(QStringLiteral("xdg-data")).toUtf8());
        qputenv("XDG_STATE_HOME", QDir(rootPath).filePath(QStringLiteral("xdg-state")).toUtf8());
        qputenv("XDG_CACHE_HOME", QDir(rootPath).filePath(QStringLiteral("xdg-cache")).toUtf8());
        const QString sharedHermesHome = QDir(rootPath).filePath(QStringLiteral("shared-hermes"));
        QDir().mkpath(QDir(sharedHermesHome).filePath(QStringLiteral("skills")));
        QDir().mkpath(QDir(sharedHermesHome).filePath(QStringLiteral("memories")));
        QDir().mkpath(QDir(sharedHermesHome).filePath(QStringLiteral("hooks")));
        writeTextFile(QDir(sharedHermesHome).filePath(QStringLiteral(".env")),
                      QByteArrayLiteral("OPENROUTER_API_KEY=test-hermes-key\n"),
                      QStringLiteral("Failed to seed shared Hermes .env."));
        writeTextFile(QDir(sharedHermesHome).filePath(QStringLiteral("config.yaml")),
                      QByteArrayLiteral("model: anthropic/claude-opus-4.6\n"),
                      QStringLiteral("Failed to seed shared Hermes config.yaml."));
        qputenv("HYDRA_HERMES_SHARED_HOME", sharedHermesHome.toUtf8());

        const QString fakeBinDir = QDir(rootPath).filePath(QStringLiteral("bin"));
        QDir().mkpath(fakeBinDir);
        writeExecutableFile(
            QDir(fakeBinDir).filePath(QStringLiteral("opencode")),
            QByteArrayLiteral(
                "#!/usr/bin/env bash\n"
                "set -euo pipefail\n"
                "if [[ \"${1-}\" == \"--version\" ]]; then\n"
                "  printf '1.2.26\\n'\n"
                "  exit 0\n"
                "fi\n"
                "if [[ \"${1-}\" == \"session\" && \"${2-}\" == \"list\" ]]; then\n"
                "  data_home=\"${XDG_DATA_HOME:-$HOME/.local/share}\"\n"
                "  file=\"$data_home/opencode/test-sessions.json\"\n"
                "  if [[ -f \"$file\" ]]; then\n"
                "    cat \"$file\"\n"
                "  else\n"
                "    printf '[]\\n'\n"
                "  fi\n"
                "  exit 0\n"
                "fi\n"
                "printf 'unexpected opencode invocation: %s\\n' \"$*\" >&2\n"
                "exit 1\n"),
            QStringLiteral("Failed to seed fake opencode helper."));
        qputenv("PATH",
                QStringLiteral("%1:%2")
                    .arg(fakeBinDir, qEnvironmentVariable("PATH"))
                    .toUtf8());

        const QString repoPath = QDir(rootPath).filePath(QStringLiteral("repo"));
        QDir().mkpath(repoPath);

        const QString dbPath = QDir(rootPath).filePath(QStringLiteral("hydra.sqlite3"));
        hydra::infrastructure::DatabaseManager databaseManager(dbPath);
        hydra::infrastructure::SqliteRepoStore repoStore(databaseManager.database());
        hydra::infrastructure::SqliteSessionStore sessionStore(databaseManager.database());
        hydra::domain::RepoRegistry repoRegistry(repoStore);

        const auto repoOutcome = repoRegistry.ensureRepository(
            repoPath,
            QStringLiteral("Smoke Repo"),
            QStringLiteral(""),
            QStringLiteral("#b37748"));
        require(repoOutcome.ok,
                QStringLiteral("Failed to seed repository: %1").arg(repoOutcome.errorMessage));

        CaptureMuxAdapter muxAdapter;
        CaptureProviderAdapter providerAdapter;
        CaptureTerminalBackend terminalBackend;
        hydra::domain::ProviderCatalog providerCatalog({&providerAdapter});
        hydra::domain::StatusAggregator statusAggregator;
        hydra::domain::SessionSupervisor supervisor(repoStore,
                                                    sessionStore,
                                                    muxAdapter,
                                                    terminalBackend,
                                                    {},
                                                    providerCatalog,
                                                    statusAggregator,
                                                    QStringLiteral("provider-launch-overrides-smoke"));

        const QHash<QString, QString> launchEnvironment = {
            {QStringLiteral("CAPTURE_API_KEY"), QStringLiteral("secret-123")},
        };

        const hydra::domain::LaunchOutcome outcome = supervisor.launchSession(
            repoOutcome.repository.id,
            providerAdapter.providerKey(),
            hydra::domain::LaunchSafety::WorkspaceSafe,
            repoPath,
            QStringLiteral("Router"),
            hydra::domain::SessionCategory::Router,
            QStringLiteral("router-model-override"),
            launchEnvironment,
            QStringLiteral("repo-template"),
            QStringLiteral(".hydra/capture-profile"));

        require(outcome.ok && outcome.session.has_value(),
                QStringLiteral("Launch failed: %1").arg(outcome.message));

        require(providerAdapter.lastLaunchOptions.modelOverride == QStringLiteral("router-model-override"),
                QStringLiteral("Provider did not receive the model override."));
        require(providerAdapter.lastLaunchOptions.sessionCategory
                    == hydra::domain::SessionCategory::Router,
                QStringLiteral("Provider did not receive the router session category."));
        require(providerAdapter.lastLaunchOptions.providerProfileMode == QStringLiteral("repo-template")
                    && providerAdapter.lastLaunchOptions.providerProfilePath
                           == QStringLiteral(".hydra/capture-profile"),
                QStringLiteral("Provider did not receive the provider profile options."));

        require(muxAdapter.launchRequests.size() == 1,
                QStringLiteral("Expected exactly one mux launch request."));

        const hydra::domain::ports::MuxLaunchRequest request = muxAdapter.launchRequests.constFirst();
        require(request.environment.value(QStringLiteral("CAPTURE_API_KEY"))
                    == QStringLiteral("secret-123"),
                QStringLiteral("Mux launch request did not receive the env override."));
        requireArgumentsContain(request.arguments,
                                {QStringLiteral("--model"),
                                 QStringLiteral("router-model-override")},
                                QStringLiteral("Mux launch request did not receive the model arguments."));

        const auto storedSession = sessionStore.sessionById(outcome.session->id);
        require(storedSession.has_value()
                    && storedSession->modelOverride == QStringLiteral("router-model-override")
                    && storedSession->launchEnvironment.value(QStringLiteral("CAPTURE_API_KEY"))
                           == QStringLiteral("secret-123")
                    && storedSession->providerProfileMode == QStringLiteral("repo-template")
                    && storedSession->providerProfilePath == QStringLiteral(".hydra/capture-profile"),
                QStringLiteral("Launch persistence did not retain resume-critical provider config."));
        require(terminalBackend.pastedTexts.isEmpty() && terminalBackend.sentKeys.isEmpty(),
                QStringLiteral("Unexpected bootstrap input for capture provider."));

        hydra::domain::SessionRecord resumableSession = storedSession.value();
        resumableSession.state = hydra::domain::SessionState::Exited;
        resumableSession.tmuxSessionName.clear();
        resumableSession.tmuxPaneId.clear();
        resumableSession.ownerInstanceId.clear();
        resumableSession.updatedAt = QDateTime::currentDateTimeUtc();
        QString resumePersistError;
        require(sessionStore.upsertSession(resumableSession, &resumePersistError),
                QStringLiteral("Failed to persist resumable capture session: %1")
                    .arg(resumePersistError));

        const hydra::domain::LaunchOutcome resumeOutcome = supervisor.resumeSession(
            resumableSession.id,
            hydra::domain::LaunchSafety::WorkspaceSafe);
        require(resumeOutcome.ok && resumeOutcome.session.has_value(),
                QStringLiteral("Resume failed: %1").arg(resumeOutcome.message));
        require(providerAdapter.lastResumeOptions.resumeToken
                    == QStringLiteral("capture-resume-token")
                    && providerAdapter.lastResumeOptions.modelOverride
                           == QStringLiteral("router-model-override")
                    && providerAdapter.lastResumeOptions.providerProfileMode
                           == QStringLiteral("repo-template")
                    && providerAdapter.lastResumeOptions.providerProfilePath
                           == QStringLiteral(".hydra/capture-profile"),
                QStringLiteral("Resume did not replay the stored provider invocation options."));
        require(muxAdapter.launchRequests.size() == 2,
                QStringLiteral("Expected a second mux launch request for resume."));
        const hydra::domain::ports::MuxLaunchRequest resumeRequest = muxAdapter.launchRequests.at(1);
        require(resumeRequest.environment.value(QStringLiteral("CAPTURE_API_KEY"))
                    == QStringLiteral("secret-123"),
                QStringLiteral("Resume did not replay the stored launch environment."));
        requireArgumentsContain(resumeRequest.arguments,
                                {QStringLiteral("--resume-token"),
                                 QStringLiteral("capture-resume-token")},
                                QStringLiteral("Resume launch did not receive the stored resume token."));

    hydra::infrastructure::CodexCliAdapter codexAdapter;
    const hydra::domain::ports::ProviderLaunchPlan codexPlan = codexAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::WorkspaceSafe,
        {.resumeToken = QString(),
         .modelOverride = QStringLiteral("gpt-5.3-flash"),
         .sessionCategory = hydra::domain::SessionCategory::Router,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = repoPath});
        requireEnvironmentExcludes(codexPlan.environment,
                                   {QStringLiteral("HOME"), QStringLiteral("HERMES_HOME")},
                                   QStringLiteral("Codex launch unexpectedly overrides provider home environment."));
        requireEnvironmentPathContains(codexPlan.environment,
                                       QStringLiteral("CODEX_HOME"),
                                       QStringLiteral("/providers/codex/"),
                                       QStringLiteral("Codex launch did not isolate CODEX_HOME for router sessions."));
        require(!codexPlan.arguments.contains(QStringLiteral("tui.animations=false"))
                    && !codexPlan.arguments.contains(QStringLiteral("tui.show_tooltips=false"))
                    && !codexPlan.arguments.contains(QStringLiteral("tui.status_line=[]"))
                    && !codexPlan.arguments.contains(QStringLiteral("tui.notifications=false")),
                QStringLiteral("Codex launch still suppresses native TUI chrome that should remain provider-native."));
        require(codexPlan.arguments.contains(QStringLiteral("--no-alt-screen")),
                QStringLiteral("Codex launch no longer keeps the embed-critical no-alt-screen flag."));

    hydra::infrastructure::ClaudeCodeAdapter claudeAdapter;
    const hydra::domain::ports::ProviderLaunchPlan claudePlan = claudeAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::WorkspaceSafe,
        {.resumeToken = QStringLiteral("claude-session-smoke"),
         .modelOverride = QStringLiteral("claude-opus-4.6"),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
        requireEnvironmentExcludes(claudePlan.environment,
                                   {QStringLiteral("HOME"),
                                    QStringLiteral("CLAUDE_HOME"),
                                    QStringLiteral("HERMES_HOME")},
                                   QStringLiteral("Claude launch unexpectedly overrides provider home environment."));
    const hydra::domain::ports::ProviderObservabilityHints claudeHints =
        claudeAdapter.observabilityHints();
        require(claudeHints.preferRecentInputHoldBeforeTailSignals
                    && claudeHints.preferActiveTitleDuringRecentInputHold
                    && claudeHints.recentInputStateHoldMs >= 8000,
                QStringLiteral("Claude observability hints no longer preserve active state during refresh churn."));

    hydra::domain::SessionRecord claudeSessionRecord;
    claudeSessionRecord.id = QStringLiteral("claude-session");
    claudeSessionRecord.name = QStringLiteral("Claude Preview [Claude Code]");
    claudeSessionRecord.providerKey = QStringLiteral("claude");
    claudeSessionRecord.tmuxSessionName = QStringLiteral("hydra-claude-preview-38ab4e7f");
    claudeSessionRecord.tmuxPaneId = QStringLiteral("%612");
    claudeSessionRecord.state = hydra::domain::SessionState::Thinking;
    claudeSessionRecord.statusDetail = QStringLiteral("Generating session preview");
    claudeSessionRecord.statusProvenance = hydra::domain::StatusProvenance::ProviderHint;
    claudeSessionRecord.updatedAt = QDateTime::currentDateTimeUtc();

    hydra::domain::SessionSignal claudePaneSignal;
    claudePaneSignal.sessionId = claudeSessionRecord.id;
    claudePaneSignal.sessionName = claudeSessionRecord.tmuxSessionName;
    claudePaneSignal.paneId = claudeSessionRecord.tmuxPaneId;
    claudePaneSignal.kind = hydra::domain::SessionSignalKind::PaneOutputObserved;
    claudePaneSignal.provenance = hydra::domain::StatusProvenance::Tmux;
    claudePaneSignal.occurredAt = QDateTime::currentDateTimeUtc();

    const hydra::domain::SessionStatusUpdate claudeHeldUpdate =
        statusAggregator.applySignal(
            claudeSessionRecord,
            claudePaneSignal,
            claudeHints,
            QStringLiteral("❯ revise the session preview\n? for shortcuts\n"),
            QString());
        require(claudeHeldUpdate.state == hydra::domain::SessionState::Thinking,
                QStringLiteral("Claude observability still drops to idle on stale prompt tails during active work."));

    const hydra::domain::SessionStatusUpdate claudeActiveTitleUpdate =
        statusAggregator.applySignal(
            claudeSessionRecord,
            claudePaneSignal,
            claudeHints,
            QStringLiteral("❯ revise the session preview\n"),
            QStringLiteral("● Claude Code"));
        require(claudeActiveTitleUpdate.state == hydra::domain::SessionState::Thinking,
                QStringLiteral("Claude observability no longer recognizes active Claude title variants."));

    hydra::infrastructure::GeminiCliAdapter geminiAdapter;
    const hydra::domain::ports::ProviderLaunchPlan geminiRouterPlan = geminiAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::Bypass,
        {.resumeToken = QString(),
         .modelOverride = QStringLiteral("gemini-3-flash"),
         .sessionCategory = hydra::domain::SessionCategory::Router,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = repoPath});
        requireEnvironmentExcludes(geminiRouterPlan.environment,
                                   {QStringLiteral("HOME"), QStringLiteral("HERMES_HOME")},
                                   QStringLiteral("Gemini launch unexpectedly overrides provider home environment."));
        requireEnvironmentPathContains(geminiRouterPlan.environment,
                                       QStringLiteral("GEMINI_CLI_HOME"),
                                       QStringLiteral("/providers/gemini/"),
                                       QStringLiteral("Gemini launch did not isolate GEMINI_CLI_HOME for router sessions."));
        require(!geminiRouterPlan.arguments.contains(QStringLiteral("--screen-reader")),
                QStringLiteral("Gemini router launch unexpectedly enabled screen-reader mode."));
        const QString geminiSettingsPath = QDir(geminiRouterPlan.environment
                                                    .value(QStringLiteral("GEMINI_CLI_HOME")))
                                               .filePath(QStringLiteral(".gemini/settings.json"));
        QFile geminiSettingsFile(geminiSettingsPath);
        require(geminiSettingsFile.open(QIODevice::ReadOnly | QIODevice::Text),
                QStringLiteral("Gemini isolated settings were not created."));
        const QJsonObject geminiSettings =
            QJsonDocument::fromJson(geminiSettingsFile.readAll()).object();
        const QJsonObject geminiUiSettings = geminiSettings.value(QStringLiteral("ui")).toObject();
        require(geminiUiSettings.value(QStringLiteral("useAlternateBuffer")).toBool(true) == false,
                QStringLiteral("Gemini launch no longer disables the alternate buffer for embedding."));
        require(!geminiUiSettings.contains(QStringLiteral("showSpinner"))
                    && !geminiUiSettings.contains(QStringLiteral("loadingPhrases")),
                QStringLiteral("Gemini launch still suppresses native spinner/loading UI that should remain provider-native."));

    const hydra::domain::ports::ProviderLaunchPlan geminiWorkerPlan = geminiAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::Bypass,
        {.resumeToken = QString(),
         .modelOverride = QStringLiteral("gemini-3-flash"),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
        require(!geminiWorkerPlan.environment.contains(QStringLiteral("GEMINI_CLI_HOME")),
                QStringLiteral("Gemini worker launch unexpectedly isolates GEMINI_CLI_HOME."));
        require(!geminiWorkerPlan.arguments.contains(QStringLiteral("--screen-reader")),
                QStringLiteral("Gemini worker launch unexpectedly enabled screen-reader mode."));

    hydra::infrastructure::HermesCliAdapter hermesAdapter;
    const QString hermesResumePendingToken = hermesAdapter.initialResumeToken();
        require(hermesResumePendingToken.startsWith(QStringLiteral("hermes-pending-hydra-hermes-")),
                QStringLiteral("Hermes initial resume token did not include the pending title prefix."));

    const QString hermesResolvedResumeToken =
        hermesAdapter.expectedResolvedResumeToken(hermesResumePendingToken);
        require(hermesResolvedResumeToken.startsWith(QStringLiteral("hydra-hermes-")),
                QStringLiteral("Hermes expected resolved resume token was malformed."));

    const QString hermesBootstrapText = hermesAdapter.postLaunchBootstrapText(
        {.resumeToken = hermesResumePendingToken,
         .modelOverride = QString(),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
        require(hermesBootstrapText.isEmpty(),
                QStringLiteral("Hermes should not inject visible bootstrap text into the terminal."));

    const hydra::domain::ports::ProviderLaunchPlan hermesPlan = hermesAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::Bypass,
        {.resumeToken = hermesResumePendingToken,
         .modelOverride = QStringLiteral("anthropic/claude-opus-4.6"),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QStringLiteral("global"),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
        require(hermesPlan.arguments.value(0) == QStringLiteral("chat")
                    && hermesPlan.arguments.contains(QStringLiteral("--yolo")),
                QStringLiteral("Hermes launch plan did not use interactive chat with yolo mode."));
        requireArgumentsContain(hermesPlan.arguments,
                                {QStringLiteral("--model"),
                                 QStringLiteral("anthropic/claude-opus-4.6")},
                                QStringLiteral("Hermes launch plan did not include the model override."));
        const QString hermesHomePath = hermesPlan.environment.value(QStringLiteral("HERMES_HOME")).trimmed();
        require(!hermesHomePath.isEmpty() && hermesHomePath.endsWith(hermesResolvedResumeToken),
                QStringLiteral("Hermes launch plan did not isolate state with a tokenized HERMES_HOME."));
        requireSymlinkTarget(QDir(hermesHomePath).filePath(QStringLiteral(".env")),
                             QDir(sharedHermesHome).filePath(QStringLiteral(".env")),
                             QStringLiteral("Hermes launch plan did not link the shared .env into the isolated home."));
        requireSymlinkTarget(QDir(hermesHomePath).filePath(QStringLiteral("config.yaml")),
                             QDir(sharedHermesHome).filePath(QStringLiteral("config.yaml")),
                             QStringLiteral("Hermes launch plan did not link the shared config into the isolated home."));
        requireSymlinkTarget(QDir(hermesHomePath).filePath(QStringLiteral("skills")),
                             QDir(sharedHermesHome).filePath(QStringLiteral("skills")),
                             QStringLiteral("Hermes launch plan did not link the shared skills directory."));
        requireSymlinkTarget(QDir(hermesHomePath).filePath(QStringLiteral("memories")),
                             QDir(sharedHermesHome).filePath(QStringLiteral("memories")),
                             QStringLiteral("Hermes launch plan did not link the shared memories directory."));

    const QString hermesBlankPendingToken = hermesAdapter.initialResumeToken();
    const QString hermesBlankResolvedToken =
        hermesAdapter.expectedResolvedResumeToken(hermesBlankPendingToken);
    const hydra::domain::ports::ProviderLaunchPlan hermesBlankPlan = hermesAdapter.buildLaunchPlan(
        hydra::domain::LaunchSafety::WorkspaceSafe,
        {.resumeToken = hermesBlankPendingToken,
         .modelOverride = QString(),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QStringLiteral("blank"),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
    const QString hermesBlankHomePath =
        hermesBlankPlan.environment.value(QStringLiteral("HERMES_HOME")).trimmed();
        require(!hermesBlankHomePath.isEmpty()
                    && hermesBlankHomePath.endsWith(hermesBlankResolvedToken),
                QStringLiteral("Hermes blank profile did not get an isolated home."));
        require(!QFileInfo(QDir(hermesBlankHomePath).filePath(QStringLiteral(".env"))).exists()
                    && !QFileInfo(QDir(hermesBlankHomePath).filePath(QStringLiteral("config.yaml"))).exists(),
                QStringLiteral("Hermes blank profile unexpectedly inherited shared config assets."));

        const QString repoTemplateHome =
            QDir(rootPath).filePath(QStringLiteral("repo-template-hermes"));
        QDir().mkpath(QDir(repoTemplateHome).filePath(QStringLiteral("skills")));
        writeTextFile(QDir(repoTemplateHome).filePath(QStringLiteral("config.yaml")),
                      QByteArrayLiteral("model: google/gemini-3-pro\n"),
                      QStringLiteral("Failed to seed repo-template Hermes config."));
        writeTextFile(QDir(repoTemplateHome).filePath(QStringLiteral("SOUL.md")),
                      QByteArrayLiteral("Repo-specific Hermes persona\n"),
                      QStringLiteral("Failed to seed repo-template Hermes SOUL.md."));

        const QString hermesRepoTemplatePendingToken = hermesAdapter.initialResumeToken();
        const hydra::domain::ports::ProviderLaunchPlan hermesRepoTemplatePlan =
            hermesAdapter.buildLaunchPlan(
                hydra::domain::LaunchSafety::WorkspaceSafe,
                {.resumeToken = hermesRepoTemplatePendingToken,
                 .modelOverride = QString(),
                 .sessionCategory = hydra::domain::SessionCategory::Worker,
                 .providerProfileMode = QStringLiteral("repo-template"),
                 .providerProfilePath = repoTemplateHome,
                 .workingDirectory = QString()});
        const QString hermesRepoTemplateHomePath =
            hermesRepoTemplatePlan.environment.value(QStringLiteral("HERMES_HOME")).trimmed();
        requireSymlinkTarget(QDir(hermesRepoTemplateHomePath).filePath(QStringLiteral("config.yaml")),
                             QDir(repoTemplateHome).filePath(QStringLiteral("config.yaml")),
                             QStringLiteral("Hermes repo-template profile did not link template config.yaml."));
        requireSymlinkTarget(QDir(hermesRepoTemplateHomePath).filePath(QStringLiteral("SOUL.md")),
                             QDir(repoTemplateHome).filePath(QStringLiteral("SOUL.md")),
                             QStringLiteral("Hermes repo-template profile did not link template SOUL.md."));
        requireSymlinkTarget(QDir(hermesRepoTemplateHomePath).filePath(QStringLiteral(".env")),
                             QDir(sharedHermesHome).filePath(QStringLiteral(".env")),
                             QStringLiteral("Hermes repo-template profile did not fall back to shared .env."));
        requireSymlinkTarget(QDir(hermesRepoTemplateHomePath).filePath(QStringLiteral("skills")),
                             QDir(repoTemplateHome).filePath(QStringLiteral("skills")),
                             QStringLiteral("Hermes repo-template profile did not link template skills."));

    const hydra::domain::ports::ProviderLaunchPlan hermesResumePlan = hermesAdapter.buildResumePlan(
        hydra::domain::LaunchSafety::Bypass,
        {.resumeToken = hermesResolvedResumeToken,
         .modelOverride = QStringLiteral("anthropic/claude-opus-4.6"),
         .sessionCategory = hydra::domain::SessionCategory::Worker,
         .providerProfileMode = QString(),
         .providerProfilePath = QString(),
         .workingDirectory = QString()});
        require(hermesResumePlan.arguments.contains(QStringLiteral("--continue"))
                    && !hermesResumePlan.arguments.contains(QStringLiteral("--resume")),
                QStringLiteral("Hermes resume plan did not use isolated-home continue mode."));
        require(hermesResumePlan.environment.value(QStringLiteral("HERMES_HOME")).trimmed()
                    == hermesHomePath,
                QStringLiteral("Hermes resume plan did not reuse the launch HERMES_HOME."));

    const QHash<QString, QString> hermesPromptEnvironment =
        hermesAdapter.promptEnvironment(QStringLiteral("Hydra router instructions"));
        require(hermesPromptEnvironment.value(QStringLiteral("HERMES_EPHEMERAL_SYSTEM_PROMPT"))
                    == QStringLiteral("Hydra router instructions"),
                QStringLiteral("Hermes prompt environment was not populated."));

    const hydra::domain::ports::ProviderObservabilityHints hermesHints =
        hermesAdapter.observabilityHints();
        require(!hermesHints.allowGenericActiveTailFallback,
                QStringLiteral("Hermes observability hints still allow generic active-tail fallback."));

    hydra::domain::SessionRecord hermesSessionRecord;
    hermesSessionRecord.id = QStringLiteral("hermes-session");
    hermesSessionRecord.name = QStringLiteral("Hermes Testing [Hermes]");
    hermesSessionRecord.providerKey = QStringLiteral("hermes");
    hermesSessionRecord.tmuxSessionName = QStringLiteral("hydra-hermes-testing-bad48431");
    hermesSessionRecord.tmuxPaneId = QStringLiteral("%760");
    hermesSessionRecord.state = hydra::domain::SessionState::Thinking;
    hermesSessionRecord.statusDetail =
        QStringLiteral("- Dev server: Running at `http://localhost:3000`");
    hermesSessionRecord.statusProvenance = hydra::domain::StatusProvenance::ProviderHint;
    hermesSessionRecord.updatedAt = QDateTime::currentDateTimeUtc();

    hydra::domain::SessionSignal hermesPaneSignal;
    hermesPaneSignal.sessionId = hermesSessionRecord.id;
    hermesPaneSignal.sessionName = hermesSessionRecord.tmuxSessionName;
    hermesPaneSignal.paneId = hermesSessionRecord.tmuxPaneId;
    hermesPaneSignal.kind = hydra::domain::SessionSignalKind::PaneOutputObserved;
    hermesPaneSignal.provenance = hydra::domain::StatusProvenance::Tmux;
    hermesPaneSignal.occurredAt = QDateTime::currentDateTimeUtc();

    const QString hermesIdleTail = QStringLiteral(
        "## All Done - HYDRA Showcase Site Ready\n"
        "- Dev server: Running at `http://localhost:3000`\n"
        "❯\n");
    const hydra::domain::SessionStatusUpdate hermesIdleUpdate =
        statusAggregator.applySignal(
            hermesSessionRecord,
            hermesPaneSignal,
            hermesHints,
            hermesIdleTail,
            QString());
        require(hermesIdleUpdate.state == hydra::domain::SessionState::Idle
                    && hermesIdleUpdate.detail.contains(QStringLiteral("❯")),
                QStringLiteral("Hermes observability still misclassifies an idle prompt as active."));

    const QString hermesActiveTail = QStringLiteral(
        "(⊙_⊙) contemplating...\n"
        "⚕ ❯\n");
    const hydra::domain::SessionStatusUpdate hermesActiveUpdate =
        statusAggregator.applySignal(
            hermesSessionRecord,
            hermesPaneSignal,
            hermesHints,
            hermesActiveTail,
            QString());
        require(hermesActiveUpdate.state == hydra::domain::SessionState::Thinking,
                QStringLiteral("Hermes observability no longer recognizes a real busy prompt."));

    QDir().mkpath(hermesHomePath);
    const QString hermesDbPath = QDir(hermesHomePath).filePath(QStringLiteral("state.db"));
    const QString connectionName = QStringLiteral("hydra-provider-launch-overrides-hermes");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        database.setDatabaseName(hermesDbPath);
            require(database.open(), QStringLiteral("Failed to create Hermes SQLite test DB."));

        QSqlQuery query(database);
        require(query.exec(QStringLiteral(
                "CREATE TABLE sessions ("
                "id TEXT PRIMARY KEY,"
                "source TEXT NOT NULL,"
                "title TEXT,"
                "started_at REAL NOT NULL)")),
                QStringLiteral("Failed to create Hermes sessions test table."));

        query.prepare(QStringLiteral(
            "INSERT INTO sessions (id, source, title, started_at) VALUES (?, ?, ?, ?)"));
        query.addBindValue(QStringLiteral("session-one"));
        query.addBindValue(QStringLiteral("cli"));
        query.addBindValue(QStringLiteral("Hydra worker session"));
        query.addBindValue(1000.0);
            require(query.exec(), QStringLiteral("Failed to seed isolated Hermes session DB."));

        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    hydra::infrastructure::HermesSessionResolver hermesResolver;
        require(hermesResolver.sessionExists(hermesResolvedResumeToken),
                QStringLiteral("Hermes resolver could not find the isolated Hydra-managed session."));

    const QSet<QString> knownHermesIds =
        hermesResolver.knownSessionIdsForWorkingDirectory(QStringLiteral("/unused"));
        require(knownHermesIds.contains(hermesResolvedResumeToken),
                QStringLiteral("Hermes resolver did not enumerate isolated Hydra-managed sessions."));

    const QString discoveredHermesId = hermesResolver.findNewSessionId(
        QStringLiteral("/unused"),
        {},
        {});
        require(discoveredHermesId == hermesResolvedResumeToken,
                QStringLiteral("Hermes resolver did not discover the isolated Hydra-managed token."));

    hydra::infrastructure::OpenCodeCliAdapter openCodeAdapter;
    const hydra::domain::ports::ProviderLaunchPlan openCodeRouterPlan =
        openCodeAdapter.buildLaunchPlan(
            hydra::domain::LaunchSafety::WorkspaceSafe,
            {.resumeToken = QString(),
             .modelOverride = QStringLiteral("openai/gpt-5"),
             .sessionCategory = hydra::domain::SessionCategory::Router,
             .providerProfileMode = QString(),
             .providerProfilePath = QString(),
             .workingDirectory = repoPath});
        requireArgumentsContain(openCodeRouterPlan.arguments,
                                {QStringLiteral("--model"), QStringLiteral("openai/gpt-5")},
                                QStringLiteral("OpenCode launch plan did not include the model override."));
        requireEnvironmentPathContains(openCodeRouterPlan.environment,
                                       QStringLiteral("XDG_DATA_HOME"),
                                       QStringLiteral("/providers/opencode/"),
                                       QStringLiteral("OpenCode router launch did not isolate XDG_DATA_HOME."));
        requireEnvironmentPathContains(openCodeRouterPlan.environment,
                                       QStringLiteral("XDG_STATE_HOME"),
                                       QStringLiteral("/providers/opencode/"),
                                       QStringLiteral("OpenCode router launch did not isolate XDG_STATE_HOME."));
        requireEnvironmentPathContains(openCodeRouterPlan.environment,
                                       QStringLiteral("XDG_CACHE_HOME"),
                                       QStringLiteral("/providers/opencode/"),
                                       QStringLiteral("OpenCode router launch did not isolate XDG_CACHE_HOME."));
        const QJsonDocument openCodeRouterConfig =
            QJsonDocument::fromJson(openCodeRouterPlan.environment
                                        .value(QStringLiteral("OPENCODE_CONFIG_CONTENT"))
                                        .toUtf8());
        require(openCodeRouterConfig.isObject(),
                QStringLiteral("OpenCode router launch did not emit valid inline config JSON."));
        require(!openCodeRouterConfig.object().value(QStringLiteral("autoupdate")).toBool(true),
                QStringLiteral("OpenCode router launch did not disable autoupdate."));
        const QJsonObject openCodeRouterPermissions =
            openCodeRouterConfig.object().value(QStringLiteral("permission")).toObject();
        require(openCodeRouterPermissions.value(QStringLiteral("bash")).toString() == QStringLiteral("ask")
                    && openCodeRouterPermissions.value(QStringLiteral("edit")).toString()
                           == QStringLiteral("ask"),
                QStringLiteral("OpenCode workspace-safe launch did not force ask-mode permissions."));

    const hydra::domain::ports::ProviderLaunchPlan openCodeWorkerPlan =
        openCodeAdapter.buildLaunchPlan(
            hydra::domain::LaunchSafety::Bypass,
            {.resumeToken = QString(),
             .modelOverride = QStringLiteral("openai/gpt-5"),
             .sessionCategory = hydra::domain::SessionCategory::Worker,
             .providerProfileMode = QString(),
             .providerProfilePath = QString(),
             .workingDirectory = repoPath});
        requireEnvironmentExcludes(openCodeWorkerPlan.environment,
                                   {QStringLiteral("XDG_DATA_HOME"),
                                    QStringLiteral("XDG_STATE_HOME"),
                                    QStringLiteral("XDG_CACHE_HOME")},
                                   QStringLiteral("OpenCode worker launch unexpectedly isolated XDG paths."));
        const QJsonDocument openCodeWorkerConfig =
            QJsonDocument::fromJson(openCodeWorkerPlan.environment
                                        .value(QStringLiteral("OPENCODE_CONFIG_CONTENT"))
                                        .toUtf8());
        require(openCodeWorkerConfig.isObject()
                    && openCodeWorkerConfig.object().value(QStringLiteral("permission")).toString()
                           == QStringLiteral("allow"),
                QStringLiteral("OpenCode bypass launch did not force allow-mode permissions."));

    const hydra::domain::ports::ProviderLaunchPlan openCodeResumePlan =
        openCodeAdapter.buildResumePlan(
            hydra::domain::LaunchSafety::WorkspaceSafe,
            {.resumeToken = QStringLiteral("ses_smoke_opencode"),
             .modelOverride = QStringLiteral("openai/gpt-5"),
             .sessionCategory = hydra::domain::SessionCategory::Router,
             .providerProfileMode = QString(),
             .providerProfilePath = QString(),
             .workingDirectory = repoPath});
        requireArgumentsContain(openCodeResumePlan.arguments,
                                {QStringLiteral("--session"), QStringLiteral("ses_smoke_opencode")},
                                QStringLiteral("OpenCode resume plan did not use explicit session-id resume."));

        const QString sharedOpenCodeSessionsPath =
            QDir(qEnvironmentVariable("XDG_DATA_HOME")).filePath(QStringLiteral("opencode/test-sessions.json"));
        QDir().mkpath(QFileInfo(sharedOpenCodeSessionsPath).absolutePath());
        writeTextFile(sharedOpenCodeSessionsPath,
                      QByteArrayLiteral(
                          "[{\"id\":\"ses_shared_opencode\",\"title\":\"Shared\",\"updated\":10,"
                          "\"created\":1,\"projectId\":\"global\",\"directory\":\"")
                          + repoPath.toUtf8()
                          + QByteArrayLiteral("\"}]"),
                      QStringLiteral("Failed to seed shared OpenCode sessions."));

        const QString isolatedOpenCodeSessionsPath =
            QDir(hydra::infrastructure::OpenCodeSessionResolver::xdgDataHomePathForWorkingDirectory(
                     repoPath))
                .filePath(QStringLiteral("opencode/test-sessions.json"));
        QDir().mkpath(QFileInfo(isolatedOpenCodeSessionsPath).absolutePath());
        writeTextFile(isolatedOpenCodeSessionsPath,
                      QByteArrayLiteral(
                          "[{\"id\":\"ses_isolated_opencode\",\"title\":\"Isolated\",\"updated\":20,"
                          "\"created\":2,\"projectId\":\"global\",\"directory\":\"")
                          + repoPath.toUtf8()
                          + QByteArrayLiteral("\"}]"),
                      QStringLiteral("Failed to seed isolated OpenCode sessions."));

    hydra::infrastructure::OpenCodeSessionResolver openCodeResolver;
    const QSet<QString> knownOpenCodeIds =
        openCodeResolver.knownSessionIdsForWorkingDirectory(repoPath);
        require(knownOpenCodeIds.contains(QStringLiteral("ses_shared_opencode"))
                    && knownOpenCodeIds.contains(QStringLiteral("ses_isolated_opencode")),
                QStringLiteral("OpenCode resolver did not enumerate shared and isolated sessions."));

    const QString discoveredOpenCodeId = openCodeResolver.findNewSessionId(
        repoPath,
        {QStringLiteral("ses_shared_opencode")},
        {});
        require(discoveredOpenCodeId == QStringLiteral("ses_isolated_opencode"),
                QStringLiteral("OpenCode resolver did not discover the isolated session id."));
        require(openCodeResolver.sessionExists(QStringLiteral("ses_isolated_opencode"), repoPath),
                QStringLiteral("OpenCode resolver could not confirm the isolated session id."));
        const hydra::domain::ports::ProviderObservabilityHints openCodeHints =
            openCodeAdapter.observabilityHints();
        require(openCodeHints.preferRecentInputHoldBeforeTailSignals
                    && openCodeHints.recentInputStateHoldMs >= 8000,
                QStringLiteral("OpenCode observability no longer preserves active state briefly after local input."));

        hydra::domain::SessionRecord openCodeSessionRecord;
        openCodeSessionRecord.id = QStringLiteral("opencode-session");
        openCodeSessionRecord.name = QStringLiteral("OpenCode Preview [OpenCode]");
        openCodeSessionRecord.providerKey = QStringLiteral("opencode");
        openCodeSessionRecord.tmuxSessionName = QStringLiteral("hydra-opencode-preview-9c0be8f4");
        openCodeSessionRecord.tmuxPaneId = QStringLiteral("%901");
        openCodeSessionRecord.state = hydra::domain::SessionState::Thinking;
        openCodeSessionRecord.statusDetail = QStringLiteral("Continuing implementation");
        openCodeSessionRecord.statusProvenance = hydra::domain::StatusProvenance::ProviderHint;
        openCodeSessionRecord.updatedAt = QDateTime::currentDateTimeUtc();

        hydra::domain::SessionSignal openCodePaneSignal;
        openCodePaneSignal.sessionId = openCodeSessionRecord.id;
        openCodePaneSignal.sessionName = openCodeSessionRecord.tmuxSessionName;
        openCodePaneSignal.paneId = openCodeSessionRecord.tmuxPaneId;
        openCodePaneSignal.kind = hydra::domain::SessionSignalKind::PaneOutputObserved;
        openCodePaneSignal.provenance = hydra::domain::StatusProvenance::Tmux;
        openCodePaneSignal.occurredAt = QDateTime::currentDateTimeUtc();

        const hydra::domain::SessionStatusUpdate openCodeHeldUpdate =
            statusAggregator.applySignal(
                openCodeSessionRecord,
                openCodePaneSignal,
                openCodeHints,
                QStringLiteral("┃ Ask anything...\n tab agents  ctrl+p commands\n"),
                QString());
        require(openCodeHeldUpdate.state == hydra::domain::SessionState::Thinking,
                QStringLiteral("OpenCode observability still falls back to idle too quickly on prompt-heavy tails."));

        QTextStream(stdout) << "Hydra provider launch overrides smoke passed. repo="
                            << repoPath << " session=" << outcome.session->id << Qt::endl;
        return 0;
    } catch (const std::exception &error) {
        QTextStream(stderr) << error.what() << Qt::endl;
        return 1;
    }
}
