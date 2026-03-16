#include "infrastructure/providers/opencode_cli_adapter.hpp"

#include <filesystem>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "domain/models/provider_identity.hpp"
#include "infrastructure/providers/opencode_session_resolver.hpp"
#include "infrastructure/providers/provider_support.hpp"

namespace hydra::infrastructure {

namespace {

namespace fs = std::filesystem;

QString sharedXdgDataHomePath()
{
    const QString configured = qEnvironmentVariable("XDG_DATA_HOME").trimmed();
    if (!configured.isEmpty()) {
        return QDir::cleanPath(configured);
    }

    return QDir::cleanPath(QDir::homePath() + QStringLiteral("/.local/share"));
}

QString sharedOpenCodeDataPath()
{
    return QDir(sharedXdgDataHomePath()).filePath(QStringLiteral("opencode"));
}

bool ensureSymlinkedDirectoryIfPresent(const QString &sourcePath, const QString &destinationPath)
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isDir()) {
        return true;
    }

    QFileInfo destinationInfo(destinationPath);
    if (destinationInfo.isSymLink()) {
        if (QDir::cleanPath(destinationInfo.symLinkTarget()) == QDir::cleanPath(sourcePath)) {
            return true;
        }
        if (!QFile::remove(destinationPath)) {
            return false;
        }
        destinationInfo = QFileInfo(destinationPath);
    }

    if (destinationInfo.exists()) {
        return true;
    }

    if (!QDir(QFileInfo(destinationPath).absolutePath()).mkpath(QStringLiteral("."))) {
        return false;
    }

    std::error_code error;
    fs::create_directory_symlink(fs::path(sourcePath.toStdString()),
                                 fs::path(destinationPath.toStdString()),
                                 error);
    return !error;
}

bool prepareIsolatedOpenCodeState(const QString &workingDirectory)
{
    const QString xdgDataHome =
        OpenCodeSessionResolver::xdgDataHomePathForWorkingDirectory(workingDirectory);
    const QString xdgStateHome =
        OpenCodeSessionResolver::xdgStateHomePathForWorkingDirectory(workingDirectory);
    const QString xdgCacheHome =
        OpenCodeSessionResolver::xdgCacheHomePathForWorkingDirectory(workingDirectory);
    if (xdgDataHome.isEmpty() || xdgStateHome.isEmpty() || xdgCacheHome.isEmpty()) {
        return false;
    }

    if (!QDir().mkpath(QDir(xdgDataHome).filePath(QStringLiteral("opencode")))
        || !QDir().mkpath(QDir(xdgStateHome).filePath(QStringLiteral("opencode")))
        || !QDir().mkpath(QDir(xdgCacheHome).filePath(QStringLiteral("opencode"))))
    {
        return false;
    }

    return ensureSymlinkedDirectoryIfPresent(
        QDir(sharedOpenCodeDataPath()).filePath(QStringLiteral("bin")),
        QDir(xdgDataHome).filePath(QStringLiteral("opencode/bin")));
}

QString openCodeConfigContent(const domain::LaunchSafety safety)
{
    QJsonObject root;
    root.insert(QStringLiteral("$schema"), QStringLiteral("https://opencode.ai/config.json"));
    root.insert(QStringLiteral("autoupdate"), false);

    if (safety == domain::LaunchSafety::Bypass) {
        root.insert(QStringLiteral("permission"), QStringLiteral("allow"));
    } else {
        QJsonObject permissions;
        permissions.insert(QStringLiteral("bash"), QStringLiteral("ask"));
        permissions.insert(QStringLiteral("edit"), QStringLiteral("ask"));
        root.insert(QStringLiteral("permission"), permissions);
    }

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

QHash<QString, QString> openCodeEnvironmentForInvocation(
    const domain::LaunchSafety safety,
    const domain::ports::ProviderInvocationOptions &options)
{
    QHash<QString, QString> environment;
    environment.insert(QStringLiteral("OPENCODE_CONFIG_CONTENT"), openCodeConfigContent(safety));

    if (options.sessionCategory != domain::SessionCategory::Worker
        && !options.workingDirectory.trimmed().isEmpty()
        && prepareIsolatedOpenCodeState(options.workingDirectory))
    {
        environment.insert(QStringLiteral("XDG_DATA_HOME"),
                           OpenCodeSessionResolver::xdgDataHomePathForWorkingDirectory(
                               options.workingDirectory));
        environment.insert(QStringLiteral("XDG_STATE_HOME"),
                           OpenCodeSessionResolver::xdgStateHomePathForWorkingDirectory(
                               options.workingDirectory));
        environment.insert(QStringLiteral("XDG_CACHE_HOME"),
                           OpenCodeSessionResolver::xdgCacheHomePathForWorkingDirectory(
                               options.workingDirectory));
    }

    return environment;
}

}  // namespace

QString OpenCodeCliAdapter::providerKey() const
{
    return QStringLiteral("opencode");
}

QString OpenCodeCliAdapter::displayName() const
{
    return domain::providerDisplayNameForKey(providerKey());
}

domain::ports::ProviderProbe OpenCodeCliAdapter::probe() const
{
    const auto executable = providers::support::probeExecutable(
        QStringLiteral("opencode"),
        {QStringLiteral("--version")});

    return {.providerKey = providerKey(),
            .displayName = displayName(),
            .available = executable.available,
            .executablePath = executable.executablePath,
            .versionString = executable.versionString,
            .statusMessage = executable.statusMessage};
}

bool OpenCodeCliAdapter::supportsResume() const
{
    return true;
}

bool OpenCodeCliAdapter::supportsModelOverride() const
{
    return true;
}

QString OpenCodeCliAdapter::initialResumeToken() const
{
    return QStringLiteral("opencode-pending-%1")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

domain::ports::ProviderLaunchPlan OpenCodeCliAdapter::buildLaunchPlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }

    return {.executable = QStringLiteral("opencode"),
            .arguments = arguments,
            .environment = openCodeEnvironmentForInvocation(safety, options)};
}

domain::ports::ProviderLaunchPlan OpenCodeCliAdapter::buildResumePlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    arguments << QStringLiteral("--session") << options.resumeToken.trimmed();

    return {.executable = QStringLiteral("opencode"),
            .arguments = arguments,
            .environment = openCodeEnvironmentForInvocation(safety, options)};
}

QString OpenCodeCliAdapter::launchSafetySummary(const domain::LaunchSafety safety) const
{
    if (safety == domain::LaunchSafety::Bypass) {
        return QStringLiteral("OpenCode launches with Hydra forcing permission mode to allow.");
    }

    return QStringLiteral("OpenCode launches with Hydra forcing approval prompts for bash and edit actions.");
}

domain::ports::ProviderObservabilityHints OpenCodeCliAdapter::observabilityHints() const
{
    return {
        .promptPatterns = {QStringLiteral(R"(\bAsk anything\.\.\.)"),
                           QStringLiteral(R"(\bctrl\+t variants\b)"),
                           QStringLiteral(R"(\btab agents\b)"),
                           QStringLiteral(R"(\bctrl\+p commands\b)")},
        .promptTitlePatterns = {},
        .approvalPatterns = {QStringLiteral(R"(\bapprove\b)")},
        .waitingInputPatterns = {QStringLiteral(R"(\binput required\b)")},
        .activePatterns = {QStringLiteral(R"(\besc interrupt\b)")},
        .approvalTitlePatterns = {},
        .waitingTitlePatterns = {},
        .activeTitlePatterns = {},
        .ignoredTailPatterns = {QStringLiteral(R"(^\s*[●•]\s+Tip\b.*$)"),
                                QStringLiteral(R"(^\s*.+\s+/status\s+\S+\s*$)"),
                                QStringLiteral(R"(^\s*Build\s+.+$)")},
        .restrictActiveTailToPromptSegment = true,
        .allowGenericActiveTailFallback = false,
        .preferRecentInputHoldBeforeTailSignals = true,
        .recentInputStateHoldMs = 8000,
        .activeState = domain::SessionState::Thinking,
    };
}

bool OpenCodeCliAdapter::prepareSystemPromptWorkspace(const QString &workspaceDir,
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
    return file.flush();
}

}  // namespace hydra::infrastructure
