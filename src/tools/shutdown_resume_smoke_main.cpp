#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QRegularExpression>
#include <QThread>
#include <QTemporaryDir>
#include <QTextStream>
#include <QUuid>

#include <stdexcept>

#include "domain/models/launch_safety.hpp"
#include "domain/services/provider_catalog.hpp"
#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/status_aggregator.hpp"
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
#include "infrastructure/providers/hermes_cli_adapter.hpp"
#include "infrastructure/providers/hermes_session_resolver.hpp"
#include "infrastructure/terminal/tmux_terminal_backend.hpp"

namespace {

using hydra::infrastructure::process::ProcessResult;

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

QStringList tmuxClientArguments(const QStringList &arguments)
{
    QStringList effectiveArguments;

    const QString socketPath = qEnvironmentVariable("HYDRA_TMUX_SOCKET_PATH").trimmed();
    if (!socketPath.isEmpty()) {
        effectiveArguments << QStringLiteral("-S") << socketPath;
    } else {
        const QString socketName = qEnvironmentVariable("HYDRA_TMUX_SOCKET_NAME").trimmed();
        if (!socketName.isEmpty()) {
            effectiveArguments << QStringLiteral("-L") << socketName;
        }
    }

    effectiveArguments << arguments;
    return effectiveArguments;
}

void require(bool condition, const QString &message)
{
    if (!condition) {
        throw std::runtime_error(message.toStdString());
    }
}

void requireSessionArtifactsPersisted(const hydra::domain::SessionRecord &session,
                                      const QString &label)
{
    require(!session.transcriptPath.trimmed().isEmpty(),
            QStringLiteral("%1 did not persist a transcript path").arg(label));
    require(!session.auditPath.trimmed().isEmpty(),
            QStringLiteral("%1 did not persist an audit path").arg(label));
    require(QFileInfo::exists(session.transcriptPath),
            QStringLiteral("%1 transcript artifact missing on disk").arg(label));
    require(QFileInfo::exists(session.auditPath),
            QStringLiteral("%1 audit artifact missing on disk").arg(label));
}

void requireStoredExitedSession(const std::optional<hydra::domain::SessionRecord> &storedSession,
                                const QString &label)
{
    require(storedSession.has_value(),
            QStringLiteral("%1 stored session missing").arg(label));
    require(storedSession->state == hydra::domain::SessionState::Exited,
            QStringLiteral("%1 was not marked exited").arg(label));
}

void ensureGitRepo(const QString &path)
{
    QDir().mkpath(path);
    require(runCommand(QStringLiteral("git"), {QStringLiteral("init"), QStringLiteral("-q")}, path).ok,
            QStringLiteral("git init failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("config"),
                        QStringLiteral("user.email"),
                        QStringLiteral("hydra-test@example.com")},
                       path)
                .ok,
            QStringLiteral("git config user.email failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("config"),
                        QStringLiteral("user.name"),
                        QStringLiteral("Hydra Test")},
                       path)
                .ok,
            QStringLiteral("git config user.name failed"));

    QFile readme(QFileInfo(path, QStringLiteral("README.md")).absoluteFilePath());
    require(readme.open(QIODevice::WriteOnly | QIODevice::Text),
            QStringLiteral("failed to write README.md"));
    QTextStream(&readme) << "seed\n";
    readme.close();

    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("add"), QStringLiteral("README.md")},
                       path)
                .ok,
            QStringLiteral("git add failed"));
    require(runCommand(QStringLiteral("git"),
                       {QStringLiteral("commit"),
                        QStringLiteral("-q"),
                        QStringLiteral("-m"),
                        QStringLiteral("seed")},
                       path)
                .ok,
            QStringLiteral("git commit failed"));
}

QString tmuxCapture(const QString &target, const int lines = 120)
{
    const ProcessResult result = runCommand(QStringLiteral("tmux"),
                                            tmuxClientArguments({QStringLiteral("capture-pane"),
                                                                 QStringLiteral("-p"),
                                                                 QStringLiteral("-J"),
                                                                 QStringLiteral("-S"),
                                                                 QStringLiteral("-%1").arg(lines),
                                                                 QStringLiteral("-t"),
                                                                 target}));
    return result.ok ? result.standardOutput : QString();
}

void tmuxSend(const QStringList &arguments)
{
    ProcessResult result = runCommand(QStringLiteral("tmux"), tmuxClientArguments(arguments));
    require(result.ok, QStringLiteral("tmux send failed: %1").arg(result.standardError));
}

void waitForPaneContains(const QString &target, const QString &needle, const int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    QString last;
    while (timer.elapsed() < timeoutMs) {
        last = tmuxCapture(target);
        if (last.contains(needle)) {
            return;
        }
        QThread::msleep(350);
    }

    throw std::runtime_error(
        QStringLiteral("tmux pane never showed %1: %2").arg(needle, last.right(1200)).toStdString());
}

void waitForPaneMatches(const QString &target,
                        const QRegularExpression &pattern,
                        const int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    QString last;
    while (timer.elapsed() < timeoutMs) {
        last = tmuxCapture(target);
        if (pattern.match(last).hasMatch()) {
            return;
        }
        QThread::msleep(350);
    }

    throw std::runtime_error(
        QStringLiteral("tmux pane never matched %1: %2")
            .arg(pattern.pattern(), last.right(1200))
            .toStdString());
}

QRegularExpression bulletWordPattern(const QString &word)
{
    return QRegularExpression(
        QRegularExpression::escape(QString(QChar(0x25CF)))
            + QStringLiteral(R"(\s*%1\b)").arg(QRegularExpression::escape(word)),
        QRegularExpression::CaseInsensitiveOption);
}

QRegularExpression alphaBulletPattern()
{
    return bulletWordPattern(QStringLiteral("alpha"));
}

QRegularExpression betaBulletPattern()
{
    return bulletWordPattern(QStringLiteral("beta"));
}

void tmuxSendLiteral(const QString &target, const QString &text)
{
    tmuxSend({QStringLiteral("send-keys"),
              QStringLiteral("-t"),
              target,
              QStringLiteral("-l"),
              text});
}

void tmuxSendEnter(const QString &target)
{
    tmuxSend({QStringLiteral("send-keys"),
              QStringLiteral("-t"),
              target,
              QStringLiteral("Enter")});
}

void exerciseCodexConversation(const QString &tmuxTarget)
{
    waitForPaneContains(tmuxTarget, QStringLiteral("Press enter to continue"), 30000);
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("OpenAI Codex"), 20000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra codex ping one"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("hydra codex ping one"), 20000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra codex ping two"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("hydra codex ping two"), 20000);
}

bool exerciseGeminiConversation(const QString &tmuxTarget)
{
    QString initial = tmuxCapture(tmuxTarget);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 30000) {
        if (initial.contains(QStringLiteral("Trust folder"), Qt::CaseInsensitive)
            || initial.contains(QStringLiteral("Type your message"), Qt::CaseInsensitive))
        {
            break;
        }
        QThread::msleep(350);
        initial = tmuxCapture(tmuxTarget);
    }

    if (!initial.contains(QStringLiteral("Trust folder"), Qt::CaseInsensitive)
        && !initial.contains(QStringLiteral("Type your message"), Qt::CaseInsensitive))
    {
        qWarning().noquote()
            << QStringLiteral("Hydra shutdown/resume smoke skipped Gemini interactive prompt validation because the isolated Gemini pane never reached trust or live footer state.");
        return false;
    }

    if (initial.contains(QStringLiteral("Trust folder"), Qt::CaseInsensitive)) {
        tmuxSendEnter(tmuxTarget);
        waitForPaneContains(tmuxTarget,
                            QStringLiteral("Gemini CLI is restarting to apply the trust changes"),
                            20000);
        waitForPaneContains(tmuxTarget, QStringLiteral("Type your message"), 30000);
    }

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra gemini ping one"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("hydra gemini ping one"), 20000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra gemini ping two"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("hydra gemini ping two"), 20000);
    return true;
}

void exerciseHermesConversation(const QString &tmuxTarget)
{
    waitForPaneContains(tmuxTarget, QStringLiteral("Welcome to Hermes Agent!"), 30000);
    waitForPaneContains(tmuxTarget, QStringLiteral("❯"), 30000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra hermes ping one"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneContains(tmuxTarget, QStringLiteral("hydra hermes ping one"), 20000);
    waitForPaneMatches(
        tmuxTarget,
        QRegularExpression(QStringLiteral(R"((How can I assist|❯))"),
                           QRegularExpression::CaseInsensitiveOption),
        30000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("hydra hermes ping two"));
    tmuxSendEnter(tmuxTarget);
    waitForPaneMatches(
        tmuxTarget,
        QRegularExpression(QStringLiteral(R"((hydra hermes ping two|Ping successful\.|Target:\s+8\.8\.8\.8))"),
                           QRegularExpression::CaseInsensitiveOption),
        20000);
}

void exerciseClaudeConversation(const QString &tmuxTarget)
{
    QString initial = tmuxCapture(tmuxTarget);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 20000) {
        if (initial.contains(QStringLiteral("trust this folder"), Qt::CaseInsensitive)
            || initial.contains(QStringLiteral("bypass permissions on"), Qt::CaseInsensitive))
        {
            break;
        }
        QThread::msleep(350);
        initial = tmuxCapture(tmuxTarget);
    }

    require(initial.contains(QStringLiteral("trust this folder"), Qt::CaseInsensitive)
                || initial.contains(QStringLiteral("bypass permissions on"), Qt::CaseInsensitive),
            QStringLiteral("Claude prompt never reached trust or live footer state"));

    if (initial.contains(QStringLiteral("trust this folder"), Qt::CaseInsensitive)) {
        tmuxSendEnter(tmuxTarget);
        waitForPaneContains(tmuxTarget, QStringLiteral("bypass permissions on"), 20000);
    }

    tmuxSendLiteral(tmuxTarget, QStringLiteral("Say the word alpha and stop."));
    tmuxSendEnter(tmuxTarget);
    waitForPaneMatches(tmuxTarget, alphaBulletPattern(), 30000);
    waitForPaneContains(tmuxTarget, QStringLiteral("bypass permissions on"), 12000);

    tmuxSendLiteral(tmuxTarget, QStringLiteral("Say the word beta and stop."));
    tmuxSendEnter(tmuxTarget);
    waitForPaneMatches(tmuxTarget, betaBulletPattern(), 30000);
    waitForPaneContains(tmuxTarget, QStringLiteral("bypass permissions on"), 12000);
}

QString normalizeWorkingDirectory(const QString &workingDirectory)
{
    const QFileInfo info(workingDirectory.trimmed());
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QString pendingTokenForProvider(const QString &providerKey)
{
    if (providerKey == QStringLiteral("codex")) {
        return QStringLiteral("codex-pending-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    if (providerKey == QStringLiteral("gemini")) {
        return QStringLiteral("gemini-pending-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

hydra::domain::SessionRecord waitForResolvedStoredSession(
    hydra::domain::SessionSupervisor &supervisor,
    hydra::domain::ports::SessionStore &sessionStore,
    const QString &sessionId,
    const QString &expectedProviderSessionId,
    const int maxRefreshes = 6)
{
    for (int attempt = 0; attempt < maxRefreshes; ++attempt) {
        supervisor.refreshSessionStates();
        const auto stored = sessionStore.sessionById(sessionId);
        if (!stored.has_value()) {
            throw std::runtime_error(
                QStringLiteral("stored session disappeared while resolving %1")
                    .arg(sessionId)
                    .toStdString());
        }
        if (stored->providerSessionId == expectedProviderSessionId) {
            return stored.value();
        }
    }

    const auto stored = sessionStore.sessionById(sessionId);
    throw std::runtime_error(
        QStringLiteral("session %1 never resolved provider token %2 (last token %3)")
            .arg(sessionId,
                 expectedProviderSessionId,
                 stored.has_value() ? stored->providerSessionId : QStringLiteral("<missing>"))
            .toStdString());
}

class FakeResumableProviderAdapter final : public hydra::domain::ports::ProviderAdapter {
public:
    explicit FakeResumableProviderAdapter(QString providerKey)
        : m_providerKey(std::move(providerKey))
    {
    }

    QString providerKey() const override { return m_providerKey; }
    QString displayName() const override { return QStringLiteral("Fake %1").arg(m_providerKey); }
    hydra::domain::ports::ProviderProbe probe() const override
    {
        return {.providerKey = m_providerKey,
                .displayName = displayName(),
                .available = true,
                .executablePath = QStringLiteral("/bin/sh"),
                .versionString = QStringLiteral("smoke"),
                .statusMessage = QStringLiteral("ok")};
    }
    bool supportsResume() const override { return true; }
    QString initialResumeToken() const override { return pendingTokenForProvider(m_providerKey); }
    hydra::domain::ports::ProviderLaunchPlan buildLaunchPlan(
        hydra::domain::LaunchSafety,
        hydra::domain::ports::ProviderInvocationOptions options) const override
    {
        Q_UNUSED(options);
        return {.executable = QStringLiteral("bash"),
                .arguments = {QStringLiteral("-lc"),
                              QStringLiteral("printf 'boot\\n'; sleep 120")},
                .environment = {}};
    }
    hydra::domain::ports::ProviderLaunchPlan buildResumePlan(
        const hydra::domain::LaunchSafety safety,
        hydra::domain::ports::ProviderInvocationOptions options) const override
    {
        return {.executable = QStringLiteral("bash"),
                .arguments = {QStringLiteral("-lc"),
                              QStringLiteral("printf 'resumed %1 [%2]\\n'; sleep 120")
                                  .arg(options.resumeToken, hydra::domain::launchSafetyKey(safety))},
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

private:
    QString m_providerKey;
};

class FakeNonResumableProviderAdapter final : public hydra::domain::ports::ProviderAdapter {
public:
    QString providerKey() const override { return QStringLiteral("host-shell"); }
    QString displayName() const override { return QStringLiteral("Host Shell"); }

    hydra::domain::ports::ProviderProbe probe() const override
    {
        return {.providerKey = providerKey(),
                .displayName = displayName(),
                .available = true,
                .executablePath = QStringLiteral("/bin/bash"),
                .versionString = QStringLiteral("smoke"),
                .statusMessage = QStringLiteral("ok")};
    }

    hydra::domain::ports::ProviderLaunchPlan buildLaunchPlan(
        hydra::domain::LaunchSafety,
        hydra::domain::ports::ProviderInvocationOptions options) const override
    {
        Q_UNUSED(options);
        return {.executable = QStringLiteral("/bin/bash"),
                .arguments = {QStringLiteral("--noprofile"), QStringLiteral("--norc")},
                .environment = {}};
    }

    QString launchSafetySummary(hydra::domain::LaunchSafety) const override
    {
        return QStringLiteral("Host shell smoke adapter.");
    }

    hydra::domain::ports::ProviderObservabilityHints observabilityHints() const override
    {
        return {
            .promptPatterns = {QStringLiteral(R"((?:^|\s)[$#]\s?$)"),
                               QStringLiteral(R"([\w.@:/~+-]+[$#]\s?$)")},
            .promptTitlePatterns = {},
            .approvalPatterns = {},
            .waitingInputPatterns = {},
            .activePatterns = {},
            .approvalTitlePatterns = {},
            .waitingTitlePatterns = {},
            .activeTitlePatterns = {},
            .ignoredTailPatterns = {},
            .activeState = hydra::domain::SessionState::RunningTool,
        };
    }
};

class DelayedSessionResolver final : public hydra::domain::ports::ProviderSessionResolver {
public:
    DelayedSessionResolver(QString providerKey,
                           QString workingDirectory,
                           QString resolvedSessionId,
                           const int resolveAfterAttempts)
        : m_providerKey(std::move(providerKey)),
          m_workingDirectory(normalizeWorkingDirectory(workingDirectory)),
          m_resolvedSessionId(std::move(resolvedSessionId)),
          m_resolveAfterAttempts(resolveAfterAttempts)
    {
    }

    QString providerKey() const override { return m_providerKey; }

    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override
    {
        if (normalizeWorkingDirectory(workingDirectory) != m_workingDirectory) {
            return {};
        }
        return {};
    }

    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds) const override
    {
        if (normalizeWorkingDirectory(workingDirectory) != m_workingDirectory) {
            return {};
        }
        ++m_attemptCount;
        if (m_attemptCount < m_resolveAfterAttempts) {
            return {};
        }
        if (knownSessionIds.contains(m_resolvedSessionId)
            || excludedSessionIds.contains(m_resolvedSessionId))
        {
            return {};
        }
        return m_resolvedSessionId;
    }

    bool sessionExists(const QString &sessionId, const QString &workingDirectory) const override
    {
        return sessionId.trimmed() == m_resolvedSessionId
               && (workingDirectory.trimmed().isEmpty()
                   || normalizeWorkingDirectory(workingDirectory) == m_workingDirectory);
    }

private:
    QString m_providerKey;
    QString m_workingDirectory;
    QString m_resolvedSessionId;
    int m_resolveAfterAttempts = 1;
    mutable int m_attemptCount = 0;
};

class StaticSessionResolver final : public hydra::domain::ports::ProviderSessionResolver {
public:
    StaticSessionResolver(QString providerKey,
                          QString workingDirectory,
                          QSet<QString> knownSessionIds)
        : m_providerKey(std::move(providerKey)),
          m_workingDirectory(normalizeWorkingDirectory(workingDirectory)),
          m_knownSessionIds(std::move(knownSessionIds))
    {
    }

    QString providerKey() const override { return m_providerKey; }

    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override
    {
        if (normalizeWorkingDirectory(workingDirectory) != m_workingDirectory) {
            return {};
        }
        return m_knownSessionIds;
    }

    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds) const override
    {
        if (normalizeWorkingDirectory(workingDirectory) != m_workingDirectory) {
            return {};
        }

        for (const QString &candidate : m_knownSessionIds) {
            if (!knownSessionIds.contains(candidate) && !excludedSessionIds.contains(candidate)) {
                return candidate;
            }
        }

        return {};
    }

    bool sessionExists(const QString &sessionId, const QString &workingDirectory) const override
    {
        return m_knownSessionIds.contains(sessionId.trimmed())
               && (workingDirectory.trimmed().isEmpty()
                   || normalizeWorkingDirectory(workingDirectory) == m_workingDirectory);
    }

private:
    QString m_providerKey;
    QString m_workingDirectory;
    QSet<QString> m_knownSessionIds;
};

}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tempRoot;
    require(tempRoot.isValid(), QStringLiteral("failed to create temporary root"));
    const QByteArray tmuxSocketName =
        QStringLiteral("hydra-shutdown-smoke-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))
            .toUtf8();
    qputenv("HYDRA_TMUX_SOCKET_NAME", tmuxSocketName);
    qputenv("TMUX_TMPDIR", tempRoot.path().toUtf8());

    const QString repoPath = QFileInfo(tempRoot.path(), QStringLiteral("repo")).absoluteFilePath();
    const QString dbPath =
        QFileInfo(QFileInfo(tempRoot.path(), QStringLiteral("data")).absoluteFilePath()
                      + QStringLiteral("/Hydra/Hydra V2"),
                  QStringLiteral("hydra.sqlite3"))
            .absoluteFilePath();

    ensureGitRepo(repoPath);

    hydra::infrastructure::DatabaseManager databaseManager(dbPath);
    hydra::infrastructure::SqliteRepoStore repoStore(databaseManager.database());
    hydra::infrastructure::SqliteSessionStore sessionStore(databaseManager.database());
    hydra::domain::RepoRegistry repoRegistry(repoStore);
    hydra::infrastructure::TmuxAdapter tmuxAdapter;
    FakeNonResumableProviderAdapter hostShellAdapter;
    hydra::infrastructure::CodexCliAdapter codexAdapter;
    hydra::infrastructure::CodexSessionResolver codexResolver;
    hydra::infrastructure::GeminiCliAdapter geminiAdapter;
    hydra::infrastructure::GeminiSessionResolver geminiResolver;
    hydra::infrastructure::ClaudeCodeAdapter claudeAdapter;
    hydra::infrastructure::HermesCliAdapter hermesAdapter;
    hydra::infrastructure::HermesSessionResolver hermesResolver;
    hydra::infrastructure::TmuxTerminalBackend terminalBackend;
    hydra::domain::ProviderCatalog providerCatalog(
        QVector<hydra::domain::ports::ProviderAdapter *>{&hostShellAdapter,
                                                         &codexAdapter,
                                                         &geminiAdapter,
                                                         &claudeAdapter,
                                                         &hermesAdapter});
    hydra::domain::StatusAggregator statusAggregator;
    hydra::domain::SessionSupervisor supervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&codexResolver,
                                                                 &geminiResolver,
                                                                 &hermesResolver},
        providerCatalog,
        statusAggregator,
        QStringLiteral("owner-primary"));
    hydra::domain::SessionSupervisor secondarySupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&codexResolver,
                                                                 &geminiResolver,
                                                                 &hermesResolver},
        providerCatalog,
        statusAggregator,
        QStringLiteral("owner-secondary"));

    const auto repoOutcome =
        repoRegistry.ensureRepository(repoPath,
                                      QStringLiteral("Shutdown Resume Repo"),
                                      QStringLiteral(""),
                                      QStringLiteral("#B38A59"));
    require(repoOutcome.ok, QStringLiteral("failed to register repo"));
    const QString repoId = repoOutcome.repository.id;

    FakeResumableProviderAdapter fakeCodexAdapter(QStringLiteral("codex"));
    FakeResumableProviderAdapter fakeGeminiAdapter(QStringLiteral("gemini"));
    DelayedSessionResolver delayedCodexResolver(
        QStringLiteral("codex"),
        repoPath,
        QStringLiteral("codex-hydra-owned-smoke"),
        4);
    DelayedSessionResolver delayedGeminiResolver(
        QStringLiteral("gemini"),
        repoPath,
        QStringLiteral("gemini-hydra-owned-smoke"),
        5);
    hydra::domain::ProviderCatalog fakeProviderCatalog(
        QVector<hydra::domain::ports::ProviderAdapter *>{&fakeCodexAdapter, &fakeGeminiAdapter});
    hydra::domain::SessionSupervisor fakeLifecycleSupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&delayedCodexResolver,
                                                                 &delayedGeminiResolver},
        fakeProviderCatalog,
        statusAggregator,
        QStringLiteral("owner-fake"));

    const auto ownerScopeLaunch = supervisor.launchSession(repoId,
                                                           hostShellAdapter.providerKey(),
                                                           hydra::domain::LaunchSafety::WorkspaceSafe,
                                                           repoPath,
                                                           QStringLiteral("Owner Scope"));
    require(ownerScopeLaunch.ok && ownerScopeLaunch.session.has_value(),
            QStringLiteral("owner scope launch failed: %1").arg(ownerScopeLaunch.message));
    require(supervisor.ownedLiveSessionCount() >= 1,
            QStringLiteral("primary owner did not see its live session"));
    require(secondarySupervisor.ownedLiveSessionCount() == 0,
            QStringLiteral("secondary owner incorrectly saw live sessions"));
    const hydra::domain::SessionShutdownOutcome secondaryShutdownOutcome =
        secondarySupervisor.shutdownOwnedSessionsForAppExit();
    require(secondaryShutdownOutcome.ok,
            QStringLiteral("secondary shutdown unexpectedly failed: %1")
                .arg(secondaryShutdownOutcome.errors.join(QStringLiteral(" | "))));
    require(secondaryShutdownOutcome.terminatedCount == 0,
            QStringLiteral("secondary shutdown terminated sessions it did not own"));
    require(tmuxAdapter.hasLiveSession(ownerScopeLaunch.session->tmuxSessionName),
            QStringLiteral("secondary shutdown killed primary-owned tmux session"));
    const hydra::domain::SessionShutdownOutcome secondaryStartupRecoveryOutcome =
        secondarySupervisor.cleanupForeignOwnedSessionsForStartup();
    require(secondaryStartupRecoveryOutcome.ok,
            QStringLiteral("secondary startup recovery failed: %1")
                .arg(secondaryStartupRecoveryOutcome.errors.join(QStringLiteral(" | "))));
    require(secondaryStartupRecoveryOutcome.terminatedCount == 1,
            QStringLiteral("secondary startup recovery did not terminate the stale live session"));
    require(!tmuxAdapter.hasLiveSession(ownerScopeLaunch.session->tmuxSessionName),
            QStringLiteral("secondary startup recovery left the stale tmux session live"));
    const auto ownerScopeRecovered = sessionStore.sessionById(ownerScopeLaunch.session->id);
    requireStoredExitedSession(ownerScopeRecovered,
                               QStringLiteral("owner scope recovered session"));
    require(ownerScopeRecovered->statusDetail.contains(QStringLiteral("earlier app instance"),
                                                       Qt::CaseInsensitive),
            QStringLiteral("secondary startup recovery did not record stale-instance cleanup"));

    const auto codexInteractionLaunch = supervisor.launchSession(repoId,
                                                                 QStringLiteral("codex"),
                                                                 hydra::domain::LaunchSafety::WorkspaceSafe,
                                                                 repoPath,
                                                                 QStringLiteral("Runtime Codex"));
    require(codexInteractionLaunch.ok && codexInteractionLaunch.session.has_value(),
            QStringLiteral("codex interaction launch failed: %1").arg(codexInteractionLaunch.message));
    exerciseCodexConversation(codexInteractionLaunch.session->tmuxSessionName);
    const auto codexInteractionTerminate =
        supervisor.terminateSession(codexInteractionLaunch.session->id);
    require(codexInteractionTerminate.ok && codexInteractionTerminate.session.has_value(),
            QStringLiteral("codex interaction terminate failed: %1")
                .arg(codexInteractionTerminate.message));
    requireSessionArtifactsPersisted(codexInteractionTerminate.session.value(),
                                     QStringLiteral("codex interaction"));

    const auto geminiInteractionLaunch = supervisor.launchSession(repoId,
                                                                  QStringLiteral("gemini"),
                                                                  hydra::domain::LaunchSafety::WorkspaceSafe,
                                                                  repoPath,
                                                                  QStringLiteral("Runtime Gemini"));
    require(geminiInteractionLaunch.ok && geminiInteractionLaunch.session.has_value(),
            QStringLiteral("gemini interaction launch failed: %1")
                .arg(geminiInteractionLaunch.message));
    const bool geminiConversationExercised =
        exerciseGeminiConversation(geminiInteractionLaunch.session->tmuxSessionName);
    const auto geminiInteractionTerminate =
        supervisor.terminateSession(geminiInteractionLaunch.session->id);
    require(geminiInteractionTerminate.ok && geminiInteractionTerminate.session.has_value(),
            QStringLiteral("gemini interaction terminate failed: %1")
                .arg(geminiInteractionTerminate.message));
    if (geminiConversationExercised) {
        requireSessionArtifactsPersisted(geminiInteractionTerminate.session.value(),
                                         QStringLiteral("gemini interaction"));
    }

    const auto hermesInteractionLaunch = supervisor.launchSession(repoId,
                                                                  QStringLiteral("hermes"),
                                                                  hydra::domain::LaunchSafety::WorkspaceSafe,
                                                                  repoPath,
                                                                  QStringLiteral("Runtime Hermes"));
    require(hermesInteractionLaunch.ok && hermesInteractionLaunch.session.has_value(),
            QStringLiteral("hermes interaction launch failed: %1")
                .arg(hermesInteractionLaunch.message));
    exerciseHermesConversation(hermesInteractionLaunch.session->tmuxSessionName);
    const auto hermesInteractionTerminate =
        supervisor.terminateSession(hermesInteractionLaunch.session->id);
    require(hermesInteractionTerminate.ok && hermesInteractionTerminate.session.has_value(),
            QStringLiteral("hermes interaction terminate failed: %1")
                .arg(hermesInteractionTerminate.message));
    requireSessionArtifactsPersisted(hermesInteractionTerminate.session.value(),
                                     QStringLiteral("hermes interaction"));

    const auto fakeCodexLaunch = fakeLifecycleSupervisor.launchSession(repoId,
                                                                       QStringLiteral("codex"),
                                                                       hydra::domain::LaunchSafety::Bypass,
                                                                       repoPath,
                                                                       QStringLiteral("Owned End"));
    require(fakeCodexLaunch.ok && fakeCodexLaunch.session.has_value(),
            QStringLiteral("fake codex launch failed: %1").arg(fakeCodexLaunch.message));
    const auto fakeCodexTerminate = fakeLifecycleSupervisor.terminateSession(fakeCodexLaunch.session->id);
    require(fakeCodexTerminate.ok && fakeCodexTerminate.session.has_value(),
            QStringLiteral("fake codex terminate failed: %1").arg(fakeCodexTerminate.message));
    require(fakeCodexTerminate.session->providerSessionId.startsWith(
                QStringLiteral("codex-pending-")),
            QStringLiteral("fake codex terminate did not preserve the pending resume token"));
    requireSessionArtifactsPersisted(fakeCodexTerminate.session.value(),
                                     QStringLiteral("fake codex terminate"));
    const hydra::domain::SessionRecord resolvedFakeCodex = waitForResolvedStoredSession(
        fakeLifecycleSupervisor,
        sessionStore,
        fakeCodexLaunch.session->id,
        QStringLiteral("codex-hydra-owned-smoke"));
    const auto fakeCodexResume = fakeLifecycleSupervisor.resumeSession(
        resolvedFakeCodex.id,
        hydra::domain::LaunchSafety::WorkspaceSafe);
    require(fakeCodexResume.ok && fakeCodexResume.session.has_value(),
            QStringLiteral("fake codex resume failed after terminate: %1").arg(fakeCodexResume.message));
    require(tmuxAdapter.hasLiveSession(fakeCodexResume.session->tmuxSessionName),
            QStringLiteral("fake codex resumed tmux session is not live"));
    require(fakeCodexResume.session->launchSafetyKey == QStringLiteral("workspace-safe"),
            QStringLiteral("fake codex resume did not adopt current resume safety"));
    waitForPaneContains(fakeCodexResume.session->tmuxSessionName,
                        QStringLiteral("resumed codex-hydra-owned-smoke [workspace-safe]"),
                        12000);
    fakeLifecycleSupervisor.terminateSession(fakeCodexResume.session->id);

    const auto fakeGeminiLaunch = fakeLifecycleSupervisor.launchSession(repoId,
                                                                        QStringLiteral("gemini"),
                                                                        hydra::domain::LaunchSafety::Bypass,
                                                                        repoPath,
                                                                        QStringLiteral("Owned Shutdown"));
    require(fakeGeminiLaunch.ok && fakeGeminiLaunch.session.has_value(),
            QStringLiteral("fake gemini launch failed: %1").arg(fakeGeminiLaunch.message));
    const hydra::domain::SessionShutdownOutcome fakeShutdownOutcome =
        fakeLifecycleSupervisor.shutdownOwnedSessionsForAppExit();
    require(fakeShutdownOutcome.ok,
            QStringLiteral("fake shutdown flow failed: %1")
                .arg(fakeShutdownOutcome.errors.join(QStringLiteral(" | "))));
    const auto fakeGeminiStored = sessionStore.sessionById(fakeGeminiLaunch.session->id);
    require(fakeGeminiStored.has_value(),
            QStringLiteral("fake gemini stored session missing after shutdown"));
    require(fakeGeminiStored->providerSessionId.startsWith(
                QStringLiteral("gemini-pending-")),
            QStringLiteral("fake gemini shutdown did not preserve the pending resume token"));
    requireSessionArtifactsPersisted(fakeGeminiStored.value(),
                                     QStringLiteral("fake gemini shutdown"));
    const hydra::domain::SessionRecord resolvedFakeGemini = waitForResolvedStoredSession(
        fakeLifecycleSupervisor,
        sessionStore,
        fakeGeminiLaunch.session->id,
        QStringLiteral("gemini-hydra-owned-smoke"),
        7);
    const auto fakeGeminiResume = fakeLifecycleSupervisor.resumeSession(
        resolvedFakeGemini.id,
        hydra::domain::LaunchSafety::WorkspaceSafe);
    require(fakeGeminiResume.ok && fakeGeminiResume.session.has_value(),
            QStringLiteral("fake gemini resume failed after shutdown: %1")
                .arg(fakeGeminiResume.message));
    require(tmuxAdapter.hasLiveSession(fakeGeminiResume.session->tmuxSessionName),
            QStringLiteral("fake gemini resumed tmux session is not live"));
    require(fakeGeminiResume.session->launchSafetyKey == QStringLiteral("workspace-safe"),
            QStringLiteral("fake gemini resume did not adopt current resume safety"));
    waitForPaneContains(fakeGeminiResume.session->tmuxSessionName,
                        QStringLiteral("resumed gemini-hydra-owned-smoke [workspace-safe]"),
                        12000);
    fakeLifecycleSupervisor.terminateSession(fakeGeminiResume.session->id);

    const auto restartRecoveryLaunch = fakeLifecycleSupervisor.launchSession(
        repoId,
        QStringLiteral("codex"),
        hydra::domain::LaunchSafety::Bypass,
        repoPath,
        QStringLiteral("Restart Recovery"));
    require(restartRecoveryLaunch.ok && restartRecoveryLaunch.session.has_value(),
            QStringLiteral("restart recovery launch failed: %1").arg(restartRecoveryLaunch.message));

    StaticSessionResolver recoveredCodexResolver(
        QStringLiteral("codex"),
        repoPath,
        QSet<QString>{QStringLiteral("codex-hydra-restarted-smoke")});
    hydra::domain::SessionSupervisor restartedRecoverySupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&recoveredCodexResolver,
                                                                 &delayedGeminiResolver},
        fakeProviderCatalog,
        statusAggregator,
        QStringLiteral("owner-restarted-recovery"));
    const auto restartRecoveryTerminate =
        restartedRecoverySupervisor.terminateSession(restartRecoveryLaunch.session->id);
    require(restartRecoveryTerminate.ok && restartRecoveryTerminate.session.has_value(),
            QStringLiteral("restart recovery terminate failed: %1")
                .arg(restartRecoveryTerminate.message));
    require(restartRecoveryTerminate.session->providerSessionId
                == QStringLiteral("codex-hydra-restarted-smoke"),
            QStringLiteral("restart recovery did not adopt the recovered Codex session id"));

    const auto inactivePendingLaunch = fakeLifecycleSupervisor.launchSession(
        repoId,
        QStringLiteral("gemini"),
        hydra::domain::LaunchSafety::Bypass,
        repoPath,
        QStringLiteral("Inactive Pending"));
    require(inactivePendingLaunch.ok && inactivePendingLaunch.session.has_value(),
            QStringLiteral("inactive pending launch failed: %1").arg(inactivePendingLaunch.message));
    const auto inactiveKill = tmuxAdapter.terminateSession(inactivePendingLaunch.session->tmuxSessionName);
    require(inactiveKill.ok,
            QStringLiteral("failed to pre-kill inactive pending tmux session: %1")
                .arg(inactiveKill.errorMessage));

    StaticSessionResolver emptyGeminiResolver(QStringLiteral("gemini"), repoPath, {});
    hydra::domain::SessionSupervisor inactivePendingSupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&delayedCodexResolver,
                                                                 &emptyGeminiResolver},
        fakeProviderCatalog,
        statusAggregator,
        QStringLiteral("owner-restarted-inactive"));
    const auto inactivePendingTerminate =
        inactivePendingSupervisor.terminateSession(inactivePendingLaunch.session->id);
    require(inactivePendingTerminate.ok && inactivePendingTerminate.session.has_value(),
            QStringLiteral("inactive pending terminate failed after tmux died: %1")
                .arg(inactivePendingTerminate.message));
    require(inactivePendingTerminate.session->state == hydra::domain::SessionState::Exited,
            QStringLiteral("inactive pending session was not marked exited"));

    const auto foreignOwnedPendingLaunch = fakeLifecycleSupervisor.launchSession(
        repoId,
        QStringLiteral("codex"),
        hydra::domain::LaunchSafety::Bypass,
        repoPath,
        QStringLiteral("Foreign Pending"));
    require(foreignOwnedPendingLaunch.ok && foreignOwnedPendingLaunch.session.has_value(),
            QStringLiteral("foreign-owned pending launch failed: %1")
                .arg(foreignOwnedPendingLaunch.message));

    StaticSessionResolver emptyCodexResolver(QStringLiteral("codex"), repoPath, {});
    hydra::domain::SessionSupervisor foreignOwnedPendingSupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&emptyCodexResolver,
                                                                 &delayedGeminiResolver},
        fakeProviderCatalog,
        statusAggregator,
        QStringLiteral("owner-restarted-foreign-pending"));
    const auto foreignOwnedPendingTerminate =
        foreignOwnedPendingSupervisor.terminateSession(foreignOwnedPendingLaunch.session->id);
    require(foreignOwnedPendingTerminate.ok && foreignOwnedPendingTerminate.session.has_value(),
            QStringLiteral("foreign-owned pending terminate failed: %1")
                .arg(foreignOwnedPendingTerminate.message));
    require(foreignOwnedPendingTerminate.session->state == hydra::domain::SessionState::Exited,
            QStringLiteral("foreign-owned pending session was not marked exited"));
    require(!tmuxAdapter.hasLiveSession(foreignOwnedPendingLaunch.session->tmuxSessionName),
            QStringLiteral("foreign-owned pending tmux session is still live after termination"));
    require(foreignOwnedPendingTerminate.session->statusDetail.contains(
                QStringLiteral("earlier app instance"),
                Qt::CaseInsensitive),
            QStringLiteral("foreign-owned pending terminate did not record the stale-instance cleanup detail"));

    hydra::domain::SessionSupervisor unresolvedShutdownSupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&emptyCodexResolver,
                                                                 &delayedGeminiResolver},
        fakeProviderCatalog,
        statusAggregator,
        QStringLiteral("owner-unresolved-shutdown"));
    const auto unresolvedShutdownLaunch = unresolvedShutdownSupervisor.launchSession(
        repoId,
        QStringLiteral("codex"),
        hydra::domain::LaunchSafety::Bypass,
        repoPath,
        QStringLiteral("Pending Shutdown"));
    require(unresolvedShutdownLaunch.ok && unresolvedShutdownLaunch.session.has_value(),
            QStringLiteral("pending shutdown launch failed: %1")
                .arg(unresolvedShutdownLaunch.message));
    const hydra::domain::SessionShutdownOutcome unresolvedShutdownOutcome =
        unresolvedShutdownSupervisor.shutdownOwnedSessionsForAppExit();
    require(unresolvedShutdownOutcome.ok,
            QStringLiteral("pending shutdown flow failed: %1")
                .arg(unresolvedShutdownOutcome.errors.join(QStringLiteral(" | "))));
    require(!tmuxAdapter.hasLiveSession(unresolvedShutdownLaunch.session->tmuxSessionName),
            QStringLiteral("pending shutdown tmux session is still live after app shutdown"));
    const auto unresolvedShutdownStored = sessionStore.sessionById(unresolvedShutdownLaunch.session->id);
    requireStoredExitedSession(unresolvedShutdownStored,
                               QStringLiteral("pending shutdown session"));
    require(unresolvedShutdownStored->providerSessionId.startsWith(
                QStringLiteral("codex-pending-")),
            QStringLiteral("pending shutdown session did not preserve the unresolved resume token"));
    requireSessionArtifactsPersisted(unresolvedShutdownStored.value(),
                                     QStringLiteral("pending shutdown"));
    require(unresolvedShutdownStored->statusDetail.contains(
                QStringLiteral("Resume will unlock automatically"),
                Qt::CaseInsensitive),
            QStringLiteral("pending shutdown session did not record the deferred-resume detail"));

    const auto hostShellLaunch = supervisor.launchSession(repoId,
                                                          hostShellAdapter.providerKey(),
                                                          hydra::domain::LaunchSafety::WorkspaceSafe,
                                                          repoPath,
                                                          QStringLiteral("Shutdown Probe"));
    require(hostShellLaunch.ok && hostShellLaunch.session.has_value(),
            QStringLiteral("host-shell launch failed: %1").arg(hostShellLaunch.message));

    const auto staleOwnedLaunch = supervisor.launchSession(repoId,
                                                           hostShellAdapter.providerKey(),
                                                           hydra::domain::LaunchSafety::WorkspaceSafe,
                                                           repoPath,
                                                           QStringLiteral("Stale Exit Probe"));
    require(staleOwnedLaunch.ok && staleOwnedLaunch.session.has_value(),
            QStringLiteral("stale-exit launch failed: %1").arg(staleOwnedLaunch.message));
    auto staleOwnedStored = sessionStore.sessionById(staleOwnedLaunch.session->id);
    require(staleOwnedStored.has_value(),
            QStringLiteral("stale-exit stored session missing before shutdown"));
    staleOwnedStored->state = hydra::domain::SessionState::Exited;
    staleOwnedStored->updatedAt = QDateTime::currentDateTimeUtc();
    QString stalePersistError;
    require(sessionStore.upsertSession(staleOwnedStored.value(), &stalePersistError),
            QStringLiteral("failed to persist stale-exit regression setup: %1")
                .arg(stalePersistError));
    require(supervisor.ownedLiveSessionCount() >= 2,
            QStringLiteral("owned live session count skipped the stale-exit tmux session"));

    const auto claudeLaunch = supervisor.launchSession(repoId,
                                                       QStringLiteral("claude"),
                                                       hydra::domain::LaunchSafety::Bypass,
                                                       repoPath,
                                                       QStringLiteral("Shutdown Probe"));
    require(claudeLaunch.ok && claudeLaunch.session.has_value(),
            QStringLiteral("claude launch failed: %1").arg(claudeLaunch.message));
    require(!claudeLaunch.session->providerSessionId.trimmed().isEmpty(),
            QStringLiteral("claude launch did not persist a resume token"));

    exerciseClaudeConversation(claudeLaunch.session->tmuxSessionName);

    const hydra::domain::SessionShutdownOutcome shutdownOutcome =
        supervisor.shutdownOwnedSessionsForAppExit();
    require(shutdownOutcome.ok,
            QStringLiteral("shutdown flow failed: %1").arg(shutdownOutcome.errors.join(QStringLiteral(" | "))));

    require(!tmuxAdapter.hasLiveSession(hostShellLaunch.session->tmuxSessionName),
            QStringLiteral("host-shell tmux session still live after app shutdown"));
    require(!tmuxAdapter.hasLiveSession(staleOwnedLaunch.session->tmuxSessionName),
            QStringLiteral("stale-exit tmux session still live after app shutdown"));
    require(!tmuxAdapter.hasLiveSession(claudeLaunch.session->tmuxSessionName),
            QStringLiteral("claude tmux session still live after app shutdown"));

    const auto hostShellStored = sessionStore.sessionById(hostShellLaunch.session->id);
    staleOwnedStored = sessionStore.sessionById(staleOwnedLaunch.session->id);
    const auto claudeStored = sessionStore.sessionById(claudeLaunch.session->id);
    requireStoredExitedSession(hostShellStored, QStringLiteral("host-shell session"));
    requireStoredExitedSession(staleOwnedStored, QStringLiteral("stale-exit session"));
    requireStoredExitedSession(claudeStored, QStringLiteral("claude session"));
    requireSessionArtifactsPersisted(hostShellStored.value(),
                                     QStringLiteral("host-shell shutdown"));
    requireSessionArtifactsPersisted(staleOwnedStored.value(),
                                     QStringLiteral("stale-exit shutdown"));
    requireSessionArtifactsPersisted(claudeStored.value(),
                                     QStringLiteral("claude shutdown"));
    require(!claudeStored->providerSessionId.trimmed().isEmpty(),
            QStringLiteral("claude resume token was not preserved"));

    const auto resumeOutcome =
        supervisor.resumeSession(claudeLaunch.session->id, hydra::domain::LaunchSafety::WorkspaceSafe);
    require(resumeOutcome.ok && resumeOutcome.session.has_value(),
            QStringLiteral("claude resume failed: %1").arg(resumeOutcome.message));
    require(tmuxAdapter.hasLiveSession(resumeOutcome.session->tmuxSessionName),
            QStringLiteral("resumed claude tmux session is not live"));
    waitForPaneMatches(resumeOutcome.session->tmuxSessionName, alphaBulletPattern(), 20000);

    supervisor.terminateSession(resumeOutcome.session->id);

    const QString geminiTmpRoot = QFileInfo(tempRoot.path(), QStringLiteral("gemini-tmp")).absoluteFilePath();
    const QString geminiProjectDir =
        QFileInfo(geminiTmpRoot, QStringLiteral("project-a")).absoluteFilePath();
    const QString geminiChatsDir =
        QFileInfo(geminiProjectDir, QStringLiteral("chats")).absoluteFilePath();
    QDir().mkpath(geminiChatsDir);
    QFile projectRootFile(QFileInfo(geminiProjectDir, QStringLiteral(".project_root")).absoluteFilePath());
    require(projectRootFile.open(QIODevice::WriteOnly | QIODevice::Text),
            QStringLiteral("failed to write Gemini .project_root"));
    QTextStream(&projectRootFile) << repoPath;
    projectRootFile.close();

    const QString externalGeminiSessionId = QStringLiteral("external-gemini-session");
    QFile externalGeminiSession(
        QFileInfo(geminiChatsDir, QStringLiteral("session-external.json")).absoluteFilePath());
    require(externalGeminiSession.open(QIODevice::WriteOnly | QIODevice::Text),
            QStringLiteral("failed to write Gemini external session"));
    const QJsonObject geminiObject{
        {QStringLiteral("sessionId"), externalGeminiSessionId},
        {QStringLiteral("startTime"), QStringLiteral("2026-03-09T00:00:00Z")},
        {QStringLiteral("lastUpdated"), QStringLiteral("2026-03-09T00:00:00Z")},
    };
    externalGeminiSession.write(QJsonDocument(geminiObject).toJson(QJsonDocument::Compact));
    externalGeminiSession.close();

    hydra::infrastructure::GeminiSessionResolver isolatedGeminiResolver{QDir(geminiTmpRoot)};
    hydra::domain::SessionSupervisor isolatedSupervisor(
        repoStore,
        sessionStore,
        tmuxAdapter,
        terminalBackend,
        QVector<hydra::domain::ports::ProviderSessionResolver *>{&codexResolver, &isolatedGeminiResolver},
        providerCatalog,
        statusAggregator,
        QStringLiteral("owner-isolated"));

    hydra::domain::SessionRecord unresolvedGemini;
    unresolvedGemini.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    unresolvedGemini.repoId = repoId;
    unresolvedGemini.name = QStringLiteral("Shutdown Probe [Gemini]");
    unresolvedGemini.providerKey = QStringLiteral("gemini");
    unresolvedGemini.providerSessionId = QStringLiteral("gemini-pending-smoke");
    unresolvedGemini.launchSafetyKey = QStringLiteral("bypass");
    unresolvedGemini.state = hydra::domain::SessionState::Exited;
    unresolvedGemini.workingDirectory = repoPath;
    unresolvedGemini.statusDetail = QStringLiteral("Hydra closed this provider session when the app exited. Use Resume to reopen it.");
    unresolvedGemini.createdAt = QDateTime::currentDateTimeUtc();
    unresolvedGemini.updatedAt = unresolvedGemini.createdAt;
    QString geminiPersistError;
    require(sessionStore.upsertSession(unresolvedGemini, &geminiPersistError),
            QStringLiteral("failed to persist unresolved Gemini session: %1").arg(geminiPersistError));

    const auto geminiResumeOutcome = isolatedSupervisor.resumeSession(
        unresolvedGemini.id,
        hydra::domain::LaunchSafety::WorkspaceSafe);
    require(!geminiResumeOutcome.ok,
            QStringLiteral("Gemini resume unexpectedly succeeded from external session metadata"));
    require(geminiResumeOutcome.message.contains(QStringLiteral("resume metadata is not available"),
                                                 Qt::CaseInsensitive),
            QStringLiteral("Unexpected Gemini resume failure message: %1").arg(geminiResumeOutcome.message));

    QTextStream(stdout) << "Hydra shutdown/resume smoke passed. repo=" << repoPath << Qt::endl;
    return 0;
}
