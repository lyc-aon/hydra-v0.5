#include "infrastructure/providers/claude_code_adapter.hpp"

#include <QDir>
#include <QFile>
#include <QUuid>

#include "domain/models/provider_identity.hpp"
#include "infrastructure/providers/provider_support.hpp"

namespace hydra::infrastructure {

QString ClaudeCodeAdapter::providerKey() const
{
    return QStringLiteral("claude");
}

QString ClaudeCodeAdapter::displayName() const
{
    return domain::providerDisplayNameForKey(providerKey());
}

domain::ports::ProviderProbe ClaudeCodeAdapter::probe() const
{
    const auto executable = providers::support::probeExecutable(
        QStringLiteral("claude"),
        {QStringLiteral("--version")});

    return {.providerKey = providerKey(),
            .displayName = displayName(),
            .available = executable.available,
            .executablePath = executable.executablePath,
            .versionString = executable.versionString,
            .statusMessage = executable.statusMessage};
}

bool ClaudeCodeAdapter::supportsResume() const
{
    return true;
}

bool ClaudeCodeAdapter::supportsModelOverride() const
{
    return true;
}

QString ClaudeCodeAdapter::defaultApiKeyEnvironmentVariable() const
{
    return QStringLiteral("ANTHROPIC_API_KEY");
}

QString ClaudeCodeAdapter::initialResumeToken() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

domain::ports::ProviderLaunchPlan ClaudeCodeAdapter::buildLaunchPlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (safety == domain::LaunchSafety::Bypass) {
        arguments = {QStringLiteral("--dangerously-skip-permissions")};
    } else {
        arguments = {QStringLiteral("--permission-mode"),
                     QStringLiteral("default")};
    }
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    if (!options.resumeToken.trimmed().isEmpty()) {
        arguments << QStringLiteral("--session-id") << options.resumeToken.trimmed();
    }

    return {.executable = QStringLiteral("claude"),
            .arguments = arguments,
            .environment = {}};
}

domain::ports::ProviderLaunchPlan ClaudeCodeAdapter::buildResumePlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (safety == domain::LaunchSafety::Bypass) {
        arguments = {QStringLiteral("--dangerously-skip-permissions")};
    } else {
        arguments = {QStringLiteral("--permission-mode"),
                     QStringLiteral("default")};
    }
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    arguments << QStringLiteral("--resume") << options.resumeToken.trimmed();

    return {.executable = QStringLiteral("claude"),
            .arguments = arguments,
            .environment = {}};
}

QString ClaudeCodeAdapter::launchSafetySummary(const domain::LaunchSafety safety) const
{
    if (safety == domain::LaunchSafety::Bypass) {
        return QStringLiteral("Claude Code launches with permission checks bypassed.");
    }

    return QStringLiteral("Claude Code launches with its normal permission-gated mode.");
}

domain::ports::ProviderObservabilityHints ClaudeCodeAdapter::observabilityHints() const
{
    return {
        .promptPatterns = {QStringLiteral(R"(^\s*\x{276F}(?:\s+.*)?$)")},
        .promptTitlePatterns = {QStringLiteral(R"(^\s*\x{2733}\s+Claude Code$)")},
        .approvalPatterns = {QStringLiteral(R"(\btrust this folder\b)"),
                             QStringLiteral(R"(\byes,\s*i trust this folder\b)"),
                             QStringLiteral(R"(\benter to confirm\b)"),
                             QStringLiteral(R"(\besc to cancel\b)"),
                             QStringLiteral(R"(\bapprove\b)"),
                             QStringLiteral(R"(\bapproval\b)"),
                             QStringLiteral(R"(\ballow\b)"),
                             QStringLiteral(R"(\bpermission\b)"),
                             QStringLiteral(R"(\b[yY]/[nN]\b)")},
        .waitingInputPatterns = {QStringLiteral(R"(\binput required\b)")},
        .activePatterns = {QStringLiteral(R"(^\s*[\x{2722}\x{2723}\x{2726}]\s+.*$)"),
                           QStringLiteral(R"(\bhatching\b)"),
                           QStringLiteral(R"(\bmustering\b)"),
                           QStringLiteral(R"(\bpercolating\b)"),
                           QStringLiteral(R"(\brunning tool\b)"),
                           QStringLiteral(R"(\brunning command\b)"),
                           QStringLiteral(R"(^\s*[●•]\s+Bash\()"),
                           QStringLiteral(R"(^\s*⎿\s+Running)"),
                           QStringLiteral(R"(\bboogieing\b)"),
                           QStringLiteral(R"(\bthinking\b)")},
        .approvalTitlePatterns = {},
        .waitingTitlePatterns = {},
        .activeTitlePatterns = {QStringLiteral(
            R"(^\s*(?:[\x{2800}-\x{28FF}]|[●•◦✢✣✶✦✧⋯…])\s+Claude Code$)")},
        .ignoredTailPatterns = {QStringLiteral(R"(^[─-]{8,}.*$)"),
                                QStringLiteral(R"(^\s*⎿\s+Tip:.*$)"),
                                QStringLiteral(R"(\?\s*for shortcuts\b)"),
                                QStringLiteral(R"(\besc to interrupt\b)"),
                                QStringLiteral(R"(\bbypass permissions on\b.*\bshift\+tab to cycle\b)"),
                                QStringLiteral(R"(\bshift\+tab to cycle\b)"),
                                QStringLiteral(R"(\bctrl\+b ctrl\+b\b.*\brun in background\b)"),
                                QStringLiteral(R"(/model\b)")},
        .preferRecentInputHoldBeforeTailSignals = true,
        .preferActiveTitleDuringRecentInputHold = true,
        .recentInputStateHoldMs = 8000,
        .activeState = domain::SessionState::Thinking,
    };
}

bool ClaudeCodeAdapter::prepareSystemPromptWorkspace(const QString &workspaceDir,
                                                      const QString &promptText) const
{
    QDir dir(workspaceDir);
    if (!dir.mkpath(QStringLiteral(".")) || !dir.mkpath(QStringLiteral(".claude"))) {
        return false;
    }

    QFile file(dir.filePath(QStringLiteral("CLAUDE.md")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    file.write(promptText.toUtf8());
    return true;
}

}  // namespace hydra::infrastructure
