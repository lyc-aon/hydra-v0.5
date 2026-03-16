#include "ui/viewmodels/app_state_facets.hpp"

#include "ui/viewmodels/app_state.hpp"

namespace hydra::ui {

MasterState::MasterState(AppState *appState, QObject *parent)
    : QObject(parent),
      m_appState(appState)
{
}

QString MasterState::sessionId() const { return m_appState->masterSessionId(); }
QString MasterState::sessionName() const { return m_appState->masterSessionName(); }
QString MasterState::sessionProviderKey() const { return m_appState->masterSessionProviderKey(); }
QString MasterState::sessionTmuxSessionName() const { return m_appState->masterSessionTmuxSessionName(); }
QString MasterState::sessionPaneId() const { return m_appState->masterSessionPaneId(); }
QString MasterState::sessionWorkingDirectory() const { return m_appState->masterSessionWorkingDirectory(); }
QString MasterState::sessionAlias() const { return m_appState->masterSessionAlias(); }
bool MasterState::sessionAvailable() const { return m_appState->masterSessionAvailable(); }
QString MasterState::sessionStateTone() const { return m_appState->masterSessionStateTone(); }
QString MasterState::sessionActivityLabel() const { return m_appState->masterSessionActivityLabel(); }
bool MasterState::sessionApprovalPending() const { return m_appState->masterSessionApprovalPending(); }
QString MasterState::providerKey() const { return m_appState->masterProviderKey(); }
void MasterState::setProviderKey(const QString &providerKey) { m_appState->setMasterProviderKey(providerKey); }
QString MasterState::launchSafetyKey() const { return m_appState->masterLaunchSafetyKey(); }
void MasterState::setLaunchSafetyKey(const QString &safetyKey) { m_appState->setMasterLaunchSafetyKey(safetyKey); }
QString MasterState::modelId() const { return m_appState->masterModelId(); }
void MasterState::setModelId(const QString &modelId) { m_appState->setMasterModelId(modelId); }
QString MasterState::apiKeyEnvVarName() const { return m_appState->masterApiKeyEnvVarName(); }
void MasterState::setApiKeyEnvVarName(const QString &envVarName) { m_appState->setMasterApiKeyEnvVarName(envVarName); }
QString MasterState::apiKeyValue() const { return m_appState->masterApiKeyValue(); }
void MasterState::setApiKeyValue(const QString &apiKeyValue) { m_appState->setMasterApiKeyValue(apiKeyValue); }
QString MasterState::sysprompt() const { return m_appState->masterSysprompt(); }
void MasterState::setSysprompt(const QString &sysprompt) { m_appState->setMasterSysprompt(sysprompt); }
QString MasterState::hermesProfileMode() const { return m_appState->masterHermesProfileMode(); }
void MasterState::setHermesProfileMode(const QString &profileMode) { m_appState->setMasterHermesProfileMode(profileMode); }
QString MasterState::hermesProfilePath() const { return m_appState->masterHermesProfilePath(); }
void MasterState::setHermesProfilePath(const QString &profilePath) { m_appState->setMasterHermesProfilePath(profilePath); }
void MasterState::ensureSession() { m_appState->ensureMasterSession(); }
QString MasterState::defaultSysprompt() const { return m_appState->defaultMasterSysprompt(); }
void MasterState::applyConfig(const QString &providerKey,
                              const QString &safetyKey,
                              const QString &sysprompt,
                              const QString &modelId,
                              const QString &apiKeyEnvVarName,
                              const QString &apiKeyValue,
                              const QString &hermesProfileMode,
                              const QString &hermesProfilePath)
{
    m_appState->applyMasterConfig(providerKey,
                                  safetyKey,
                                  sysprompt,
                                  modelId,
                                  apiKeyEnvVarName,
                                  apiKeyValue,
                                  hermesProfileMode,
                                  hermesProfilePath);
}
void MasterState::relaunchSession() { m_appState->relaunchMasterSession(); }

RouterState::RouterState(AppState *appState, QObject *parent)
    : QObject(parent),
      m_appState(appState)
{
}

QString RouterState::sessionId() const { return m_appState->routerSessionId(); }
QString RouterState::sessionName() const { return m_appState->routerSessionName(); }
QString RouterState::sessionProviderKey() const { return m_appState->routerSessionProviderKey(); }
QString RouterState::sessionTmuxSessionName() const { return m_appState->routerSessionTmuxSessionName(); }
QString RouterState::sessionPaneId() const { return m_appState->routerSessionPaneId(); }
QString RouterState::sessionWorkingDirectory() const { return m_appState->routerSessionWorkingDirectory(); }
QString RouterState::sessionAlias() const { return m_appState->routerSessionAlias(); }
bool RouterState::sessionAvailable() const { return m_appState->routerSessionAvailable(); }
QString RouterState::sessionStateTone() const { return m_appState->routerSessionStateTone(); }
QString RouterState::sessionActivityLabel() const { return m_appState->routerSessionActivityLabel(); }
bool RouterState::sessionApprovalPending() const { return m_appState->routerSessionApprovalPending(); }
QString RouterState::providerKey() const { return m_appState->routerProviderKey(); }
void RouterState::setProviderKey(const QString &providerKey) { m_appState->setRouterProviderKey(providerKey); }
QString RouterState::launchSafetyKey() const { return m_appState->routerLaunchSafetyKey(); }
void RouterState::setLaunchSafetyKey(const QString &safetyKey) { m_appState->setRouterLaunchSafetyKey(safetyKey); }
QString RouterState::modelId() const { return m_appState->routerModelId(); }
void RouterState::setModelId(const QString &modelId) { m_appState->setRouterModelId(modelId); }
QString RouterState::apiKeyEnvVarName() const { return m_appState->routerApiKeyEnvVarName(); }
void RouterState::setApiKeyEnvVarName(const QString &envVarName) { m_appState->setRouterApiKeyEnvVarName(envVarName); }
QString RouterState::apiKeyValue() const { return m_appState->routerApiKeyValue(); }
void RouterState::setApiKeyValue(const QString &apiKeyValue) { m_appState->setRouterApiKeyValue(apiKeyValue); }
QString RouterState::presetKey() const { return m_appState->routerPresetKey(); }
void RouterState::setPresetKey(const QString &presetKey) { m_appState->setRouterPresetKey(presetKey); }
QString RouterState::userDefaultContext() const { return m_appState->routerUserDefaultContext(); }
void RouterState::setUserDefaultContext(const QString &defaultContext) { m_appState->setRouterUserDefaultContext(defaultContext); }
QVariantList RouterState::presetOptions() const { return m_appState->routerPresetOptions(); }
QString RouterState::hermesProfileMode() const { return m_appState->routerHermesProfileMode(); }
void RouterState::setHermesProfileMode(const QString &profileMode) { m_appState->setRouterHermesProfileMode(profileMode); }
QString RouterState::hermesProfilePath() const { return m_appState->routerHermesProfilePath(); }
void RouterState::setHermesProfilePath(const QString &profilePath) { m_appState->setRouterHermesProfilePath(profilePath); }
QString RouterState::presetPrompt(const QString &presetKey) const { return m_appState->routerPresetPrompt(presetKey); }
QString RouterState::presetPromptPreview(const QString &presetKey, const QString &userDefaultContext) const
{
    return m_appState->routerPresetPromptPreview(presetKey, userDefaultContext);
}
QString RouterState::presetDescription(const QString &presetKey) const { return m_appState->routerPresetDescription(presetKey); }
bool RouterState::presetIsBuiltIn(const QString &presetKey) const { return m_appState->routerPresetIsBuiltIn(presetKey); }
QString RouterState::saveCustomPreset(const QString &presetKey, const QString &name, const QString &prompt)
{
    return m_appState->saveCustomRouterPreset(presetKey, name, prompt);
}
bool RouterState::deleteCustomPreset(const QString &presetKey) { return m_appState->deleteCustomRouterPreset(presetKey); }
void RouterState::ensureSession() { m_appState->ensureRouterSession(); }
void RouterState::applyConfig(const QString &providerKey,
                              const QString &safetyKey,
                              const QString &presetKey,
                              const QString &userDefaultContext,
                              const QString &modelId,
                              const QString &apiKeyEnvVarName,
                              const QString &apiKeyValue,
                              const QString &hermesProfileMode,
                              const QString &hermesProfilePath)
{
    m_appState->applyRouterConfig(providerKey,
                                  safetyKey,
                                  presetKey,
                                  userDefaultContext,
                                  modelId,
                                  apiKeyEnvVarName,
                                  apiKeyValue,
                                  hermesProfileMode,
                                  hermesProfilePath);
}
void RouterState::relaunchSession() { m_appState->relaunchRouterSession(); }

}  // namespace hydra::ui
