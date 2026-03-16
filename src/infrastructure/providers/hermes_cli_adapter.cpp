#include "infrastructure/providers/hermes_cli_adapter.hpp"

#include <filesystem>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include "domain/models/provider_identity.hpp"
#include "infrastructure/providers/provider_state_paths.hpp"
#include "infrastructure/providers/provider_support.hpp"

namespace hydra::infrastructure {

namespace {

namespace fs = std::filesystem;

constexpr char kHermesPendingPrefix[] = "hermes-pending-";
constexpr char kHermesHydraTokenPrefix[] = "hydra-hermes-";
constexpr char kHermesProfileBlankKey[] = "blank";
constexpr char kHermesProfileRepoTemplateKey[] = "repo-template";

QString makeHydraHermesResumeToken()
{
    return QString::fromLatin1(kHermesHydraTokenPrefix)
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString normalizedHermesResumeToken(const QString &token)
{
    const QString trimmed = token.trimmed();
    if (trimmed.startsWith(QLatin1StringView(kHermesPendingPrefix))) {
        return trimmed.mid(qsizetype(sizeof(kHermesPendingPrefix) - 1));
    }
    if (trimmed.startsWith(QLatin1StringView(kHermesHydraTokenPrefix))) {
        return trimmed;
    }
    return {};
}

QString sharedHermesHomePath()
{
    const QString configuredSharedHome =
        qEnvironmentVariable("HYDRA_HERMES_SHARED_HOME").trimmed();
    if (!configuredSharedHome.isEmpty()) {
        return QDir::cleanPath(configuredSharedHome);
    }

    const QString inheritedHermesHome = qEnvironmentVariable("HERMES_HOME").trimmed();
    if (!inheritedHermesHome.isEmpty()) {
        return QDir::cleanPath(inheritedHermesHome);
    }

    return QDir::cleanPath(QDir::homePath() + QStringLiteral("/.hermes"));
}

QString normalizedHermesProfileMode(const domain::ports::ProviderInvocationOptions &options)
{
    const QString trimmed = options.providerProfileMode.trimmed().toLower();
    if (trimmed == QLatin1StringView(kHermesProfileBlankKey)
        || trimmed == QLatin1StringView(kHermesProfileRepoTemplateKey))
    {
        return trimmed;
    }
    return QStringLiteral("global");
}

QString hermesHomePathForToken(const QString &token)
{
    const QString normalizedToken = normalizedHermesResumeToken(token);
    if (normalizedToken.isEmpty()) {
        return {};
    }

    return QDir(providerStateRootPath(QStringLiteral("hermes"))).filePath(normalizedToken);
}

fs::path toFsPath(const QString &path)
{
    return fs::path(path.toStdString());
}

bool ensureLinkedHermesAsset(const QString &sourcePath, const QString &destinationPath)
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() && !sourceInfo.isSymLink()) {
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

    QDir destinationParent = QFileInfo(destinationPath).dir();
    if (!destinationParent.mkpath(QStringLiteral("."))) {
        return false;
    }

    std::error_code error;
    fs::create_symlink(toFsPath(sourcePath), toFsPath(destinationPath), error);
    return !error;
}

QString hermesTemplateHomePath(const domain::ports::ProviderInvocationOptions &options)
{
    const QString trimmed = options.providerProfilePath.trimmed();
    return trimmed.isEmpty() ? QString() : QDir::cleanPath(trimmed);
}

QString hermesSeedAssetSourcePath(const domain::ports::ProviderInvocationOptions &options,
                                  const QString &asset)
{
    const QString profileMode = normalizedHermesProfileMode(options);
    if (profileMode == QLatin1StringView(kHermesProfileBlankKey)) {
        return {};
    }

    if (profileMode == QLatin1StringView(kHermesProfileRepoTemplateKey)) {
        const QString templateHome = hermesTemplateHomePath(options);
        const QString templateAsset = templateHome.isEmpty()
                                          ? QString()
                                          : QDir(templateHome).filePath(asset);
        const QFileInfo templateInfo(templateAsset);
        if (!templateAsset.isEmpty() && (templateInfo.exists() || templateInfo.isSymLink())) {
            return templateAsset;
        }
    }

    return QDir(sharedHermesHomePath()).filePath(asset);
}

bool ensureHermesHomeInitialized(const QString &token,
                                 const domain::ports::ProviderInvocationOptions &options)
{
    const QString hermesHome = hermesHomePathForToken(token);
    if (hermesHome.isEmpty()) {
        return false;
    }

    QDir privateHome(hermesHome);
    if (!privateHome.mkpath(QStringLiteral("."))) {
        return false;
    }

    const QStringList sharedAssets = {
        QStringLiteral(".env"),
        QStringLiteral("config.yaml"),
        QStringLiteral("auth.json"),
        QStringLiteral("SOUL.md"),
        QStringLiteral("skills"),
        QStringLiteral("memories"),
        QStringLiteral("hooks"),
    };

    for (const QString &asset : sharedAssets) {
        const QString sourcePath = hermesSeedAssetSourcePath(options, asset);
        if (sourcePath.isEmpty()) {
            continue;
        }
        if (!ensureLinkedHermesAsset(sourcePath, privateHome.filePath(asset))) {
            return false;
        }
    }

    return true;
}

QHash<QString, QString> hermesEnvironmentForToken(
    const QString &token,
    const domain::ports::ProviderInvocationOptions &options,
    const bool initializeHome)
{
    const QString hermesHome = hermesHomePathForToken(token);
    if (hermesHome.isEmpty()) {
        return {};
    }

    if (initializeHome && !ensureHermesHomeInitialized(token, options)) {
        return {};
    }

    return {{QStringLiteral("HERMES_HOME"), hermesHome}};
}

QStringList hermesArgumentsForSafety(const domain::LaunchSafety safety)
{
    QStringList arguments{QStringLiteral("chat")};
    if (safety == domain::LaunchSafety::Bypass) {
        arguments << QStringLiteral("--yolo");
    }
    return arguments;
}

}  // namespace

QString HermesCliAdapter::providerKey() const
{
    return QStringLiteral("hermes");
}

QString HermesCliAdapter::displayName() const
{
    return domain::providerDisplayNameForKey(providerKey());
}

domain::ports::ProviderProbe HermesCliAdapter::probe() const
{
    const auto executable = providers::support::probeExecutable(
        QStringLiteral("hermes"),
        {QStringLiteral("version")});

    return {.providerKey = providerKey(),
            .displayName = displayName(),
            .available = executable.available,
            .executablePath = executable.executablePath,
            .versionString = executable.versionString,
            .statusMessage = executable.statusMessage};
}

bool HermesCliAdapter::supportsResume() const
{
    return true;
}

bool HermesCliAdapter::supportsModelOverride() const
{
    return true;
}

QString HermesCliAdapter::initialResumeToken() const
{
    return QString::fromLatin1(kHermesPendingPrefix) + makeHydraHermesResumeToken();
}

QString HermesCliAdapter::expectedResolvedResumeToken(const QString &initialResumeToken) const
{
    return normalizedHermesResumeToken(initialResumeToken);
}

domain::ports::ProviderLaunchPlan HermesCliAdapter::buildLaunchPlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments = hermesArgumentsForSafety(safety);
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }

    return {.executable = QStringLiteral("hermes"),
            .arguments = arguments,
            .environment = hermesEnvironmentForToken(options.resumeToken, options, true)};
}

domain::ports::ProviderLaunchPlan HermesCliAdapter::buildResumePlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    const QString normalizedToken = normalizedHermesResumeToken(options.resumeToken);
    QStringList arguments = hermesArgumentsForSafety(safety);
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }

    if (normalizedToken.startsWith(QLatin1StringView(kHermesHydraTokenPrefix))) {
        arguments << QStringLiteral("--continue");
    } else if (!options.resumeToken.trimmed().isEmpty()) {
        arguments << QStringLiteral("--resume") << options.resumeToken.trimmed();
    }

    return {.executable = QStringLiteral("hermes"),
            .arguments = arguments,
            .environment = hermesEnvironmentForToken(options.resumeToken, options, false)};
}

QString HermesCliAdapter::launchSafetySummary(const domain::LaunchSafety safety) const
{
    if (safety == domain::LaunchSafety::Bypass) {
        return QStringLiteral("Hermes launches with dangerous command approvals bypassed.");
    }

    return QStringLiteral("Hermes launches with its normal command approval flow.");
}

domain::ports::ProviderObservabilityHints HermesCliAdapter::observabilityHints() const
{
    return {
        .promptPatterns = {QStringLiteral(R"(^\s*❯(?:\s+.*)?$)"),
                           QStringLiteral(R"(^\s*[⚔Ψ◉✦]\s+❯(?:\s+.*)?$)")},
        .promptTitlePatterns = {},
        .approvalPatterns = {QStringLiteral(R"(^\s*[⚠🔐]\s+❯(?:\s+.*)?$)"),
                             QStringLiteral(R"(\bapprove\b)"),
                             QStringLiteral(R"(\bapproval\b)"),
                             QStringLiteral(R"(\ballow\b)"),
                             QStringLiteral(R"(\bdangerous command\b)"),
                             QStringLiteral(R"(\b[yY]/[nN]\b)")},
        .waitingInputPatterns = {QStringLiteral(R"(^\s*[✎\?]\s+❯(?:\s+.*)?$)"),
                                 QStringLiteral(R"(\binput required\b)")},
        .activePatterns = {QStringLiteral(R"(^\s*⚕\s+❯(?:\s+.*)?$)"),
                           QStringLiteral(R"(^\s*[⏳◜◠◝◞◡◟]\s+.+$)"),
                           QStringLiteral(R"(^\s*\([^)]*\)\s+[A-Za-z][A-Za-z -]*\.{3}$)")},
        .approvalTitlePatterns = {},
        .waitingTitlePatterns = {},
        .activeTitlePatterns = {},
        .ignoredTailPatterns = {QStringLiteral(R"(^[─-]{8,}.*$)"),
                                QStringLiteral(R"(^\s*Welcome to Hermes Agent!.*$)"),
                                QStringLiteral(R"(^\s*\d+\s+tools\s+·\s+\d+\s+skills\b.*$)"),
                                QStringLiteral(R"(^\s*Session:\s+[A-Za-z0-9_:-]+\s*$)")},
        .allowGenericActiveTailFallback = false,
        .activeState = domain::SessionState::Thinking,
    };
}

QString HermesCliAdapter::postLaunchBootstrapText(domain::ports::ProviderInvocationOptions options) const
{
    Q_UNUSED(options);
    return {};
}

QHash<QString, QString> HermesCliAdapter::promptEnvironment(const QString &promptText) const
{
    const QString trimmed = promptText.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    return {{QStringLiteral("HERMES_EPHEMERAL_SYSTEM_PROMPT"), promptText}};
}

bool HermesCliAdapter::prepareSystemPromptWorkspace(const QString &workspaceDir,
                                                    const QString &promptText) const
{
    Q_UNUSED(promptText);

    QDir dir(workspaceDir);
    return dir.mkpath(QStringLiteral("."));
}

}  // namespace hydra::infrastructure
