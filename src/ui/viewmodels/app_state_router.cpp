#include "ui/viewmodels/app_state.hpp"

#include "domain/models/session_state.hpp"
#include "ui/viewmodels/app_state_control_activity.hpp"

namespace hydra::ui {

QString AppState::routerSessionId() const { return m_routerSessionId; }

QString AppState::routerSessionName() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->name : QString();
}

QString AppState::routerSessionProviderKey() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->providerKey : QString();
}

QString AppState::routerSessionTmuxSessionName() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->tmuxSessionName : QString();
}

QString AppState::routerSessionPaneId() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->tmuxPaneId : QString();
}

QString AppState::routerSessionWorkingDirectory() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->workingDirectory : QString();
}

QString AppState::routerSessionAlias() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? session->alias : QString();
}

bool AppState::routerSessionAvailable() const
{
    return findRouterSession() != nullptr;
}

QString AppState::routerSessionStateTone() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr ? domain::sessionStateToneKey(session->state)
                              : QStringLiteral("idle");
}

QString AppState::routerSessionActivityLabel() const
{
    return activityLabelForControlSession(findRouterSession(), m_timelineBySession);
}

bool AppState::routerSessionApprovalPending() const
{
    const domain::SessionRecord *session = findRouterSession();
    return session != nullptr && session->state == domain::SessionState::AwaitingApproval;
}

const domain::SessionRecord *AppState::findRouterSession() const
{
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited
            && session.category == domain::SessionCategory::Router)
        {
            return &session;
        }
    }
    return nullptr;
}

void AppState::resolveRouterSession()
{
    const domain::SessionRecord *router = findRouterSession();
    const QString nextRouterSessionId = router != nullptr ? router->id : QString();
    if (m_routerSessionId == nextRouterSessionId) {
        return;
    }
    m_routerSessionId = nextRouterSessionId;
    emit routerSessionChanged();
}

void AppState::applyRouterConfig(const QString &providerKey,
                                 const QString &safetyKey,
                                 const QString &presetKey,
                                 const QString &userDefaultContext,
                                 const QString &modelId,
                                 const QString &apiKeyEnvVarName,
                                 const QString &apiKeyValue,
                                 const QString &hermesProfileMode,
                                 const QString &hermesProfilePath)
{
    const bool routerWasRunning = findRouterSession() != nullptr;

    m_routerProviderKey = providerKey.trimmed();
    m_routerLaunchSafety = domain::launchSafetyFromKey(safetyKey);
    m_routerModelId = modelId.trimmed();
    m_routerApiKeyEnvVarName = apiKeyEnvVarName.trimmed();
    m_routerApiKeyValue = apiKeyValue;
    m_routerPresetKey = normalizedRouterPresetKey(presetKey);
    m_routerUserDefaultContext = userDefaultContext;
    m_routerHermesProfileMode = normalizedHermesProfileMode(hermesProfileMode);
    m_routerHermesProfilePath = hermesProfilePath.trimmed().isEmpty()
                                    ? defaultHermesProfileTemplatePath()
                                    : hermesProfilePath.trimmed();
    normalizeRouterPresetState();
    persistSelectionState();
    emit routerConfigChanged();
    if (routerWasRunning) {
        relaunchRouterSession();
    } else {
        showInfoStatus(QStringLiteral("Saved router config. Launch the router when needed."));
    }
}

void AppState::relaunchRouterSession()
{
    const QString routerSessionId = [this]() -> QString {
        const auto *router = findRouterSession();
        return router != nullptr ? router->id : QString();
    }();
    if (!routerSessionId.isEmpty()) {
        m_sessionSupervisor.terminateSession(routerSessionId);
    }
    m_routerSessionId.clear();
    refreshSessionsSnapshot();
    resolveRouterSession();
    emit routerSessionChanged();
    ensureRouterSession();
}

void AppState::ensureRouterSession()
{
    if (findRouterSession() != nullptr) {
        return;
    }

    if (m_routerProviderKey.isEmpty()) {
        return;
    }

    QString launchRepoId = m_selectedRepoId;
    if (launchRepoId.isEmpty() && !m_repositories.isEmpty()) {
        launchRepoId = m_repositories.first().id;
    }
    if (launchRepoId.isEmpty()) {
        return;
    }

    const QString effectiveSysprompt = effectiveRouterSysprompt();
    const QString effectiveModelId = m_routerModelId.trimmed();

    const auto *adapter = m_providerCatalog.adapterFor(m_routerProviderKey);
    if (adapter == nullptr) {
        showWarningStatus(QStringLiteral("Selected router provider could not be resolved."));
        return;
    }

    const QString workingDirectory = routerWorkspacePath();
    if (!stageProviderControlWorkspace(workingDirectory)) {
        showWarningStatus(QStringLiteral("Hydra could not stage the router control workspace."));
        return;
    }
    if (!adapter->prepareSystemPromptWorkspace(workingDirectory, effectiveSysprompt)) {
        showWarningStatus(QStringLiteral("Hydra could not prepare the router prompt workspace for %1.")
                              .arg(adapter->displayName()));
        return;
    }

    QHash<QString, QString> launchEnvironment = buildProviderLaunchEnvironment(
        m_routerProviderKey,
        m_routerApiKeyEnvVarName,
        m_routerApiKeyValue);
    const QHash<QString, QString> promptEnvironment = adapter->promptEnvironment(effectiveSysprompt);
    for (auto it = promptEnvironment.cbegin(); it != promptEnvironment.cend(); ++it) {
        launchEnvironment.insert(it.key(), it.value());
    }
    const QString routerHermesProfileMode = providerUsesHermesProfiles(m_routerProviderKey)
                                                ? m_routerHermesProfileMode
                                                : QString();
    const QString routerHermesProfilePath = providerUsesHermesProfiles(m_routerProviderKey)
                                                ? resolvedHermesProfilePath(
                                                      launchRepoId,
                                                      m_routerHermesProfilePath)
                                                : QString();

    const domain::LaunchOutcome outcome = m_sessionSupervisor.launchSession(
        launchRepoId,
        m_routerProviderKey,
        m_routerLaunchSafety,
        workingDirectory,
        QStringLiteral("Router"),
        domain::SessionCategory::Router,
        effectiveModelId,
        launchEnvironment,
        routerHermesProfileMode,
        routerHermesProfilePath);

    if (outcome.ok) {
        ++m_refreshContextVersion;
        showInfoStatus(outcome.message);
        if (outcome.session.has_value()) {
            upsertSessionInSnapshot(*outcome.session);
        }
        recordActivity(QStringLiteral("router-launch"));
        scheduleRefresh(1500);
    } else {
        showWarningStatus(outcome.message);
    }
}

}  // namespace hydra::ui
