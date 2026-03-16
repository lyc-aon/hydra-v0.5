#include "ui/viewmodels/app_state.hpp"

#include <QDir>
#include <QFileInfo>

namespace hydra::ui {

namespace {

constexpr auto kHermesProfileGlobalKey = "global";
constexpr auto kHermesProfileBlankKey = "blank";
constexpr auto kHermesProfileRepoTemplateKey = "repo-template";

}  // namespace

bool AppState::providerSupportsModelOverride(const QString &providerKey) const
{
    const auto *adapter = m_providerCatalog.adapterFor(providerKey.trimmed());
    return adapter != nullptr && adapter->supportsModelOverride();
}

QString AppState::normalizedHermesProfileMode(const QString &profileMode)
{
    const QString trimmed = profileMode.trimmed().toLower();
    if (trimmed == QLatin1StringView(kHermesProfileBlankKey)
        || trimmed == QLatin1StringView(kHermesProfileRepoTemplateKey))
    {
        return trimmed;
    }
    return QString::fromUtf8(kHermesProfileGlobalKey);
}

bool AppState::providerUsesHermesProfiles(const QString &providerKey) const
{
    return providerKey.trimmed() == QStringLiteral("hermes");
}

QVariantList AppState::hermesProfileModeOptions() const
{
    return {
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(kHermesProfileGlobalKey)},
            {QStringLiteral("label"), QStringLiteral("GLOBAL")},
        },
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(kHermesProfileBlankKey)},
            {QStringLiteral("label"), QStringLiteral("BLANK")},
        },
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(kHermesProfileRepoTemplateKey)},
            {QStringLiteral("label"), QStringLiteral("REPO TEMPLATE")},
        },
    };
}

QString AppState::defaultHermesProfileTemplatePath() const
{
    return QStringLiteral(".hydra/hermes-profile");
}

QString AppState::hermesProfileModeSummary(const QString &profileMode) const
{
    const QString normalizedMode = normalizedHermesProfileMode(profileMode);
    if (normalizedMode == QLatin1StringView(kHermesProfileBlankKey)) {
        return QStringLiteral(
            "Start Hermes with a clean private home. No shared config, auth, skills, or memories are inherited.");
    }
    if (normalizedMode == QLatin1StringView(kHermesProfileRepoTemplateKey)) {
        return QStringLiteral(
            "Seed the private Hermes home from a repo template path. Missing files fall back to ~/.hermes.");
    }
    return QStringLiteral(
        "Use your shared ~/.hermes config, auth, skills, memories, and hooks while keeping Hydra session state private.");
}

QString AppState::defaultProviderApiKeyEnvVarName(const QString &providerKey) const
{
    const auto *adapter = m_providerCatalog.adapterFor(providerKey.trimmed());
    return adapter != nullptr ? adapter->defaultApiKeyEnvironmentVariable() : QString();
}

QString AppState::normalizedProviderApiKeyEnvVarName(const QString &providerKey,
                                                     const QString &envVarName) const
{
    const QString trimmed = envVarName.trimmed();
    return trimmed.isEmpty() ? defaultProviderApiKeyEnvVarName(providerKey) : trimmed;
}

QHash<QString, QString> AppState::buildProviderLaunchEnvironment(
    const QString &providerKey,
    const QString &apiKeyEnvVarName,
    const QString &apiKeyValue) const
{
    const QString normalizedEnvVar =
        normalizedProviderApiKeyEnvVarName(providerKey, apiKeyEnvVarName);
    const QString trimmedValue = apiKeyValue.trimmed();
    if (normalizedEnvVar.isEmpty() || trimmedValue.isEmpty()) {
        return {};
    }

    return {{normalizedEnvVar, trimmedValue}};
}

QString AppState::resolvedHermesProfilePath(const QString &repositoryId,
                                            const QString &profilePath) const
{
    const QString trimmed = profilePath.trimmed();
    const QString effectivePath =
        trimmed.isEmpty() ? defaultHermesProfileTemplatePath() : trimmed;
    if (effectivePath.isEmpty()) {
        return QString();
    }

    const QFileInfo pathInfo(effectivePath);
    if (pathInfo.isAbsolute()) {
        return QDir::cleanPath(pathInfo.absoluteFilePath());
    }

    const auto repository = m_repoRegistry.repositoryById(repositoryId.trimmed());
    const QString baseDirectory = repository.has_value() && !repository->path.trimmed().isEmpty()
                                      ? repository->path
                                      : QDir::homePath();
    return QDir(baseDirectory).filePath(effectivePath);
}

}  // namespace hydra::ui
