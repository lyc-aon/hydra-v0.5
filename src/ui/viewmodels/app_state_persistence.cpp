#include "ui/viewmodels/app_state.hpp"

#include <QJsonDocument>
#include <QSettings>

namespace hydra::ui {

namespace {

constexpr auto kSelectedRepoIdKey = "workspace/selected_repo_id";
constexpr auto kSelectedWorktreePathKey = "workspace/selected_worktree_path";
constexpr auto kPreserveEmptyRepoSelectionKey = "workspace/preserve_empty_repo_selection";
constexpr auto kSelectedProviderKeyKey = "launch/selected_provider_key";
constexpr auto kSelectedModelIdKey = "launch/selected_model_id";
constexpr auto kSelectedHermesProfileModeKey = "launch/hermes_profile_mode";
constexpr auto kSelectedHermesProfilePathKey = "launch/hermes_profile_path";
constexpr auto kSelectedLaunchSafetyKey = "launch/selected_launch_safety_key";
constexpr auto kLaunchInHomeDirectoryKey = "launch/in_home_directory";
constexpr auto kSelectedSessionIdKey = "sessions/selected_session_id";
constexpr auto kSessionAutosortEnabledKey = "sessions/autosort_enabled";
constexpr auto kSessionManualOrderKey = "sessions/manual_order";
constexpr auto kMasterProviderKeyKey = "master/provider_key";
constexpr auto kMasterLaunchSafetyKeyKey = "master/launch_safety_key";
constexpr auto kMasterModelIdKey = "master/model_id";
constexpr auto kMasterApiKeyEnvVarNameKey = "master/api_key_env_var_name";
constexpr auto kMasterSyspromptKey = "master/sysprompt";
constexpr auto kMasterHermesProfileModeKey = "master/hermes_profile_mode";
constexpr auto kMasterHermesProfilePathKey = "master/hermes_profile_path";
constexpr auto kRouterProviderKeyKey = "router/provider_key";
constexpr auto kRouterLaunchSafetyKeyKey = "router/launch_safety_key";
constexpr auto kRouterModelIdKey = "router/model_id";
constexpr auto kRouterApiKeyEnvVarNameKey = "router/api_key_env_var_name";
constexpr auto kRouterPresetKeyKey = "router/preset_key";
constexpr auto kRouterUserDefaultContextKey = "router/user_default_context";
constexpr auto kRouterCustomPresetsKey = "router/custom_presets";
constexpr auto kRouterHermesProfileModeKey = "router/hermes_profile_mode";
constexpr auto kRouterHermesProfilePathKey = "router/hermes_profile_path";

QString normalizedSettingText(const QVariant &value)
{
    return value.toString().trimmed();
}

}  // namespace

void AppState::loadPersistedSelectionState()
{
    QSettings settings;
    m_selectedRepoId = normalizedSettingText(settings.value(QString::fromUtf8(kSelectedRepoIdKey)));
    m_selectedWorktreePath =
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedWorktreePathKey)));
    m_preserveEmptyRepoSelection =
        settings.value(QString::fromUtf8(kPreserveEmptyRepoSelectionKey), false).toBool();
    m_selectedProviderKey =
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedProviderKeyKey)));
    m_selectedModelId =
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedModelIdKey)));
    m_selectedHermesProfileMode = normalizedHermesProfileMode(
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedHermesProfileModeKey))));
    m_selectedHermesProfilePath =
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedHermesProfilePathKey)));
    if (m_selectedHermesProfilePath.isEmpty()) {
        m_selectedHermesProfilePath = defaultHermesProfileTemplatePath();
    }
    m_selectedLaunchSafety = domain::launchSafetyFromKey(
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedLaunchSafetyKey))));
    m_launchInHomeDirectory =
        settings.value(QString::fromUtf8(kLaunchInHomeDirectoryKey), false).toBool();
    m_selectedSessionId =
        normalizedSettingText(settings.value(QString::fromUtf8(kSelectedSessionIdKey)));
    m_sessionAutosortEnabled =
        settings.value(QString::fromUtf8(kSessionAutosortEnabledKey), false).toBool();
    m_sessionManualOrder = settings.value(QString::fromUtf8(kSessionManualOrderKey)).toStringList();
    m_masterProviderKey = normalizedSettingText(settings.value(QString::fromUtf8(kMasterProviderKeyKey)));
    m_masterLaunchSafety = domain::launchSafetyFromKey(
        normalizedSettingText(settings.value(QString::fromUtf8(kMasterLaunchSafetyKeyKey))));
    m_masterModelId = normalizedSettingText(settings.value(QString::fromUtf8(kMasterModelIdKey)));
    m_masterApiKeyEnvVarName =
        normalizedSettingText(settings.value(QString::fromUtf8(kMasterApiKeyEnvVarNameKey)));
    m_masterSysprompt = settings.value(QString::fromUtf8(kMasterSyspromptKey)).toString();
    m_masterHermesProfileMode = normalizedHermesProfileMode(
        normalizedSettingText(settings.value(QString::fromUtf8(kMasterHermesProfileModeKey))));
    m_masterHermesProfilePath =
        normalizedSettingText(settings.value(QString::fromUtf8(kMasterHermesProfilePathKey)));
    if (m_masterHermesProfilePath.isEmpty()) {
        m_masterHermesProfilePath = defaultHermesProfileTemplatePath();
    }
    m_routerProviderKey = normalizedSettingText(settings.value(QString::fromUtf8(kRouterProviderKeyKey)));
    m_routerLaunchSafety = domain::launchSafetyFromKey(
        normalizedSettingText(settings.value(QString::fromUtf8(kRouterLaunchSafetyKeyKey))));
    m_routerModelId = normalizedSettingText(settings.value(QString::fromUtf8(kRouterModelIdKey)));
    m_routerApiKeyEnvVarName =
        normalizedSettingText(settings.value(QString::fromUtf8(kRouterApiKeyEnvVarNameKey)));
    m_routerPresetKey =
        normalizedSettingText(settings.value(QString::fromUtf8(kRouterPresetKeyKey)));
    m_routerUserDefaultContext =
        settings.value(QString::fromUtf8(kRouterUserDefaultContextKey)).toString();
    m_routerCustomPresets =
        settings.value(QString::fromUtf8(kRouterCustomPresetsKey)).toList();
    m_routerHermesProfileMode = normalizedHermesProfileMode(
        normalizedSettingText(settings.value(QString::fromUtf8(kRouterHermesProfileModeKey))));
    m_routerHermesProfilePath =
        normalizedSettingText(settings.value(QString::fromUtf8(kRouterHermesProfilePathKey)));
    if (m_routerHermesProfilePath.isEmpty()) {
        m_routerHermesProfilePath = defaultHermesProfileTemplatePath();
    }
    normalizeRouterPresetState();
    m_persistedSelectionSignature = selectionPersistenceSignature();
}

void AppState::persistSelectionState()
{
    const QString signature = selectionPersistenceSignature();
    if (m_persistedSelectionSignature == signature) {
        return;
    }

    QSettings settings;
    settings.setValue(QString::fromUtf8(kSelectedRepoIdKey), m_selectedRepoId);
    settings.setValue(QString::fromUtf8(kSelectedWorktreePathKey), m_selectedWorktreePath);
    settings.setValue(QString::fromUtf8(kPreserveEmptyRepoSelectionKey),
                      m_preserveEmptyRepoSelection);
    settings.setValue(QString::fromUtf8(kSelectedProviderKeyKey), m_selectedProviderKey);
    settings.setValue(QString::fromUtf8(kSelectedModelIdKey), m_selectedModelId);
    settings.setValue(QString::fromUtf8(kSelectedHermesProfileModeKey), m_selectedHermesProfileMode);
    settings.setValue(QString::fromUtf8(kSelectedHermesProfilePathKey), m_selectedHermesProfilePath);
    settings.setValue(QString::fromUtf8(kSelectedLaunchSafetyKey),
                      domain::launchSafetyKey(m_selectedLaunchSafety));
    settings.setValue(QString::fromUtf8(kLaunchInHomeDirectoryKey), m_launchInHomeDirectory);
    settings.setValue(QString::fromUtf8(kSelectedSessionIdKey), m_selectedSessionId);
    settings.setValue(QString::fromUtf8(kSessionAutosortEnabledKey), m_sessionAutosortEnabled);
    settings.setValue(QString::fromUtf8(kSessionManualOrderKey), m_sessionManualOrder);
    settings.setValue(QString::fromUtf8(kMasterProviderKeyKey), m_masterProviderKey);
    settings.setValue(QString::fromUtf8(kMasterLaunchSafetyKeyKey),
                      domain::launchSafetyKey(m_masterLaunchSafety));
    settings.setValue(QString::fromUtf8(kMasterModelIdKey), m_masterModelId);
    settings.setValue(QString::fromUtf8(kMasterApiKeyEnvVarNameKey), m_masterApiKeyEnvVarName);
    settings.setValue(QString::fromUtf8(kMasterSyspromptKey), m_masterSysprompt);
    settings.setValue(QString::fromUtf8(kMasterHermesProfileModeKey), m_masterHermesProfileMode);
    settings.setValue(QString::fromUtf8(kMasterHermesProfilePathKey), m_masterHermesProfilePath);
    settings.setValue(QString::fromUtf8(kRouterProviderKeyKey), m_routerProviderKey);
    settings.setValue(QString::fromUtf8(kRouterLaunchSafetyKeyKey),
                      domain::launchSafetyKey(m_routerLaunchSafety));
    settings.setValue(QString::fromUtf8(kRouterModelIdKey), m_routerModelId);
    settings.setValue(QString::fromUtf8(kRouterApiKeyEnvVarNameKey), m_routerApiKeyEnvVarName);
    settings.setValue(QString::fromUtf8(kRouterPresetKeyKey), m_routerPresetKey);
    settings.setValue(QString::fromUtf8(kRouterUserDefaultContextKey), m_routerUserDefaultContext);
    settings.setValue(QString::fromUtf8(kRouterCustomPresetsKey), m_routerCustomPresets);
    settings.setValue(QString::fromUtf8(kRouterHermesProfileModeKey), m_routerHermesProfileMode);
    settings.setValue(QString::fromUtf8(kRouterHermesProfilePathKey), m_routerHermesProfilePath);
    settings.sync();

    m_persistedSelectionSignature = signature;
}

QString AppState::selectionPersistenceSignature() const
{
    const QStringList parts = {
        m_selectedRepoId,
        m_selectedWorktreePath,
        m_preserveEmptyRepoSelection ? QStringLiteral("1") : QStringLiteral("0"),
        m_selectedProviderKey,
        m_selectedModelId,
        m_selectedHermesProfileMode,
        m_selectedHermesProfilePath,
        domain::launchSafetyKey(m_selectedLaunchSafety),
        m_launchInHomeDirectory ? QStringLiteral("1") : QStringLiteral("0"),
        m_selectedSessionId,
        m_sessionAutosortEnabled ? QStringLiteral("1") : QStringLiteral("0"),
        m_sessionManualOrder.join(QStringLiteral("\x1e")),
        m_masterProviderKey,
        domain::launchSafetyKey(m_masterLaunchSafety),
        m_masterModelId,
        m_masterApiKeyEnvVarName,
        m_masterSysprompt,
        m_masterHermesProfileMode,
        m_masterHermesProfilePath,
        m_routerProviderKey,
        domain::launchSafetyKey(m_routerLaunchSafety),
        m_routerModelId,
        m_routerApiKeyEnvVarName,
        m_routerPresetKey,
        m_routerUserDefaultContext,
        QString::fromUtf8(QJsonDocument::fromVariant(m_routerCustomPresets)
                              .toJson(QJsonDocument::Compact)),
        m_routerHermesProfileMode,
        m_routerHermesProfilePath,
    };
    return parts.join(QStringLiteral("\x1f"));
}

}  // namespace hydra::ui
