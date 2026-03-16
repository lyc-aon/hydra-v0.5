#include "ui/viewmodels/app_state.hpp"

#include <QDebug>
#include <QVariantMap>

#include "domain/models/session_category.hpp"
#include "domain/models/session_state.hpp"

namespace hydra::ui {

AppState::AppState(domain::RepoRegistry &repoRegistry,
                   domain::WorktreeManager &worktreeManager,
                   domain::SessionSupervisor &sessionSupervisor,
                   const domain::ProviderCatalog &providerCatalog,
                   const QString &databasePath,
                   const bool tmuxAvailable,
                   QObject *parent)
    : QObject(parent),
      m_repoRegistry(repoRegistry),
      m_worktreeManager(worktreeManager),
      m_sessionSupervisor(sessionSupervisor),
      m_providerCatalog(providerCatalog),
      m_databasePath(databasePath),
      m_tmuxAvailable(tmuxAvailable),
      m_providerModel(),
      m_repoModel(),
      m_resumeModel(),
      m_sessionModel(),
      m_worktreeModel()
{
    loadPersistedSelectionState();

    m_providerProbes = m_providerCatalog.placeholderProbes();
    if (m_selectedProviderKey.trimmed().isEmpty()) {
        m_selectedProviderKey = m_providerCatalog.firstProviderKey();
    }
    m_providerModel.setSelectedProviderKey(m_selectedProviderKey);
    m_providerModel.setProviders(m_providerProbes);

    m_masterState = std::make_unique<MasterState>(this, this);
    m_routerState = std::make_unique<RouterState>(this, this);

    auto connectFacet = [this](auto signal, QObject *facet) {
        QObject::connect(this, signal, facet, [facet]() {
            QMetaObject::invokeMethod(facet, "stateChanged", Qt::DirectConnection);
        });
    };

    connectFacet(&AppState::launchConfigurationChanged, m_masterState.get());
    connectFacet(&AppState::masterConfigChanged, m_masterState.get());
    connectFacet(&AppState::masterSessionChanged, m_masterState.get());
    connectFacet(&AppState::sessionCountChanged, m_masterState.get());

    connectFacet(&AppState::launchConfigurationChanged, m_routerState.get());
    connectFacet(&AppState::routerConfigChanged, m_routerState.get());
    connectFacet(&AppState::routerSessionChanged, m_routerState.get());
    connectFacet(&AppState::sessionCountChanged, m_routerState.get());

    m_sessionMonitorThreadPool.setMaxThreadCount(1);
    m_sessionMonitorThreadPool.setExpiryTimeout(-1);

    m_statusMessageTimer.setSingleShot(true);
    QObject::connect(&m_statusMessageTimer, &QTimer::timeout, this, [this]() {
        clearStatusMessage();
    });
    QObject::connect(&m_refreshWatcher, &QFutureWatcher<RefreshSnapshot>::finished, this, [this]() {
        const RefreshSnapshot snapshot = m_refreshWatcher.result();
        setRefreshing(false);

        if (snapshot.requestVersion != m_refreshContextVersion) {
            QMetaObject::invokeMethod(this, &AppState::refresh, Qt::QueuedConnection);
            return;
        }

        if (!snapshot.errorMessage.trimmed().isEmpty()) {
            showWarningStatus(QStringLiteral("Hydra refresh failed: %1").arg(snapshot.errorMessage));
            return;
        }

        applyRefreshSnapshot(snapshot);
        if (!snapshot.workspaceSnapshot.ok && !snapshot.workspaceSnapshot.errorMessage.trimmed().isEmpty()) {
            showWarningStatus(snapshot.workspaceSnapshot.errorMessage);
            return;
        }
    });
    QObject::connect(&m_providerProbeWatcher,
                     &QFutureWatcher<ProviderProbeSnapshot>::finished,
                     this,
                     [this]() {
                         applyProviderProbeSnapshot(m_providerProbeWatcher.result());
                     });

    m_sessionMonitorTimer.setInterval(2500);
    QObject::connect(&m_sessionMonitorTimer, &QTimer::timeout, this, &AppState::pollSessionMonitor);
    QObject::connect(&m_sessionMonitorWatcher,
                     &QFutureWatcher<SessionMonitorSnapshot>::finished,
                     this,
                     [this]() {
                         const SessionMonitorSnapshot snapshot = m_sessionMonitorWatcher.result();
                         if (snapshot.requestVersion != m_refreshContextVersion) {
                             return;
                         }

                         if (!snapshot.errorMessage.trimmed().isEmpty()) {
                             qWarning().noquote()
                                 << QStringLiteral("Hydra session monitor failed: %1")
                                        .arg(snapshot.errorMessage);
                             return;
                         }

                         applySessionMonitorSnapshot(snapshot);
                     });

}

AppState::~AppState()
{
    m_statusMessageTimer.stop();
    m_sessionMonitorTimer.stop();
    QObject::disconnect(&m_refreshWatcher, nullptr, this, nullptr);
    QObject::disconnect(&m_providerProbeWatcher, nullptr, this, nullptr);
    QObject::disconnect(&m_sessionMonitorWatcher, nullptr, this, nullptr);
    if (m_refreshWatcher.isRunning()) {
        m_refreshWatcher.waitForFinished();
    }
    if (m_providerProbeWatcher.isRunning()) {
        m_providerProbeWatcher.waitForFinished();
    }
    if (m_sessionMonitorWatcher.isRunning()) {
        m_sessionMonitorWatcher.waitForFinished();
    }
    m_sessionMonitorThreadPool.waitForDone();
}

void AppState::startLifecycle()
{
    if (m_lifecycleStarted) {
        return;
    }

    m_lifecycleStarted = true;
    const domain::SessionShutdownOutcome startupRecoveryOutcome =
        m_sessionSupervisor.cleanupForeignOwnedSessionsForStartup();
    if (!startupRecoveryOutcome.ok) {
        showWarningStatus(QStringLiteral("Hydra could not fully clean up sessions left by an earlier app instance: %1")
                              .arg(startupRecoveryOutcome.errors.join(QStringLiteral(" | "))),
                          5200);
    }
    if (m_tmuxAvailable && !m_sessionMonitorTimer.isActive()) {
        m_sessionMonitorTimer.start();
    }
    refreshProviderCatalogAsync(true);
    refresh();
}

QAbstractListModel *AppState::repoModel()
{
    return &m_repoModel;
}

QAbstractListModel *AppState::providerModel()
{
    return &m_providerModel;
}

QAbstractListModel *AppState::resumeModel()
{
    return &m_resumeModel;
}

QAbstractListModel *AppState::sessionModel()
{
    return &m_sessionModel;
}

QAbstractListModel *AppState::worktreeModel()
{
    return &m_worktreeModel;
}

MasterState *AppState::master() const
{
    return m_masterState.get();
}

RouterState *AppState::router() const
{
    return m_routerState.get();
}

QString AppState::statusMessage() const
{
    return m_statusMessage;
}

bool AppState::statusIsWarning() const
{
    return m_statusIsWarning;
}

bool AppState::refreshing() const
{
    return m_refreshing;
}

QString AppState::repositoryRootPath() const
{
    return m_repositoryRootPath;
}

bool AppState::repositoryIsGit() const
{
    return m_repositoryIsGit;
}

bool AppState::tmuxAvailable() const
{
    return m_tmuxAvailable;
}

int AppState::activityPulse() const
{
    return m_activityPulse;
}

QString AppState::lastActivityKind() const
{
    return m_lastActivityKind;
}

int AppState::repoCount() const
{
    return m_repositories.size();
}

int AppState::resumableSessionCount() const
{
    return m_resumeModel.rowCount();
}

int AppState::ownedLiveSessionCount() const
{
    return m_sessionSupervisor.ownedLiveSessionCount();
}

QString AppState::resumeSessionIdAt(const int row) const
{
    return m_resumeModel.sessionIdAt(row);
}

int AppState::sessionIndexOfId(const QString &sessionId) const
{
    return m_sessionModel.indexOfSessionId(sessionId);
}

int AppState::sessionCount() const
{
    return m_sessionModel.rowCount();
}

int AppState::worktreeCount() const
{
    return m_worktreeModel.rowCount();
}

QStringList AppState::liveTmuxSessionNames() const
{
    QStringList names;
    names.reserve(m_sessions.size());
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state == domain::SessionState::Exited) {
            continue;
        }

        const QString tmuxSessionName = session.tmuxSessionName.trimmed();
        if (!tmuxSessionName.isEmpty()) {
            names.push_back(tmuxSessionName);
        }
    }

    return names;
}

QVariantList AppState::sessionStateDistribution() const
{
    QHash<QString, int> counts;
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state == domain::SessionState::Exited) {
            continue;
        }
        const QString tone = domain::sessionStateToneKey(session.state);
        counts[tone] = counts.value(tone, 0) + 1;
    }

    QVariantList result;
    result.reserve(counts.size());
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        QVariantMap entry;
        entry.insert(QStringLiteral("tone"), it.key());
        entry.insert(QStringLiteral("count"), it.value());
        result.push_back(entry);
    }
    return result;
}

void AppState::clearStatusMessage()
{
    if (m_statusMessage.isEmpty()) {
        return;
    }

    m_statusMessage.clear();
    m_statusIsWarning = false;
    emit statusMessageChanged();
}

void AppState::showInfoStatus(const QString &statusMessage, const int durationMs)
{
    showTransientStatus(statusMessage, durationMs, false);
}

void AppState::showWarningStatus(const QString &statusMessage, const int durationMs)
{
    showTransientStatus(statusMessage, durationMs, true);
}

void AppState::showTransientStatus(const QString &statusMessage,
                                   const int durationMs,
                                   const bool warning)
{
    setStatusMessage(statusMessage, warning);
    if (!statusMessage.isEmpty()) {
        m_statusMessageTimer.start(durationMs);
    }
}

const domain::Repository *AppState::findRepository(const QString &repositoryId) const
{
    for (const domain::Repository &repository : m_repositories) {
        if (repository.id == repositoryId) {
            return &repository;
        }
    }

    return nullptr;
}

void AppState::recordActivity(const QString &kind)
{
    ++m_activityPulse;
    m_lastActivityKind = kind;
    emit activityPulseChanged();
}

void AppState::setRefreshing(const bool refreshing)
{
    if (m_refreshing == refreshing) {
        return;
    }

    m_refreshing = refreshing;
    emit refreshingChanged();
}

void AppState::setStatusMessage(const QString &statusMessage, const bool warning)
{
    m_statusMessageTimer.stop();
    if (m_statusMessage == statusMessage && m_statusIsWarning == warning) {
        return;
    }

    m_statusMessage = statusMessage;
    m_statusIsWarning = !m_statusMessage.isEmpty() && warning;
    emit statusMessageChanged();
}

}  // namespace hydra::ui
