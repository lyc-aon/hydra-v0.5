#include "ui/viewmodels/app_state.hpp"

#include <algorithm>

#include <QDir>
#include <QVariantMap>

#include "domain/models/session_category.hpp"
#include "domain/models/session_state.hpp"
#include "ui/viewmodels/app_state_control_activity.hpp"

namespace hydra::ui {

QString AppState::selectedRepoId() const
{
    return m_selectedRepoId;
}

void AppState::setSelectedRepoId(const QString &selectedRepoId)
{
    if (m_selectedRepoId == selectedRepoId) {
        return;
    }

    m_selectedRepoId = selectedRepoId;
    if (!m_selectedRepoId.isEmpty()) {
        m_preserveEmptyRepoSelection = false;
        if (m_launchInHomeDirectory) {
            m_launchInHomeDirectory = false;
            emit launchConfigurationChanged();
        }
    }
    ++m_refreshContextVersion;
    m_repoModel.setSelectedRepositoryId(m_selectedRepoId);
    emit selectedRepoIdChanged();
    emit selectedRepoNameChanged();
    reloadSelectedRepoWorkspace();
    persistSelectionState();
    recordActivity(QStringLiteral("target"));
}

QString AppState::selectedRepoName() const
{
    const domain::Repository *repository = findRepository(m_selectedRepoId);
    return repository != nullptr ? repository->name : QString();
}

QString AppState::selectedWorktreePath() const
{
    return m_selectedWorktreePath;
}

void AppState::setSelectedWorktreePath(const QString &selectedWorktreePath)
{
    if (m_selectedWorktreePath == selectedWorktreePath) {
        return;
    }

    setSelectedWorktreePathInternal(selectedWorktreePath);
    ++m_refreshContextVersion;
    persistSelectionState();
    recordActivity(QStringLiteral("target"));
}

QString AppState::selectedWorktreeBranch() const
{
    for (const domain::Worktree &worktree : m_worktrees) {
        if (worktree.path == m_selectedWorktreePath) {
            return worktree.branchName;
        }
    }

    return QString();
}

QString AppState::selectedProviderKey() const
{
    return m_selectedProviderKey;
}

void AppState::setSelectedProviderKey(const QString &selectedProviderKey)
{
    const QString trimmedProviderKey = selectedProviderKey.trimmed();
    if (trimmedProviderKey.isEmpty() || m_selectedProviderKey == trimmedProviderKey) {
        return;
    }

    m_selectedProviderKey = trimmedProviderKey;
    ++m_refreshContextVersion;
    m_providerModel.setSelectedProviderKey(m_selectedProviderKey);
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("provider"));
}

QString AppState::selectedProviderName() const
{
    const auto provider = selectedProviderProbe();
    return provider.has_value() ? provider->displayName : QStringLiteral("No provider selected");
}

QString AppState::selectedProviderStatusText() const
{
    const auto provider = selectedProviderProbe();
    return provider.has_value() ? provider->statusMessage
                                : QStringLiteral("No provider selected.");
}

bool AppState::selectedProviderAvailable() const
{
    const auto provider = selectedProviderProbe();
    return provider.has_value() && provider->available;
}

QString AppState::selectedModelId() const
{
    return m_selectedModelId;
}

void AppState::setSelectedModelId(const QString &selectedModelId)
{
    const QString trimmedModelId = selectedModelId.trimmed();
    if (m_selectedModelId == trimmedModelId) {
        return;
    }

    m_selectedModelId = trimmedModelId;
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("model"));
}

QString AppState::selectedHermesProfileMode() const
{
    return normalizedHermesProfileMode(m_selectedHermesProfileMode);
}

void AppState::setSelectedHermesProfileMode(const QString &profileMode)
{
    const QString normalizedMode = normalizedHermesProfileMode(profileMode);
    if (m_selectedHermesProfileMode == normalizedMode) {
        return;
    }

    m_selectedHermesProfileMode = normalizedMode;
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("hermes-profile-mode"));
}

QString AppState::selectedHermesProfilePath() const
{
    return m_selectedHermesProfilePath.trimmed().isEmpty()
               ? defaultHermesProfileTemplatePath()
               : m_selectedHermesProfilePath;
}

void AppState::setSelectedHermesProfilePath(const QString &profilePath)
{
    const QString trimmed = profilePath.trimmed();
    const QString normalizedPath =
        trimmed.isEmpty() ? defaultHermesProfileTemplatePath() : trimmed;
    if (m_selectedHermesProfilePath == normalizedPath) {
        return;
    }

    m_selectedHermesProfilePath = normalizedPath;
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("hermes-profile-path"));
}

QVariantList AppState::launchSafetyOptions() const
{
    return {
        QVariantMap{
            {QStringLiteral("key"), domain::launchSafetyKey(domain::LaunchSafety::WorkspaceSafe)},
            {QStringLiteral("label"),
             domain::launchSafetyDisplayName(domain::LaunchSafety::WorkspaceSafe)},
        },
        QVariantMap{
            {QStringLiteral("key"), domain::launchSafetyKey(domain::LaunchSafety::Bypass)},
            {QStringLiteral("label"), domain::launchSafetyDisplayName(domain::LaunchSafety::Bypass)},
        },
    };
}

QString AppState::selectedLaunchSafetyKey() const
{
    return domain::launchSafetyKey(m_selectedLaunchSafety);
}

void AppState::setSelectedLaunchSafetyKey(const QString &selectedLaunchSafetyKey)
{
    const domain::LaunchSafety nextSafety =
        domain::launchSafetyFromKey(selectedLaunchSafetyKey.trimmed());
    if (m_selectedLaunchSafety == nextSafety) {
        return;
    }

    m_selectedLaunchSafety = nextSafety;
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("safety"));
}

QString AppState::selectedLaunchSafetyLabel() const
{
    return domain::launchSafetyDisplayName(m_selectedLaunchSafety);
}

QString AppState::selectedLaunchSafetySummary() const
{
    const domain::ports::ProviderAdapter *adapter =
        m_providerCatalog.adapterFor(m_selectedProviderKey);
    if (adapter == nullptr) {
        return QStringLiteral("Select a provider to review its sandbox behavior.");
    }

    return adapter->launchSafetySummary(m_selectedLaunchSafety);
}

bool AppState::launchInHomeDirectory() const
{
    return m_launchInHomeDirectory;
}

void AppState::setLaunchInHomeDirectory(const bool launchInHomeDirectory)
{
    if (m_launchInHomeDirectory == launchInHomeDirectory) {
        return;
    }

    m_launchInHomeDirectory = launchInHomeDirectory;
    emit launchConfigurationChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("target-mode"));
}

QString AppState::homeDirectoryPath() const
{
    return QDir::homePath();
}

QString AppState::selectedSessionId() const
{
    return m_selectedSessionId;
}

void AppState::setSelectedSessionId(const QString &selectedSessionId)
{
    const QString trimmedSessionId = selectedSessionId.trimmed();
    if (m_selectedSessionId == trimmedSessionId) {
        return;
    }

    m_selectedSessionId = trimmedSessionId;
    m_sessionModel.setSelectedSessionId(m_selectedSessionId);
    emit selectedSessionChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("session"));
}

QString AppState::selectedSessionName() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->name : QString();
}

QString AppState::selectedSessionProviderKey() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->providerKey : QString();
}

QString AppState::selectedSessionTmuxSessionName() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->tmuxSessionName : QString();
}

QString AppState::selectedSessionPaneId() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->tmuxPaneId : QString();
}

QString AppState::selectedSessionWorkingDirectory() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->workingDirectory : QString();
}

QString AppState::selectedSessionLaunchSafetyLabel() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    if (session == nullptr) {
        return QString();
    }
    return domain::launchSafetyDisplayName(domain::launchSafetyFromKey(session->launchSafetyKey));
}

QString AppState::selectedSessionLaunchSafetyTone() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    if (session == nullptr) {
        return QStringLiteral("steel");
    }
    return session->launchSafetyKey == QStringLiteral("bypass")
               ? QStringLiteral("danger")
               : QStringLiteral("ready");
}

bool AppState::selectedSessionAvailable() const
{
    return findSession(m_selectedSessionId) != nullptr;
}

QString AppState::selectedSessionAlias() const
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    return session != nullptr ? session->alias : QString();
}

bool AppState::setSessionAlias(const QString &sessionId, const QString &newAlias)
{
    if (!m_sessionSupervisor.setSessionAlias(sessionId, newAlias)) {
        showWarningStatus(QStringLiteral("Alias \"%1\" is already in use by another session.").arg(newAlias.trimmed()));
        return false;
    }

    // Update local cache immediately so the UI reflects the change without waiting for the poll.
    const QString normalized = newAlias.trimmed();
    for (auto &session : m_sessions) {
        if (session.id == sessionId) {
            session.alias = normalized;
            break;
        }
    }
    syncSessionModel();
    emit selectedSessionChanged();
    showInfoStatus(QStringLiteral("Alias set to @%1").arg(normalized.toLower()));
    return true;
}

bool AppState::sessionAutosortEnabled() const
{
    return m_sessionAutosortEnabled;
}

void AppState::setSessionAutosortEnabled(const bool sessionAutosortEnabled)
{
    if (m_sessionAutosortEnabled == sessionAutosortEnabled) {
        return;
    }

    m_sessionAutosortEnabled = sessionAutosortEnabled;
    syncSessionModel();
    emit sessionOrderingChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("session-order"));
}

bool AppState::selectAdjacentSession(const int delta)
{
    const QVector<domain::SessionRecord> sessions = presentedSessions();
    if (sessions.isEmpty()) {
        return false;
    }

    int currentIndex = -1;
    for (int index = 0; index < sessions.size(); ++index) {
        if (sessions.at(index).id == m_selectedSessionId) {
            currentIndex = index;
            break;
        }
    }

    if (currentIndex < 0) {
        currentIndex = delta >= 0 ? -1 : sessions.size();
    }

    const int lastIndex = static_cast<int>(sessions.size()) - 1;
    const int nextIndex = std::clamp(currentIndex + delta, 0, lastIndex);
    setSelectedSessionId(sessions.at(nextIndex).id);
    return true;
}

bool AppState::selectFirstSession()
{
    const QVector<domain::SessionRecord> sessions = presentedSessions();
    if (sessions.isEmpty()) {
        return false;
    }

    setSelectedSessionId(sessions.first().id);
    return true;
}

bool AppState::selectLastSession()
{
    const QVector<domain::SessionRecord> sessions = presentedSessions();
    if (sessions.isEmpty()) {
        return false;
    }

    setSelectedSessionId(sessions.last().id);
    return true;
}

bool AppState::moveSessionToIndex(const QString &sessionId, int targetIndex)
{
    const QString trimmedSessionId = sessionId.trimmed();
    if (trimmedSessionId.isEmpty() || m_sessionAutosortEnabled) {
        return false;
    }

    normalizeSessionManualOrder();
    const int currentIndex = m_sessionManualOrder.indexOf(trimmedSessionId);
    if (currentIndex < 0) {
        return false;
    }

    const int lastIndex = static_cast<int>(m_sessionManualOrder.size()) - 1;
    const int clampedTargetIndex = std::clamp(targetIndex, 0, lastIndex);
    if (currentIndex == clampedTargetIndex) {
        return false;
    }

    m_sessionManualOrder.move(currentIndex, clampedTargetIndex);
    syncSessionModel();
    emit sessionOrderingChanged();
    persistSelectionState();
    recordActivity(QStringLiteral("session-order"));
    return true;
}

void AppState::setSelectedWorktreePathInternal(const QString &selectedWorktreePath)
{
    if (m_selectedWorktreePath == selectedWorktreePath) {
        m_worktreeModel.setSelectedWorktreePath(m_selectedWorktreePath);
        return;
    }

    m_selectedWorktreePath = selectedWorktreePath;
    if (!m_selectedWorktreePath.isEmpty() && m_launchInHomeDirectory) {
        m_launchInHomeDirectory = false;
        emit launchConfigurationChanged();
    }
    m_worktreeModel.setSelectedWorktreePath(m_selectedWorktreePath);
    emit selectedWorktreePathChanged();
    emit selectedWorktreeBranchChanged();
    persistSelectionState();
}

std::optional<domain::ports::ProviderProbe> AppState::selectedProviderProbe() const
{
    for (const domain::ports::ProviderProbe &probe : m_providerProbes) {
        if (probe.providerKey == m_selectedProviderKey) {
            return probe;
        }
    }

    return std::nullopt;
}

const domain::SessionRecord *AppState::findMasterSession() const
{
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state != domain::SessionState::Exited
            && session.category == domain::SessionCategory::Master) {
            return &session;
        }
    }

    return nullptr;
}

QString AppState::masterSessionId() const
{
    return m_masterSessionId;
}

QString AppState::masterSessionName() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->name : QString();
}

QString AppState::masterSessionProviderKey() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->providerKey : QString();
}

QString AppState::masterSessionTmuxSessionName() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->tmuxSessionName : QString();
}

QString AppState::masterSessionPaneId() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->tmuxPaneId : QString();
}

QString AppState::masterSessionWorkingDirectory() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->workingDirectory : QString();
}

QString AppState::masterSessionAlias() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? session->alias : QString();
}

bool AppState::masterSessionAvailable() const
{
    return findMasterSession() != nullptr;
}

QString AppState::masterSessionStateTone() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr ? domain::sessionStateToneKey(session->state)
                              : QStringLiteral("idle");
}

QString AppState::masterSessionActivityLabel() const
{
    return activityLabelForControlSession(findMasterSession(), m_timelineBySession);
}

bool AppState::masterSessionApprovalPending() const
{
    const domain::SessionRecord *session = findMasterSession();
    return session != nullptr && session->state == domain::SessionState::AwaitingApproval;
}

void AppState::resolveMasterSession()
{
    const domain::SessionRecord *master = findMasterSession();
    const QString nextMasterSessionId = master != nullptr ? master->id : QString();
    if (m_masterSessionId == nextMasterSessionId) {
        return;
    }
    m_masterSessionId = nextMasterSessionId;
    emit masterSessionChanged();
}

const domain::SessionRecord *AppState::findSession(const QString &sessionId) const
{
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.id == sessionId && session.state != domain::SessionState::Exited) {
            return &session;
        }
    }

    return nullptr;
}

void AppState::resolveSelectedSession(const QString &preferredSessionId)
{
    const QString trimmedPreferredSessionId = preferredSessionId.trimmed();
    const QVector<domain::SessionRecord> livePresentedSessions = presentedSessions();
    auto chooseFirstLiveSession = [this]() -> QString {
        const QVector<domain::SessionRecord> sessions = presentedSessions();
        if (!sessions.isEmpty()) {
            return sessions.first().id;
        }
        return {};
    };

    QString nextSelectedSessionId = m_selectedSessionId;
    if (!trimmedPreferredSessionId.isEmpty()) {
        nextSelectedSessionId = trimmedPreferredSessionId;
    }

    bool selectedSessionStillExists = false;
    for (const domain::SessionRecord &session : livePresentedSessions) {
        if (session.id == nextSelectedSessionId) {
            selectedSessionStillExists = true;
            break;
        }
    }

    if (!selectedSessionStillExists) {
        nextSelectedSessionId = chooseFirstLiveSession();
    }

    if (m_selectedSessionId == nextSelectedSessionId) {
        m_sessionModel.setSelectedSessionId(m_selectedSessionId);
        persistSelectionState();
        return;
    }

    m_selectedSessionId = nextSelectedSessionId;
    m_sessionModel.setSelectedSessionId(m_selectedSessionId);
    emit selectedSessionChanged();
    persistSelectionState();
}

}  // namespace hydra::ui
