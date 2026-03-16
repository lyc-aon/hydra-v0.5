#pragma once

#include <memory>
#include <optional>

#include <QAbstractListModel>
#include <QDateTime>
#include <QFutureWatcher>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QThreadPool>
#include <QVariantList>
#include <QVector>

#include <QVariantMap>

#include "domain/models/launch_safety.hpp"
#include "domain/models/repository.hpp"
#include "domain/models/session_timeline_event.hpp"
#include "domain/models/worktree.hpp"
#include "domain/services/provider_catalog.hpp"
#include "domain/services/repo_registry.hpp"
#include "domain/services/session_supervisor.hpp"
#include "domain/services/worktree_manager.hpp"
#include "ui/viewmodels/app_state_facets.hpp"
#include "ui/viewmodels/provider_list_model.hpp"
#include "ui/viewmodels/repo_list_model.hpp"
#include "ui/viewmodels/resume_list_model.hpp"
#include "ui/viewmodels/session_list_model.hpp"
#include "ui/viewmodels/worktree_list_model.hpp"

namespace hydra::ui {

class AppState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAbstractListModel *repoModel READ repoModel CONSTANT)
    Q_PROPERTY(QAbstractListModel *providerModel READ providerModel CONSTANT)
    Q_PROPERTY(QAbstractListModel *resumeModel READ resumeModel CONSTANT)
    Q_PROPERTY(QAbstractListModel *sessionModel READ sessionModel CONSTANT)
    Q_PROPERTY(QAbstractListModel *worktreeModel READ worktreeModel CONSTANT)
    Q_PROPERTY(QString selectedRepoId READ selectedRepoId WRITE setSelectedRepoId NOTIFY selectedRepoIdChanged)
    Q_PROPERTY(QString selectedRepoName READ selectedRepoName NOTIFY selectedRepoNameChanged)
    Q_PROPERTY(QString selectedWorktreePath READ selectedWorktreePath WRITE setSelectedWorktreePath NOTIFY selectedWorktreePathChanged)
    Q_PROPERTY(QString selectedWorktreeBranch READ selectedWorktreeBranch NOTIFY selectedWorktreeBranchChanged)
    Q_PROPERTY(QString selectedProviderKey READ selectedProviderKey WRITE setSelectedProviderKey NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedProviderName READ selectedProviderName NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedProviderStatusText READ selectedProviderStatusText NOTIFY launchConfigurationChanged)
    Q_PROPERTY(bool selectedProviderAvailable READ selectedProviderAvailable NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedModelId READ selectedModelId WRITE setSelectedModelId NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedHermesProfileMode READ selectedHermesProfileMode WRITE setSelectedHermesProfileMode NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedHermesProfilePath READ selectedHermesProfilePath WRITE setSelectedHermesProfilePath NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QVariantList launchSafetyOptions READ launchSafetyOptions CONSTANT)
    Q_PROPERTY(QString selectedLaunchSafetyKey READ selectedLaunchSafetyKey WRITE setSelectedLaunchSafetyKey NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedLaunchSafetyLabel READ selectedLaunchSafetyLabel NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString selectedLaunchSafetySummary READ selectedLaunchSafetySummary NOTIFY launchConfigurationChanged)
    Q_PROPERTY(bool launchInHomeDirectory READ launchInHomeDirectory WRITE setLaunchInHomeDirectory NOTIFY launchConfigurationChanged)
    Q_PROPERTY(QString homeDirectoryPath READ homeDirectoryPath CONSTANT)
    Q_PROPERTY(QString selectedSessionId READ selectedSessionId WRITE setSelectedSessionId NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionName READ selectedSessionName NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionProviderKey READ selectedSessionProviderKey NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionTmuxSessionName READ selectedSessionTmuxSessionName NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionPaneId READ selectedSessionPaneId NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionWorkingDirectory READ selectedSessionWorkingDirectory NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionLaunchSafetyLabel READ selectedSessionLaunchSafetyLabel NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionLaunchSafetyTone READ selectedSessionLaunchSafetyTone NOTIFY selectedSessionChanged)
    Q_PROPERTY(bool selectedSessionAvailable READ selectedSessionAvailable NOTIFY selectedSessionChanged)
    Q_PROPERTY(QString selectedSessionAlias READ selectedSessionAlias NOTIFY selectedSessionChanged)
    Q_PROPERTY(bool sessionAutosortEnabled READ sessionAutosortEnabled WRITE setSessionAutosortEnabled NOTIFY sessionOrderingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool statusIsWarning READ statusIsWarning NOTIFY statusMessageChanged)
    Q_PROPERTY(bool refreshing READ refreshing NOTIFY refreshingChanged)
    Q_PROPERTY(QString repositoryRootPath READ repositoryRootPath NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool repositoryIsGit READ repositoryIsGit NOTIFY repositoryWorkspaceChanged)
    Q_PROPERTY(bool tmuxAvailable READ tmuxAvailable CONSTANT)
    Q_PROPERTY(int activityPulse READ activityPulse NOTIFY activityPulseChanged)
    Q_PROPERTY(QString lastActivityKind READ lastActivityKind NOTIFY activityPulseChanged)
    Q_PROPERTY(int repoCount READ repoCount NOTIFY repoCountChanged)
    Q_PROPERTY(int resumableSessionCount READ resumableSessionCount NOTIFY sessionCountChanged)
    Q_PROPERTY(int ownedLiveSessionCount READ ownedLiveSessionCount NOTIFY sessionCountChanged)
    Q_PROPERTY(int sessionCount READ sessionCount NOTIFY sessionCountChanged)
    Q_PROPERTY(int worktreeCount READ worktreeCount NOTIFY worktreeCountChanged)
    Q_PROPERTY(QStringList liveTmuxSessionNames READ liveTmuxSessionNames NOTIFY sessionCountChanged)
    Q_PROPERTY(QVariantList sessionStateDistribution READ sessionStateDistribution NOTIFY sessionCountChanged)

public:
    AppState(domain::RepoRegistry &repoRegistry,
             domain::WorktreeManager &worktreeManager,
             domain::SessionSupervisor &sessionSupervisor,
             const domain::ProviderCatalog &providerCatalog,
             const QString &databasePath,
             bool tmuxAvailable,
             QObject *parent = nullptr);
    ~AppState() override;

    QAbstractListModel *repoModel();
    QAbstractListModel *providerModel();
    QAbstractListModel *resumeModel();
    QAbstractListModel *sessionModel();
    QAbstractListModel *worktreeModel();
    MasterState *master() const;
    RouterState *router() const;

    QString selectedRepoId() const;
    void setSelectedRepoId(const QString &selectedRepoId);
    QString selectedRepoName() const;
    QString selectedWorktreePath() const;
    void setSelectedWorktreePath(const QString &selectedWorktreePath);
    QString selectedWorktreeBranch() const;
    QString selectedProviderKey() const;
    void setSelectedProviderKey(const QString &selectedProviderKey);
    QString selectedProviderName() const;
    QString selectedProviderStatusText() const;
    bool selectedProviderAvailable() const;
    QString selectedModelId() const;
    void setSelectedModelId(const QString &selectedModelId);
    QString selectedHermesProfileMode() const;
    void setSelectedHermesProfileMode(const QString &profileMode);
    QString selectedHermesProfilePath() const;
    void setSelectedHermesProfilePath(const QString &profilePath);
    QVariantList launchSafetyOptions() const;
    QString selectedLaunchSafetyKey() const;
    void setSelectedLaunchSafetyKey(const QString &selectedLaunchSafetyKey);
    QString selectedLaunchSafetyLabel() const;
    QString selectedLaunchSafetySummary() const;
    bool launchInHomeDirectory() const;
    void setLaunchInHomeDirectory(bool launchInHomeDirectory);
    QString homeDirectoryPath() const;
    QString selectedSessionId() const;
    void setSelectedSessionId(const QString &selectedSessionId);
    QString selectedSessionName() const;
    QString selectedSessionProviderKey() const;
    QString selectedSessionTmuxSessionName() const;
    QString selectedSessionPaneId() const;
    QString selectedSessionWorkingDirectory() const;
    QString selectedSessionLaunchSafetyLabel() const;
    QString selectedSessionLaunchSafetyTone() const;
    bool selectedSessionAvailable() const;
    QString selectedSessionAlias() const;
    Q_INVOKABLE bool setSessionAlias(const QString &sessionId, const QString &newAlias);
    Q_INVOKABLE QString resumeSessionIdAt(int row) const;
    Q_INVOKABLE int sessionIndexOfId(const QString &sessionId) const;
    bool sessionAutosortEnabled() const;
    void setSessionAutosortEnabled(bool sessionAutosortEnabled);
    Q_INVOKABLE bool selectAdjacentSession(int delta);
    Q_INVOKABLE bool selectFirstSession();
    Q_INVOKABLE bool selectLastSession();
    Q_INVOKABLE bool moveSessionToIndex(const QString &sessionId, int targetIndex);
    QString statusMessage() const;
    bool statusIsWarning() const;
    bool refreshing() const;
    QString repositoryRootPath() const;
    bool repositoryIsGit() const;
    bool tmuxAvailable() const;
    int activityPulse() const;
    QString lastActivityKind() const;
    int repoCount() const;
    int resumableSessionCount() const;
    int ownedLiveSessionCount() const;
    int sessionCount() const;
    int worktreeCount() const;
    QStringList liveTmuxSessionNames() const;
    QVariantList sessionStateDistribution() const;

private:
    friend class MasterState;
    friend class RouterState;

    QString masterSessionId() const;
    QString masterSessionName() const;
    QString masterSessionProviderKey() const;
    QString masterSessionTmuxSessionName() const;
    QString masterSessionPaneId() const;
    QString masterSessionWorkingDirectory() const;
    QString masterSessionAlias() const;
    bool masterSessionAvailable() const;
    QString masterSessionStateTone() const;
    QString masterSessionActivityLabel() const;
    bool masterSessionApprovalPending() const;
    void ensureMasterSession();
    QString defaultMasterSysprompt() const;
    QString defaultRouterSysprompt() const;
    QString masterProviderKey() const;
    void setMasterProviderKey(const QString &providerKey);
    QString masterLaunchSafetyKey() const;
    void setMasterLaunchSafetyKey(const QString &safetyKey);
    QString masterModelId() const;
    void setMasterModelId(const QString &modelId);
    QString masterApiKeyEnvVarName() const;
    void setMasterApiKeyEnvVarName(const QString &envVarName);
    QString masterApiKeyValue() const;
    void setMasterApiKeyValue(const QString &apiKeyValue);
    QString masterSysprompt() const;
    void setMasterSysprompt(const QString &sysprompt);
    QString masterHermesProfileMode() const;
    void setMasterHermesProfileMode(const QString &profileMode);
    QString masterHermesProfilePath() const;
    void setMasterHermesProfilePath(const QString &profilePath);
    void applyMasterConfig(const QString &providerKey,
                           const QString &safetyKey,
                           const QString &sysprompt,
                           const QString &modelId = QString(),
                           const QString &apiKeyEnvVarName = QString(),
                           const QString &apiKeyValue = QString(),
                           const QString &hermesProfileMode = QString(),
                           const QString &hermesProfilePath = QString());
    void relaunchMasterSession();
    QString routerProviderKey() const;
    void setRouterProviderKey(const QString &providerKey);
    QString routerLaunchSafetyKey() const;
    void setRouterLaunchSafetyKey(const QString &safetyKey);
    QString routerModelId() const;
    void setRouterModelId(const QString &modelId);
    QString routerApiKeyEnvVarName() const;
    void setRouterApiKeyEnvVarName(const QString &envVarName);
    QString routerApiKeyValue() const;
    void setRouterApiKeyValue(const QString &apiKeyValue);
    QString routerPresetKey() const;
    void setRouterPresetKey(const QString &presetKey);
    QString routerUserDefaultContext() const;
    void setRouterUserDefaultContext(const QString &defaultContext);
    QVariantList routerPresetOptions() const;
    Q_INVOKABLE QString routerPresetPrompt(const QString &presetKey) const;
    Q_INVOKABLE QString routerPresetPromptPreview(const QString &presetKey,
                                                  const QString &userDefaultContext) const;
    Q_INVOKABLE QString routerPresetDescription(const QString &presetKey) const;
    Q_INVOKABLE bool routerPresetIsBuiltIn(const QString &presetKey) const;
    QString saveCustomRouterPreset(const QString &presetKey,
                                   const QString &name,
                                   const QString &prompt);
    bool deleteCustomRouterPreset(const QString &presetKey);
    QString routerHermesProfileMode() const;
    void setRouterHermesProfileMode(const QString &profileMode);
    QString routerHermesProfilePath() const;
    void setRouterHermesProfilePath(const QString &profilePath);
    QString routerSessionId() const;
    QString routerSessionName() const;
    QString routerSessionProviderKey() const;
    QString routerSessionTmuxSessionName() const;
    QString routerSessionPaneId() const;
    QString routerSessionWorkingDirectory() const;
    QString routerSessionAlias() const;
    bool routerSessionAvailable() const;
    QString routerSessionStateTone() const;
    QString routerSessionActivityLabel() const;
    bool routerSessionApprovalPending() const;
    void ensureRouterSession();
    void applyRouterConfig(const QString &providerKey,
                           const QString &safetyKey,
                           const QString &presetKey = QString(),
                           const QString &userDefaultContext = QString(),
                           const QString &modelId = QString(),
                           const QString &apiKeyEnvVarName = QString(),
                           const QString &apiKeyValue = QString(),
                           const QString &hermesProfileMode = QString(),
                           const QString &hermesProfilePath = QString());
    void relaunchRouterSession();

public:
    Q_INVOKABLE bool providerSupportsModelOverride(const QString &providerKey) const;
    Q_INVOKABLE QString defaultProviderApiKeyEnvVarName(const QString &providerKey) const;
    Q_INVOKABLE bool providerUsesHermesProfiles(const QString &providerKey) const;
    Q_INVOKABLE QVariantList hermesProfileModeOptions() const;
    Q_INVOKABLE QString defaultHermesProfileTemplatePath() const;
    Q_INVOKABLE QString hermesProfileModeSummary(const QString &profileMode) const;
    Q_INVOKABLE void launchSelectedRepoSession();
    Q_INVOKABLE bool addRepositoryPath(const QString &repoPath);
    Q_INVOKABLE void clearSelectedTarget();
    Q_INVOKABLE bool createWorktree(const QString &branchName);
    Q_INVOKABLE bool deleteRepository(const QString &repositoryId);
    Q_INVOKABLE bool deleteWorktree(const QString &worktreePath);
    Q_INVOKABLE bool exportSelectedSessionAudit();
    Q_INVOKABLE void resumeSession(const QString &sessionId);
    Q_INVOKABLE int deleteResumeSessions(const QVariantList &sessionIds);
    Q_INVOKABLE int clearAllResumableSessions();
    Q_INVOKABLE void terminateSession(const QString &sessionId);
    Q_INVOKABLE void startLifecycle();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool shutdownOwnedSessionsForAppExit();

    void reload();

signals:
    void selectedRepoIdChanged();
    void selectedRepoNameChanged();
    void selectedWorktreePathChanged();
    void selectedWorktreeBranchChanged();
    void launchConfigurationChanged();
    void selectedSessionChanged();
    void masterSessionChanged();
    void masterConfigChanged();
    void routerConfigChanged();
    void routerSessionChanged();
    void sessionOrderingChanged();
    void statusMessageChanged();
    void refreshingChanged();
    void repositoryWorkspaceChanged();
    void activityPulseChanged();
    void repoCountChanged();
    void sessionCountChanged();
    void worktreeCountChanged();

private:
    struct RefreshSnapshot {
        quint64 requestVersion = 0;
        QVector<domain::Repository> repositories;
        QString resolvedRepoId;
        domain::RepoWorkspaceSnapshot workspaceSnapshot;
        QVector<domain::SessionRecord> sessions;
        QHash<QString, QVector<domain::SessionTimelineEvent>> timelineBySession;
        QString errorMessage;
    };

    struct ProviderProbeSnapshot {
        QVector<domain::ports::ProviderProbe> providerProbes;
        QString resolvedProviderKey;
    };

    struct SessionMonitorSnapshot {
        quint64 requestVersion = 0;
        QVector<domain::SessionRecord> sessions;
        QHash<QString, QVector<domain::SessionTimelineEvent>> timelineBySession;
        QString errorMessage;
    };

    static RefreshSnapshot loadRefreshSnapshot(const QString &databasePath,
                                               const QString &selectedRepoId,
                                               bool preserveEmptyRepoSelection,
                                               quint64 requestVersion);
    static ProviderProbeSnapshot loadProviderProbeSnapshot(const QString &databasePath,
                                                           const QString &selectedProviderKey);
    static SessionMonitorSnapshot loadSessionMonitorSnapshot(const QString &databasePath,
                                                             quint64 requestVersion);
    void clearStatusMessage();
    void showInfoStatus(const QString &statusMessage, int durationMs = 2600);
    void showWarningStatus(const QString &statusMessage, int durationMs = 3600);
    void showTransientStatus(const QString &statusMessage,
                             int durationMs = 2600,
                             bool warning = false);
    bool ensureRepositorySelected(const QString &warningMessage);
    const domain::Repository *findRepository(const QString &repositoryId) const;
    const domain::SessionRecord *findSession(const QString &sessionId) const;
    const domain::SessionRecord *findMasterSession() const;
    void resolveMasterSession();
    QHash<QString, domain::Repository> repositoriesById() const;
    QVector<domain::SessionRecord> refreshSessionsSnapshot();
    bool hasActiveSessionForRepository(const QString &repositoryId);
    bool hasActiveSessionForWorktree(const QString &worktreePath);
    void resolveSelectedSession(const QString &preferredSessionId = QString());
    void reloadProviderCatalog();
    void reloadSelectedRepoWorkspace();
    void scheduleRefresh(int delayMs = 0);
    void refreshProviderCatalogAsync(bool force = false);
    void applyProviderProbeSnapshot(const ProviderProbeSnapshot &snapshot);
    void applyWorkspaceSnapshot(const domain::RepoWorkspaceSnapshot &snapshot);
    void applyRefreshSnapshot(const RefreshSnapshot &snapshot);
    void applySessionMonitorSnapshot(const SessionMonitorSnapshot &snapshot);
    void normalizeSessionManualOrder();
    QVector<domain::SessionRecord> presentedSessions() const;
    QVector<ResumeListModel::Entry> presentedResumableSessions() const;
    void upsertSessionInSnapshot(const domain::SessionRecord &session);
    void syncSessionModel();
    void removeStoredSessionsFromSnapshot(const QStringList &sessionIds);
    void pollSessionMonitor();
    void updateSessionMonitorCadence(const QVector<domain::SessionRecord> &sessions);
    void recordActivity(const QString &kind);
    void loadPersistedSelectionState();
    void persistSelectionState();
    [[nodiscard]] QString selectionPersistenceSignature() const;
    void setRefreshing(bool refreshing);
    void setStatusMessage(const QString &statusMessage, bool warning = false);
    void setSelectedWorktreePathInternal(const QString &selectedWorktreePath);
    std::optional<domain::ports::ProviderProbe> selectedProviderProbe() const;
    static QString masterWorkspacePath();
    const domain::SessionRecord *findRouterSession() const;
    void resolveRouterSession();
    static QString routerWorkspacePath();
    static QString routerReportsPath();
    static QString sessionManifestPath();
    static QString routerControlScriptPath();
    static QString normalizedHermesProfileMode(const QString &profileMode);
    bool stageProviderControlWorkspace(const QString &workspaceDir) const;
    void writeSessionManifest() const;
    QHash<QString, QString> buildProviderLaunchEnvironment(const QString &providerKey,
                                                           const QString &apiKeyEnvVarName,
                                                           const QString &apiKeyValue) const;
    QString normalizedRouterPresetKey(const QString &presetKey) const;
    QVariantMap routerPresetDetails(const QString &presetKey) const;
    int customRouterPresetIndex(const QString &presetKey) const;
    void normalizeRouterPresetState();
    QString effectiveRouterSysprompt() const;
    QString resolvedHermesProfilePath(const QString &repositoryId,
                                      const QString &profilePath) const;
    QString normalizedProviderApiKeyEnvVarName(const QString &providerKey,
                                               const QString &envVarName) const;

    domain::RepoRegistry &m_repoRegistry;
    domain::WorktreeManager &m_worktreeManager;
    domain::SessionSupervisor &m_sessionSupervisor;
    const domain::ProviderCatalog &m_providerCatalog;
    QString m_databasePath;
    bool m_tmuxAvailable = false;
    bool m_lifecycleStarted = false;
    bool m_refreshing = false;
    QTimer m_statusMessageTimer;
    QTimer m_sessionMonitorTimer;
    QThreadPool m_sessionMonitorThreadPool;
    QFutureWatcher<RefreshSnapshot> m_refreshWatcher;
    QFutureWatcher<ProviderProbeSnapshot> m_providerProbeWatcher;
    QFutureWatcher<SessionMonitorSnapshot> m_sessionMonitorWatcher;
    quint64 m_refreshContextVersion = 0;
    QDateTime m_lastProviderProbeRefreshAt;
    bool m_preserveEmptyRepoSelection = false;
    QString m_selectedRepoId;
    QString m_selectedWorktreePath;
    QString m_selectedProviderKey;
    QString m_selectedModelId;
    QString m_selectedHermesProfileMode = QStringLiteral("global");
    QString m_selectedHermesProfilePath = QStringLiteral(".hydra/hermes-profile");
    QString m_selectedSessionId;
    QString m_masterSessionId;
    QString m_masterProviderKey;
    domain::LaunchSafety m_masterLaunchSafety = domain::LaunchSafety::WorkspaceSafe;
    QString m_masterModelId;
    QString m_masterApiKeyEnvVarName;
    QString m_masterApiKeyValue;
    QString m_masterSysprompt;
    QString m_masterHermesProfileMode = QStringLiteral("global");
    QString m_masterHermesProfilePath = QStringLiteral(".hydra/hermes-profile");
    QString m_routerSessionId;
    QString m_routerProviderKey;
    domain::LaunchSafety m_routerLaunchSafety = domain::LaunchSafety::WorkspaceSafe;
    QString m_routerModelId;
    QString m_routerApiKeyEnvVarName;
    QString m_routerApiKeyValue;
    QString m_routerPresetKey = QStringLiteral("standard");
    QString m_routerUserDefaultContext;
    QVariantList m_routerCustomPresets;
    QString m_routerHermesProfileMode = QStringLiteral("global");
    QString m_routerHermesProfilePath = QStringLiteral(".hydra/hermes-profile");
    domain::LaunchSafety m_selectedLaunchSafety = domain::LaunchSafety::WorkspaceSafe;
    bool m_launchInHomeDirectory = false;
    bool m_sessionAutosortEnabled = false;
    QString m_statusMessage;
    bool m_statusIsWarning = false;
    int m_activityPulse = 0;
    QString m_lastActivityKind;
    QString m_repositoryRootPath;
    bool m_repositoryIsGit = false;
    QVector<domain::ports::ProviderProbe> m_providerProbes;
    QVector<domain::Repository> m_repositories;
    QVector<domain::SessionRecord> m_sessions;
    QHash<QString, QVector<domain::SessionTimelineEvent>> m_timelineBySession;
    QStringList m_sessionManualOrder;
    QVector<domain::Worktree> m_worktrees;
    QString m_persistedSelectionSignature;
    std::unique_ptr<MasterState> m_masterState;
    std::unique_ptr<RouterState> m_routerState;
    ProviderListModel m_providerModel;
    RepoListModel m_repoModel;
    ResumeListModel m_resumeModel;
    SessionListModel m_sessionModel;
    WorktreeListModel m_worktreeModel;
};

}  // namespace hydra::ui
