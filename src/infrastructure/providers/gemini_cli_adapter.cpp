#include "infrastructure/providers/gemini_cli_adapter.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "domain/models/provider_identity.hpp"
#include "infrastructure/providers/gemini_session_resolver.hpp"
#include "infrastructure/providers/provider_support.hpp"

namespace hydra::infrastructure {

namespace {

QJsonObject loadJsonObject(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.isObject() ? document.object() : QJsonObject{};
}

bool writeJsonObject(const QString &path, const QJsonObject &object)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return file.flush();
}

QJsonObject mergedGeminiUiSettings(QJsonObject rootObject)
{
    QJsonObject uiSettings = rootObject.value(QStringLiteral("ui")).toObject();
    uiSettings.insert(QStringLiteral("useAlternateBuffer"), false);
    rootObject.insert(QStringLiteral("ui"), uiSettings);
    return rootObject;
}

bool syncGeminiFileIfPresent(const QString &sharedCliHome,
                             const QString &isolatedCliHome,
                             const QString &relativePath)
{
    const QString sharedPath = QDir(sharedCliHome).filePath(relativePath);
    const QFileInfo sharedInfo(sharedPath);
    if (!sharedInfo.exists()) {
        return true;
    }

    const QString destinationPath = QDir(isolatedCliHome).filePath(relativePath);
    QDir destinationDir(QFileInfo(destinationPath).absolutePath());
    if (!destinationDir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile::remove(destinationPath);
    return QFile::copy(sharedPath, destinationPath);
}

bool prepareGeminiCliHome(const domain::ports::ProviderInvocationOptions &options)
{
    if (options.sessionCategory == domain::SessionCategory::Worker
        || options.workingDirectory.trimmed().isEmpty())
    {
        return true;
    }

    const QString geminiCliHome =
        GeminiSessionResolver::geminiCliHomePathForWorkingDirectory(options.workingDirectory);
    if (geminiCliHome.isEmpty()) {
        return false;
    }

    QDir homeDir(geminiCliHome);
    if (!homeDir.mkpath(QStringLiteral(".gemini/tmp"))) {
        return false;
    }

    const QString sharedCliHome = GeminiSessionResolver::sharedGeminiCliHomePath();
    const QStringList sharedFiles = {
        QStringLiteral(".gemini/oauth_creds.json"),
        QStringLiteral(".gemini/google_accounts.json"),
        QStringLiteral(".gemini/installation_id"),
        QStringLiteral(".gemini/trustedFolders.json"),
    };
    for (const QString &relativePath : sharedFiles) {
        if (!syncGeminiFileIfPresent(sharedCliHome, geminiCliHome, relativePath)) {
            return false;
        }
    }

    const QString isolatedGeminiDir = QDir(geminiCliHome).filePath(QStringLiteral(".gemini"));
    QDir geminiDir(isolatedGeminiDir);
    if (!geminiDir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QJsonObject globalSettings = mergedGeminiUiSettings(
        loadJsonObject(QDir(sharedCliHome).filePath(QStringLiteral(".gemini/settings.json"))));
    if (!writeJsonObject(geminiDir.filePath(QStringLiteral("settings.json")), globalSettings)) {
        return false;
    }

    QJsonObject trustedFolders =
        loadJsonObject(geminiDir.filePath(QStringLiteral("trustedFolders.json")));
    trustedFolders.insert(QDir::cleanPath(options.workingDirectory), QStringLiteral("TRUST_FOLDER"));
    return writeJsonObject(geminiDir.filePath(QStringLiteral("trustedFolders.json")),
                           trustedFolders);
}

QHash<QString, QString> geminiEnvironmentForInvocation(
    const domain::ports::ProviderInvocationOptions &options)
{
    QHash<QString, QString> environment;
    if (options.sessionCategory != domain::SessionCategory::Worker
        && !options.workingDirectory.trimmed().isEmpty()
        && prepareGeminiCliHome(options))
    {
        environment.insert(QStringLiteral("GEMINI_CLI_HOME"),
                           GeminiSessionResolver::geminiCliHomePathForWorkingDirectory(
                               options.workingDirectory));
    }

    return environment;
}

}

QString GeminiCliAdapter::providerKey() const
{
    return QStringLiteral("gemini");
}

QString GeminiCliAdapter::displayName() const
{
    return domain::providerDisplayNameForKey(providerKey());
}

domain::ports::ProviderProbe GeminiCliAdapter::probe() const
{
    const auto executable = providers::support::probeExecutable(
        QStringLiteral("gemini"),
        {QStringLiteral("--version")});

    return {.providerKey = providerKey(),
            .displayName = displayName(),
            .available = executable.available,
            .executablePath = executable.executablePath,
            .versionString = executable.versionString,
            .statusMessage = executable.statusMessage};
}

bool GeminiCliAdapter::supportsResume() const
{
    return true;
}

bool GeminiCliAdapter::supportsModelOverride() const
{
    return true;
}

QString GeminiCliAdapter::defaultApiKeyEnvironmentVariable() const
{
    return QStringLiteral("GEMINI_API_KEY");
}

QString GeminiCliAdapter::initialResumeToken() const
{
    return QStringLiteral("gemini-pending-%1")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

domain::ports::ProviderLaunchPlan GeminiCliAdapter::buildLaunchPlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (safety == domain::LaunchSafety::Bypass) {
        arguments = {QStringLiteral("--no-sandbox"),
                     QStringLiteral("--approval-mode"),
                     QStringLiteral("yolo")};
    } else {
        arguments = {QStringLiteral("--sandbox"),
                     QStringLiteral("--approval-mode"),
                     QStringLiteral("default")};
    }

    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    return {.executable = QStringLiteral("gemini"),
            .arguments = arguments,
            .environment = geminiEnvironmentForInvocation(options)};
}

domain::ports::ProviderLaunchPlan GeminiCliAdapter::buildResumePlan(
    const domain::LaunchSafety safety,
    domain::ports::ProviderInvocationOptions options) const
{
    QStringList arguments;
    if (safety == domain::LaunchSafety::Bypass) {
        arguments = {QStringLiteral("--no-sandbox"),
                     QStringLiteral("--approval-mode"),
                     QStringLiteral("yolo")};
    } else {
        arguments = {QStringLiteral("--sandbox"),
                     QStringLiteral("--approval-mode"),
                     QStringLiteral("default")};
    }
    if (!options.modelOverride.trimmed().isEmpty()) {
        arguments << QStringLiteral("--model") << options.modelOverride.trimmed();
    }
    arguments << QStringLiteral("--resume") << options.resumeToken.trimmed();

    return {.executable = QStringLiteral("gemini"),
            .arguments = arguments,
            .environment = geminiEnvironmentForInvocation(options)};
}

QString GeminiCliAdapter::launchSafetySummary(const domain::LaunchSafety safety) const
{
    if (safety == domain::LaunchSafety::Bypass) {
        return QStringLiteral("Gemini launches without sandboxing and auto-approves all actions.");
    }

    return QStringLiteral("Gemini launches with sandboxing enabled and default approval behavior.");
}

domain::ports::ProviderObservabilityHints GeminiCliAdapter::observabilityHints() const
{
    return {
        .promptPatterns = {QStringLiteral(R"(\bType your message\b)"),
                           QStringLiteral(R"(^\s*[>\*](?:\s+.*)?$)")},
        .promptTitlePatterns = {QStringLiteral(R"(^\s*\x{25C7}\s+Ready\b)")},
        .approvalPatterns = {QStringLiteral(R"(\btrust folder\b)"),
                             QStringLiteral(R"(\btrust parent folder\b)"),
                             QStringLiteral(R"(\bdon'?t trust\b)"),
                             QStringLiteral(R"(\bwaiting for auth\b)"),
                             QStringLiteral(R"(\bapprove\b)"),
                             QStringLiteral(R"(\bapproval\b)"),
                             QStringLiteral(R"(\ballow\b)"),
                             QStringLiteral(R"(\bpermission\b)"),
                             QStringLiteral(R"(\b[yY]/[nN]\b)")},
        .waitingInputPatterns = {QStringLiteral(R"(\binput required\b)")},
        .activePatterns = {QStringLiteral(R"(^\s*[\x{2800}-\x{28FF}]+\s+(?:\x{1F4AC}\s+)?.+$)"),
                           QStringLiteral(R"(^\s*\x{2726}\s+.+$)"),
                           QStringLiteral(R"(^\s*[|│]?\s*\x{2713}\s+.+$)"),
                           QStringLiteral(R"(^\s*[|│]\s*\x{22B7}\s+.+$)")},
        .approvalTitlePatterns = {QStringLiteral(R"(^\s*\x{270B}\s+Action Required\b)")},
        .waitingTitlePatterns = {},
        .activeTitlePatterns = {QStringLiteral(R"(^\s*\x{2726}\s+.*$)"),
                                QStringLiteral(R"(^\s*\x{23F2}\s+Working.*$)")},
        .ignoredTailPatterns = {QStringLiteral(R"(^[─-]{8,}$)"),
                                QStringLiteral(R"(^[\s\x{2580}-\x{259F}]+$)"),
                                QStringLiteral(R"(^[\s\x{2500}-\x{257F}]+$)"),
                                QStringLiteral(R"(^\s*[\*>\x{2022}]\s+Type your message\b.*$)"),
                                QStringLiteral(R"(^\s*Logged in with Google:.*$)"),
                                QStringLiteral(R"(^\s*Plan:\s+.*$)"),
                                QStringLiteral(R"(\bGemini CLI update available!\b)"),
                                QStringLiteral(R"(\bAutomatic update is not available in sandbox mode\b)"),
                                QStringLiteral(R"(\bAttempting to automatically update now\b)"),
                                QStringLiteral(R"(\bAutomatic update failed\b)"),
                                QStringLiteral(R"(\bInstalled with npm\.\b)"),
                                QStringLiteral(R"(\bdebug logging of keystrokes\b)"),
                                QStringLiteral(R"(\bToggle Vim mode\b)"),
                                QStringLiteral(R"(\bmodal editing experience\b)"),
                                QStringLiteral(R"(\bToggle Markdown rendering\b)"),
                                QStringLiteral(R"(\braw markdown mode\b)"),
                                QStringLiteral(R"(\bHide the startup banner\b)"),
                                QStringLiteral(R"(^\s*\d+s\)\s+shortcuts\s*$)"),
                                QStringLiteral(R"(^\s*shortcuts\s*$)"),
                                QStringLiteral(R"(^\s*\?\s*for shortcuts\b.*$)"),
                                QStringLiteral(R"(^\s*YOLO\b.*$)"),
                                QStringLiteral(R"(\bshift\+tab to accept edits\b)"),
                                QStringLiteral(R"(^\d+\s+GEMINI\.md\s+file[s]?$)"),
                                QStringLiteral(R"(^\s*>\s+Type your message\b.*$)"),
                                QStringLiteral(R"(^\s*[/~].*\b(?:no sandbox|sandbox(?:-[^\s]+)?)\b.*\/model\b.*$)")},
        .restrictActiveTailToPromptSegment = true,
        .allowGenericActiveTailFallback = false,
        .preferRecentInputHoldBeforeTailSignals = true,
        .preferActiveTitleDuringRecentInputHold = true,
        .activeState = domain::SessionState::Thinking,
    };
}

bool GeminiCliAdapter::prepareSystemPromptWorkspace(const QString &workspaceDir,
                                                     const QString &promptText) const
{
    QDir dir(workspaceDir);
    if (!dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile file(dir.filePath(QStringLiteral("GEMINI.md")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    file.write(promptText.toUtf8());
    file.close();

    const QString settingsDirPath = dir.filePath(QStringLiteral(".gemini"));
    QDir settingsDir(settingsDirPath);
    if (!settingsDir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile settingsFile(settingsDir.filePath(QStringLiteral("settings.json")));
    const QJsonObject existingSettings = loadJsonObject(settingsFile.fileName());
    return writeJsonObject(settingsFile.fileName(), mergedGeminiUiSettings(existingSettings));
}

}  // namespace hydra::infrastructure
