#include "infrastructure/providers/codex_cli_adapter.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QUuid>

#include "domain/models/provider_identity.hpp"
#include "infrastructure/providers/codex_session_resolver.hpp"
#include "infrastructure/providers/provider_support.hpp"

namespace hydra::infrastructure {

namespace {

QString tomlQuoted(const QString &value)
{
    QString escaped = value;
    escaped.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    escaped.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

QString readSharedCodexScalar(const QString &key)
{
    const QString configPath =
        QDir(CodexSessionResolver::sharedCodexHomePath()).filePath(QStringLiteral("config.toml"));
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString contents = QString::fromUtf8(file.readAll());
    const QRegularExpression pattern(
        QStringLiteral(R"codex((?m)^\s*%1\s*=\s*"([^"]+)"\s*$)codex")
            .arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = pattern.match(contents);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}

QString sharedModelForIsolatedCodex()
{
    const QString configuredModel = readSharedCodexScalar(QStringLiteral("model"));
    return configuredModel.isEmpty() ? QStringLiteral("gpt-5.4") : configuredModel;
}

bool syncSharedAuthFile(const QString &codexHomePath)
{
    const QString sharedAuthPath =
        QDir(CodexSessionResolver::sharedCodexHomePath()).filePath(QStringLiteral("auth.json"));
    const QFileInfo sharedAuthInfo(sharedAuthPath);
    if (!sharedAuthInfo.exists()) {
        return true;
    }

    const QString destinationPath = QDir(codexHomePath).filePath(QStringLiteral("auth.json"));
    QFile::remove(destinationPath);
    return QFile::copy(sharedAuthPath, destinationPath);
}

bool prepareCodexHome(const domain::ports::ProviderInvocationOptions &options)
{
    if (options.sessionCategory == domain::SessionCategory::Worker
        || options.workingDirectory.trimmed().isEmpty())
    {
        return true;
    }

    const QString codexHome =
        CodexSessionResolver::codexHomePathForWorkingDirectory(options.workingDirectory);
    if (codexHome.isEmpty()) {
        return false;
    }

    QDir dir(codexHome);
    if (!dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    if (!syncSharedAuthFile(codexHome)) {
        return false;
    }

    QFile configFile(dir.filePath(QStringLiteral("config.toml")));
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&configFile);
    out << "check_for_update_on_startup = false\n";
    out << "history.persistence = \"none\"\n";
    out << "model = " << tomlQuoted(sharedModelForIsolatedCodex()) << "\n";

    const QString sharedReasoning =
        readSharedCodexScalar(QStringLiteral("model_reasoning_effort"));
    if (!sharedReasoning.isEmpty()) {
        out << "model_reasoning_effort = " << tomlQuoted(sharedReasoning) << "\n";
    }

    const QString sharedServiceTier = readSharedCodexScalar(QStringLiteral("service_tier"));
    if (!sharedServiceTier.isEmpty()) {
        out << "service_tier = " << tomlQuoted(sharedServiceTier) << "\n";
    }

    const QString instructionsPath =
        QDir(options.workingDirectory).filePath(QStringLiteral("AGENTS.md"));
    if (QFileInfo::exists(instructionsPath)) {
        out << "model_instructions_file = " << tomlQuoted(instructionsPath) << "\n";
    }

    out << "\n[projects." << tomlQuoted(QDir::cleanPath(options.workingDirectory)) << "]\n";
    out << "trust_level = \"trusted\"\n";
    return configFile.flush();
}

QStringList codexBaseArguments(const domain::LaunchSafety safety)
{
    QStringList arguments = {QStringLiteral("-c"),
                             QStringLiteral("check_for_update_on_startup=false"),
                             QStringLiteral("--no-alt-screen")};
    if (safety == domain::LaunchSafety::Bypass) {
        arguments << QStringLiteral("--dangerously-bypass-approvals-and-sandbox");
    } else {
        arguments << QStringLiteral("--sandbox")
                  << QStringLiteral("workspace-write")
                  << QStringLiteral("-a")
                  << QStringLiteral("on-request");
    }
    return arguments;
}

}  // namespace

QString CodexCliAdapter::providerKey() const
{
    return QStringLiteral("codex");
}

QString CodexCliAdapter::displayName() const
{
    return domain::providerDisplayNameForKey(providerKey());
}

domain::ports::ProviderProbe CodexCliAdapter::probe() const
{
    const auto executable = providers::support::probeExecutable(
        QStringLiteral("codex"),
        {QStringLiteral("--version")});

    return {.providerKey = providerKey(),
            .displayName = displayName(),
            .available = executable.available,
            .executablePath = executable.executablePath,
            .versionString = executable.versionString,
            .statusMessage = executable.statusMessage};
}

bool CodexCliAdapter::supportsResume() const
{
    return true;
}

bool CodexCliAdapter::supportsModelOverride() const
{
    return true;
}

QString CodexCliAdapter::defaultApiKeyEnvironmentVariable() const
{
    return QStringLiteral("OPENAI_API_KEY");
}

QString CodexCliAdapter::initialResumeToken() const
{
    return QStringLiteral("codex-pending-%1")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

domain::ports::ProviderLaunchPlan CodexCliAdapter::buildLaunchPlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments = codexBaseArguments(safety);
    QHash<QString, QString> environment;

    if (prepareCodexHome(options) && options.sessionCategory != domain::SessionCategory::Worker
        && !options.workingDirectory.trimmed().isEmpty())
    {
        environment.insert(QStringLiteral("CODEX_HOME"),
                           CodexSessionResolver::codexHomePathForWorkingDirectory(
                               options.workingDirectory));
    }

    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }

    return {.executable = QStringLiteral("codex"),
            .arguments = arguments,
            .environment = environment};
}

domain::ports::ProviderLaunchPlan CodexCliAdapter::buildResumePlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments = codexBaseArguments(safety);
    QHash<QString, QString> environment;

    if (prepareCodexHome(options) && options.sessionCategory != domain::SessionCategory::Worker
        && !options.workingDirectory.trimmed().isEmpty())
    {
        environment.insert(QStringLiteral("CODEX_HOME"),
                           CodexSessionResolver::codexHomePathForWorkingDirectory(
                               options.workingDirectory));
    }

    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    arguments << QStringLiteral("resume") << options.resumeToken.trimmed();

    return {.executable = QStringLiteral("codex"),
            .arguments = arguments,
            .environment = environment};
}

QString CodexCliAdapter::launchSafetySummary(const domain::LaunchSafety safety) const
{
    if (safety == domain::LaunchSafety::Bypass) {
        return QStringLiteral("Codex launches with approvals and sandbox bypassed.");
    }

    return QStringLiteral("Codex launches in workspace-write sandbox mode with interactive approvals.");
}

domain::ports::ProviderObservabilityHints CodexCliAdapter::observabilityHints() const
{
    return {
        .promptPatterns = {QStringLiteral(R"(^\s*\x{203A}(?:\s+.*)?$)")},
        .promptTitlePatterns = {},
        .approvalPatterns = {QStringLiteral(R"(\bapprove\b)"),
                             QStringLiteral(R"(\bapproval\b)"),
                             QStringLiteral(R"(\ballow\b)"),
                             QStringLiteral(R"(\bpermission\b)"),
                             QStringLiteral(R"(\b[yY]/[nN]\b)")},
        .waitingInputPatterns = {QStringLiteral(R"(\bpress enter to continue\b)"),
                                 QStringLiteral(R"(\binput required\b)"),
                                 QStringLiteral(R"(\btype your response\b)")},
        .activePatterns = {QStringLiteral(R"(\besc to interrupt\b)")},
        .approvalTitlePatterns = {},
        .waitingTitlePatterns = {},
        .activeTitlePatterns = {},
        .ignoredTailPatterns = {QStringLiteral(R"(^gpt-\d[\w.-]*\s+.*% left\b.*$)"),
                                QStringLiteral(R"(^Tip:\s+.*$)")},
        .activeState = domain::SessionState::Thinking,
    };
}

bool CodexCliAdapter::prepareSystemPromptWorkspace(const QString &workspaceDir,
                                                   const QString &promptText) const
{
    QDir dir(workspaceDir);
    if (!dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile file(dir.filePath(QStringLiteral("AGENTS.md")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    file.write(promptText.toUtf8());
    return true;
}

}  // namespace hydra::infrastructure
