#include "ui/viewmodels/app_state.hpp"

#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QStandardPaths>

#include "domain/models/session_state.hpp"
#include "ui/viewmodels/router_preset_library.hpp"

namespace hydra::ui {

namespace {

bool writeWorkspaceFile(const QString &path,
                        const QByteArray &content,
                        const QFileDevice::Permissions permissions)
{
    QFile existing(path);
    if (existing.open(QIODevice::ReadOnly) && existing.readAll() == content) {
        existing.close();
        if (existing.permissions() != permissions) {
            existing.setPermissions(permissions);
        }
        return true;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    if (file.write(content) != content.size()) {
        file.close();
        return false;
    }
    file.close();
    return file.setPermissions(permissions);
}

}  // namespace

QString AppState::routerProviderKey() const { return m_routerProviderKey; }

void AppState::setRouterProviderKey(const QString &providerKey)
{
    if (m_routerProviderKey == providerKey) return;
    m_routerProviderKey = providerKey;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerLaunchSafetyKey() const
{
    return domain::launchSafetyKey(m_routerLaunchSafety);
}

void AppState::setRouterLaunchSafetyKey(const QString &safetyKey)
{
    const auto safety = domain::launchSafetyFromKey(safetyKey);
    if (m_routerLaunchSafety == safety) return;
    m_routerLaunchSafety = safety;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerModelId() const { return m_routerModelId; }

void AppState::setRouterModelId(const QString &modelId)
{
    const QString trimmed = modelId.trimmed();
    if (m_routerModelId == trimmed) return;
    m_routerModelId = trimmed;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerApiKeyEnvVarName() const
{
    return normalizedProviderApiKeyEnvVarName(m_routerProviderKey, m_routerApiKeyEnvVarName);
}

void AppState::setRouterApiKeyEnvVarName(const QString &envVarName)
{
    const QString trimmed = envVarName.trimmed();
    if (m_routerApiKeyEnvVarName == trimmed) return;
    m_routerApiKeyEnvVarName = trimmed;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerApiKeyValue() const { return m_routerApiKeyValue; }

void AppState::setRouterApiKeyValue(const QString &apiKeyValue)
{
    if (m_routerApiKeyValue == apiKeyValue) return;
    m_routerApiKeyValue = apiKeyValue;
    emit routerConfigChanged();
}

QString AppState::normalizedRouterPresetKey(const QString &presetKey) const
{
    const QString trimmed = presetKey.trimmed().toLower();
    if (trimmed.isEmpty()) {
        return QString::fromUtf8(router_presets::kStandardKey);
    }
    if (router_presets::isBuiltInKey(trimmed)) {
        return trimmed;
    }
    return customRouterPresetIndex(trimmed) >= 0 ? trimmed
                                                 : QString::fromUtf8(router_presets::kStandardKey);
}

int AppState::customRouterPresetIndex(const QString &presetKey) const
{
    const QString trimmed = presetKey.trimmed();
    if (trimmed.isEmpty()) {
        return -1;
    }

    for (int index = 0; index < m_routerCustomPresets.size(); ++index) {
        const QVariantMap preset = m_routerCustomPresets.at(index).toMap();
        if (preset.value(QStringLiteral("key")).toString() == trimmed) {
            return index;
        }
    }
    return -1;
}

QVariantMap AppState::routerPresetDetails(const QString &presetKey) const
{
    const QString normalizedKey = presetKey.trimmed().isEmpty()
                                      ? normalizedRouterPresetKey(m_routerPresetKey)
                                      : presetKey.trimmed();
    if (router_presets::isBuiltInKey(normalizedKey)) {
        return {
            {QStringLiteral("key"), normalizedKey},
            {QStringLiteral("label"), router_presets::builtInLabel(normalizedKey)},
            {QStringLiteral("description"), router_presets::builtInDescription(normalizedKey)},
            {QStringLiteral("prompt"),
             router_presets::builtInPrompt(normalizedKey, m_routerUserDefaultContext)},
            {QStringLiteral("builtIn"), true},
        };
    }

    const int customIndex = customRouterPresetIndex(normalizedKey);
    if (customIndex >= 0) {
        QVariantMap preset = m_routerCustomPresets.at(customIndex).toMap();
        preset.insert(QStringLiteral("description"),
                      QStringLiteral("User-saved router preset."));
        preset.insert(QStringLiteral("builtIn"), false);
        return preset;
    }

    return routerPresetDetails(QString::fromUtf8(router_presets::kStandardKey));
}

void AppState::normalizeRouterPresetState()
{
    QVariantList normalizedPresets;
    QSet<QString> usedKeys;
    int customOrdinal = 1;

    const QVariantList customPresets = m_routerCustomPresets;
    for (const QVariant &entry : customPresets) {
        const QVariantMap rawPreset = entry.toMap();
        QString key = rawPreset.value(QStringLiteral("key")).toString().trimmed().toLower();
        const QString prompt = rawPreset.value(QStringLiteral("prompt")).toString();
        QString label = rawPreset.value(QStringLiteral("label")).toString().trimmed();

        if (prompt.trimmed().isEmpty()) {
            continue;
        }
        if (key.isEmpty() || router_presets::isBuiltInKey(key) || usedKeys.contains(key)
            || !key.startsWith(QLatin1StringView(router_presets::kCustomPresetPrefix)))
        {
            key = router_presets::makeCustomPresetId();
        }
        if (label.isEmpty()) {
            label = QStringLiteral("CUSTOM %1").arg(customOrdinal);
        }

        QVariantMap normalizedPreset{
            {QStringLiteral("key"), key},
            {QStringLiteral("label"), label},
            {QStringLiteral("prompt"), prompt},
        };
        normalizedPresets.append(normalizedPreset);
        usedKeys.insert(key);
        ++customOrdinal;
    }

    m_routerCustomPresets = normalizedPresets;
    m_routerPresetKey = normalizedRouterPresetKey(m_routerPresetKey);
}

QString AppState::routerPresetKey() const
{
    return normalizedRouterPresetKey(m_routerPresetKey);
}

void AppState::setRouterPresetKey(const QString &presetKey)
{
    const QString normalizedKey = normalizedRouterPresetKey(presetKey);
    if (m_routerPresetKey == normalizedKey) {
        return;
    }

    m_routerPresetKey = normalizedKey;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerUserDefaultContext() const
{
    return m_routerUserDefaultContext;
}

void AppState::setRouterUserDefaultContext(const QString &defaultContext)
{
    if (m_routerUserDefaultContext == defaultContext) {
        return;
    }

    m_routerUserDefaultContext = defaultContext;
    persistSelectionState();
    emit routerConfigChanged();
}

QVariantList AppState::routerPresetOptions() const
{
    QVariantList options{
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(router_presets::kDirectKey)},
            {QStringLiteral("label"), router_presets::builtInLabel(QString::fromUtf8(router_presets::kDirectKey))},
            {QStringLiteral("description"), router_presets::builtInDescription(QString::fromUtf8(router_presets::kDirectKey))},
            {QStringLiteral("builtIn"), true},
        },
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(router_presets::kStandardKey)},
            {QStringLiteral("label"), router_presets::builtInLabel(QString::fromUtf8(router_presets::kStandardKey))},
            {QStringLiteral("description"), router_presets::builtInDescription(QString::fromUtf8(router_presets::kStandardKey))},
            {QStringLiteral("builtIn"), true},
        },
        QVariantMap{
            {QStringLiteral("key"), QString::fromUtf8(router_presets::kStrategicKey)},
            {QStringLiteral("label"), router_presets::builtInLabel(QString::fromUtf8(router_presets::kStrategicKey))},
            {QStringLiteral("description"), router_presets::builtInDescription(QString::fromUtf8(router_presets::kStrategicKey))},
            {QStringLiteral("builtIn"), true},
        },
    };

    const QVariantList customPresets = m_routerCustomPresets;
    for (const QVariant &entry : customPresets) {
        QVariantMap preset = entry.toMap();
        preset.insert(QStringLiteral("description"),
                      QStringLiteral("User-saved router preset."));
        preset.insert(QStringLiteral("builtIn"), false);
        options.append(preset);
    }

    return options;
}

QString AppState::routerPresetPrompt(const QString &presetKey) const
{
    return routerPresetDetails(presetKey).value(QStringLiteral("prompt")).toString();
}

QString AppState::routerPresetPromptPreview(const QString &presetKey,
                                            const QString &userDefaultContext) const
{
    const QString normalizedKey = normalizedRouterPresetKey(presetKey);
    if (normalizedKey == QLatin1StringView(router_presets::kStrategicKey)) {
        return router_presets::builtInPrompt(normalizedKey, userDefaultContext);
    }
    return routerPresetPrompt(normalizedKey);
}

QString AppState::routerPresetDescription(const QString &presetKey) const
{
    return routerPresetDetails(presetKey).value(QStringLiteral("description")).toString();
}

bool AppState::routerPresetIsBuiltIn(const QString &presetKey) const
{
    return routerPresetDetails(presetKey).value(QStringLiteral("builtIn")).toBool();
}

QString AppState::saveCustomRouterPreset(const QString &presetKey,
                                         const QString &name,
                                         const QString &prompt)
{
    const QString trimmedName = name.trimmed();
    const QString effectiveName = trimmedName.isEmpty()
                                      ? QStringLiteral("CUSTOM PRESET")
                                      : trimmedName;
    if (prompt.trimmed().isEmpty()) {
        return QString();
    }

    const QString trimmedKey = presetKey.trimmed().toLower();
    const int existingIndex = customRouterPresetIndex(trimmedKey);
    const QString resolvedKey =
        existingIndex >= 0 ? trimmedKey : router_presets::makeCustomPresetId();
    const QVariantMap preset{
        {QStringLiteral("key"), resolvedKey},
        {QStringLiteral("label"), effectiveName},
        {QStringLiteral("prompt"), prompt},
    };

    if (existingIndex >= 0) {
        m_routerCustomPresets[existingIndex] = preset;
    } else {
        m_routerCustomPresets.append(preset);
    }
    if (m_routerPresetKey == trimmedKey || existingIndex < 0) {
        m_routerPresetKey = resolvedKey;
    }
    persistSelectionState();
    emit routerConfigChanged();
    return resolvedKey;
}

bool AppState::deleteCustomRouterPreset(const QString &presetKey)
{
    const int existingIndex = customRouterPresetIndex(presetKey);
    if (existingIndex < 0) {
        return false;
    }

    m_routerCustomPresets.removeAt(existingIndex);
    if (m_routerPresetKey == presetKey.trimmed()) {
        m_routerPresetKey = QString::fromUtf8(router_presets::kStandardKey);
    }
    persistSelectionState();
    emit routerConfigChanged();
    return true;
}

QString AppState::effectiveRouterSysprompt() const
{
    return routerPresetDetails(m_routerPresetKey).value(QStringLiteral("prompt")).toString();
}

QString AppState::routerHermesProfileMode() const
{
    return normalizedHermesProfileMode(m_routerHermesProfileMode);
}

void AppState::setRouterHermesProfileMode(const QString &profileMode)
{
    const QString normalizedMode = normalizedHermesProfileMode(profileMode);
    if (m_routerHermesProfileMode == normalizedMode) return;
    m_routerHermesProfileMode = normalizedMode;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerHermesProfilePath() const
{
    return m_routerHermesProfilePath.trimmed().isEmpty()
               ? defaultHermesProfileTemplatePath()
               : m_routerHermesProfilePath;
}

void AppState::setRouterHermesProfilePath(const QString &profilePath)
{
    const QString trimmed = profilePath.trimmed();
    const QString normalizedPath =
        trimmed.isEmpty() ? defaultHermesProfileTemplatePath() : trimmed;
    if (m_routerHermesProfilePath == normalizedPath) return;
    m_routerHermesProfilePath = normalizedPath;
    persistSelectionState();
    emit routerConfigChanged();
}

QString AppState::routerWorkspacePath()
{
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataDir).filePath(QStringLiteral("workspaces/router"));
}

QString AppState::routerReportsPath()
{
    return QDir(routerWorkspacePath()).filePath(QStringLiteral("reports"));
}

QString AppState::sessionManifestPath()
{
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataDir).filePath(QStringLiteral("router/manifest.json"));
}

QString AppState::routerControlScriptPath()
{
    return router_presets::controlScriptPath();
}

bool AppState::stageProviderControlWorkspace(const QString &workspaceDir) const
{
    QDir dir(workspaceDir);
    if (!dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QByteArray manifestPayload = QByteArrayLiteral("{\"sessions\":[]}");
    QFile manifestFile(sessionManifestPath());
    if (manifestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray liveManifest = manifestFile.readAll();
        if (!liveManifest.trimmed().isEmpty()) {
            manifestPayload = liveManifest;
        }
    }

    const bool manifestOk = writeWorkspaceFile(dir.filePath(QStringLiteral("manifest.json")),
                                               manifestPayload,
                                               QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                                   | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    const bool controlGuideOk = writeWorkspaceFile(
        dir.filePath(QStringLiteral("HYDRA_CONTROL.md")),
        router_presets::controlGuideContents().toUtf8(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup
            | QFileDevice::ReadOther);
    const bool routerWrapperOk = writeWorkspaceFile(
        dir.filePath(QStringLiteral("hydra-router")),
        router_presets::wrapperContents().toUtf8(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
            | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther
            | QFileDevice::ExeOther);
    return manifestOk && controlGuideOk && routerWrapperOk;
}

QString AppState::defaultRouterSysprompt() const
{
    return router_presets::builtInPrompt(QString::fromUtf8(router_presets::kStandardKey), QString());
}

void AppState::writeSessionManifest() const
{
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString manifestDir = QDir(appDataDir).filePath(QStringLiteral("router"));
    QDir().mkpath(manifestDir);
    QDir().mkpath(routerReportsPath());

    const domain::SessionRecord *router = findRouterSession();
    const domain::SessionRecord *master = findMasterSession();

    QJsonArray sessionsArray;
    for (const domain::SessionRecord &session : m_sessions) {
        if (session.state == domain::SessionState::Exited) {
            continue;
        }
        if (session.category == domain::SessionCategory::Master
            || session.category == domain::SessionCategory::Router)
        {
            continue;
        }

        QJsonObject obj;
        obj[QStringLiteral("alias")] = session.alias;
        obj[QStringLiteral("name")] = session.name;
        obj[QStringLiteral("provider")] = session.providerKey;
        obj[QStringLiteral("state")] = domain::sessionStateDisplayName(session.state);
        obj[QStringLiteral("tmux_session")] = session.tmuxSessionName;
        obj[QStringLiteral("tmux_pane")] = session.tmuxPaneId;
        obj[QStringLiteral("working_directory")] = session.workingDirectory;
        obj[QStringLiteral("launch_safety")] = session.launchSafetyKey;
        obj[QStringLiteral("status_detail")] = session.statusDetail;
        obj[QStringLiteral("last_error")] = session.lastError;
        obj[QStringLiteral("updated_at")] =
            session.updatedAt.toUTC().toString(Qt::ISODateWithMs);
        if (const domain::Repository *repository = findRepository(session.repoId);
            repository != nullptr)
        {
            obj[QStringLiteral("repo_id")] = repository->id;
            obj[QStringLiteral("repo_name")] = repository->name;
        }

        const QVector<domain::SessionTimelineEvent> timeline =
            m_timelineBySession.value(session.id);
        QJsonArray recentEvents;
        const int firstIndex = std::max(0, static_cast<int>(timeline.size()) - 3);
        for (int index = firstIndex; index < timeline.size(); ++index) {
            const domain::SessionTimelineEvent &event = timeline.at(index);
            QJsonObject eventObject;
            eventObject[QStringLiteral("summary")] = event.summary;
            eventObject[QStringLiteral("detail")] = event.detail;
            eventObject[QStringLiteral("state")] =
                domain::sessionStateDisplayName(event.state);
            eventObject[QStringLiteral("created_at")] =
                event.createdAt.toUTC().toString(Qt::ISODateWithMs);
            recentEvents.append(eventObject);
        }
        obj[QStringLiteral("recent_events")] = recentEvents;
        sessionsArray.append(obj);
    }

    QJsonObject root;
    root[QStringLiteral("master_tmux_session")] =
        master != nullptr ? master->tmuxSessionName : QString();
    root[QStringLiteral("master_pane_id")] =
        master != nullptr ? master->tmuxPaneId : QString();
    root[QStringLiteral("master_provider")] =
        master != nullptr ? master->providerKey : QString();
    root[QStringLiteral("router_tmux_session")] =
        router != nullptr ? router->tmuxSessionName : QString();
    root[QStringLiteral("router_pane_id")] =
        router != nullptr ? router->tmuxPaneId : QString();
    root[QStringLiteral("router_provider")] =
        router != nullptr ? router->providerKey : QString();
    root[QStringLiteral("sessions")] = sessionsArray;

    const QByteArray manifestPayload = QJsonDocument(root).toJson(QJsonDocument::Compact);
    writeWorkspaceFile(sessionManifestPath(),
                       manifestPayload,
                       QFileDevice::ReadOwner | QFileDevice::WriteOwner
                           | QFileDevice::ReadGroup | QFileDevice::ReadOther);

    stageProviderControlWorkspace(masterWorkspacePath());
    stageProviderControlWorkspace(routerWorkspacePath());
}

}  // namespace hydra::ui
