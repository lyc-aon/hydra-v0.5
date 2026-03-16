#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace hydra::ui {

class AppState;

class MasterState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY stateChanged)
    Q_PROPERTY(QString sessionName READ sessionName NOTIFY stateChanged)
    Q_PROPERTY(QString sessionProviderKey READ sessionProviderKey NOTIFY stateChanged)
    Q_PROPERTY(QString sessionTmuxSessionName READ sessionTmuxSessionName NOTIFY stateChanged)
    Q_PROPERTY(QString sessionPaneId READ sessionPaneId NOTIFY stateChanged)
    Q_PROPERTY(QString sessionWorkingDirectory READ sessionWorkingDirectory NOTIFY stateChanged)
    Q_PROPERTY(QString sessionAlias READ sessionAlias NOTIFY stateChanged)
    Q_PROPERTY(bool sessionAvailable READ sessionAvailable NOTIFY stateChanged)
    Q_PROPERTY(QString sessionStateTone READ sessionStateTone NOTIFY stateChanged)
    Q_PROPERTY(QString sessionActivityLabel READ sessionActivityLabel NOTIFY stateChanged)
    Q_PROPERTY(bool sessionApprovalPending READ sessionApprovalPending NOTIFY stateChanged)
    Q_PROPERTY(QString providerKey READ providerKey WRITE setProviderKey NOTIFY stateChanged)
    Q_PROPERTY(QString launchSafetyKey READ launchSafetyKey WRITE setLaunchSafetyKey NOTIFY stateChanged)
    Q_PROPERTY(QString modelId READ modelId WRITE setModelId NOTIFY stateChanged)
    Q_PROPERTY(QString apiKeyEnvVarName READ apiKeyEnvVarName WRITE setApiKeyEnvVarName NOTIFY stateChanged)
    Q_PROPERTY(QString apiKeyValue READ apiKeyValue WRITE setApiKeyValue NOTIFY stateChanged)
    Q_PROPERTY(QString sysprompt READ sysprompt WRITE setSysprompt NOTIFY stateChanged)
    Q_PROPERTY(QString hermesProfileMode READ hermesProfileMode WRITE setHermesProfileMode NOTIFY stateChanged)
    Q_PROPERTY(QString hermesProfilePath READ hermesProfilePath WRITE setHermesProfilePath NOTIFY stateChanged)

public:
    explicit MasterState(AppState *appState, QObject *parent = nullptr);

    QString sessionId() const;
    QString sessionName() const;
    QString sessionProviderKey() const;
    QString sessionTmuxSessionName() const;
    QString sessionPaneId() const;
    QString sessionWorkingDirectory() const;
    QString sessionAlias() const;
    bool sessionAvailable() const;
    QString sessionStateTone() const;
    QString sessionActivityLabel() const;
    bool sessionApprovalPending() const;
    QString providerKey() const;
    void setProviderKey(const QString &providerKey);
    QString launchSafetyKey() const;
    void setLaunchSafetyKey(const QString &safetyKey);
    QString modelId() const;
    void setModelId(const QString &modelId);
    QString apiKeyEnvVarName() const;
    void setApiKeyEnvVarName(const QString &envVarName);
    QString apiKeyValue() const;
    void setApiKeyValue(const QString &apiKeyValue);
    QString sysprompt() const;
    void setSysprompt(const QString &sysprompt);
    QString hermesProfileMode() const;
    void setHermesProfileMode(const QString &profileMode);
    QString hermesProfilePath() const;
    void setHermesProfilePath(const QString &profilePath);

    Q_INVOKABLE void ensureSession();
    Q_INVOKABLE QString defaultSysprompt() const;
    Q_INVOKABLE void applyConfig(const QString &providerKey,
                                 const QString &safetyKey,
                                 const QString &sysprompt,
                                 const QString &modelId = QString(),
                                 const QString &apiKeyEnvVarName = QString(),
                                 const QString &apiKeyValue = QString(),
                                 const QString &hermesProfileMode = QString(),
                                 const QString &hermesProfilePath = QString());
    Q_INVOKABLE void relaunchSession();

signals:
    void stateChanged();

private:
    AppState *m_appState;
};

class RouterState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY stateChanged)
    Q_PROPERTY(QString sessionName READ sessionName NOTIFY stateChanged)
    Q_PROPERTY(QString sessionProviderKey READ sessionProviderKey NOTIFY stateChanged)
    Q_PROPERTY(QString sessionTmuxSessionName READ sessionTmuxSessionName NOTIFY stateChanged)
    Q_PROPERTY(QString sessionPaneId READ sessionPaneId NOTIFY stateChanged)
    Q_PROPERTY(QString sessionWorkingDirectory READ sessionWorkingDirectory NOTIFY stateChanged)
    Q_PROPERTY(QString sessionAlias READ sessionAlias NOTIFY stateChanged)
    Q_PROPERTY(bool sessionAvailable READ sessionAvailable NOTIFY stateChanged)
    Q_PROPERTY(QString sessionStateTone READ sessionStateTone NOTIFY stateChanged)
    Q_PROPERTY(QString sessionActivityLabel READ sessionActivityLabel NOTIFY stateChanged)
    Q_PROPERTY(bool sessionApprovalPending READ sessionApprovalPending NOTIFY stateChanged)
    Q_PROPERTY(QString providerKey READ providerKey WRITE setProviderKey NOTIFY stateChanged)
    Q_PROPERTY(QString launchSafetyKey READ launchSafetyKey WRITE setLaunchSafetyKey NOTIFY stateChanged)
    Q_PROPERTY(QString modelId READ modelId WRITE setModelId NOTIFY stateChanged)
    Q_PROPERTY(QString apiKeyEnvVarName READ apiKeyEnvVarName WRITE setApiKeyEnvVarName NOTIFY stateChanged)
    Q_PROPERTY(QString apiKeyValue READ apiKeyValue WRITE setApiKeyValue NOTIFY stateChanged)
    Q_PROPERTY(QString presetKey READ presetKey WRITE setPresetKey NOTIFY stateChanged)
    Q_PROPERTY(QString userDefaultContext READ userDefaultContext WRITE setUserDefaultContext NOTIFY stateChanged)
    Q_PROPERTY(QVariantList presetOptions READ presetOptions NOTIFY stateChanged)
    Q_PROPERTY(QString hermesProfileMode READ hermesProfileMode WRITE setHermesProfileMode NOTIFY stateChanged)
    Q_PROPERTY(QString hermesProfilePath READ hermesProfilePath WRITE setHermesProfilePath NOTIFY stateChanged)

public:
    explicit RouterState(AppState *appState, QObject *parent = nullptr);

    QString sessionId() const;
    QString sessionName() const;
    QString sessionProviderKey() const;
    QString sessionTmuxSessionName() const;
    QString sessionPaneId() const;
    QString sessionWorkingDirectory() const;
    QString sessionAlias() const;
    bool sessionAvailable() const;
    QString sessionStateTone() const;
    QString sessionActivityLabel() const;
    bool sessionApprovalPending() const;
    QString providerKey() const;
    void setProviderKey(const QString &providerKey);
    QString launchSafetyKey() const;
    void setLaunchSafetyKey(const QString &safetyKey);
    QString modelId() const;
    void setModelId(const QString &modelId);
    QString apiKeyEnvVarName() const;
    void setApiKeyEnvVarName(const QString &envVarName);
    QString apiKeyValue() const;
    void setApiKeyValue(const QString &apiKeyValue);
    QString presetKey() const;
    void setPresetKey(const QString &presetKey);
    QString userDefaultContext() const;
    void setUserDefaultContext(const QString &defaultContext);
    QVariantList presetOptions() const;
    QString hermesProfileMode() const;
    void setHermesProfileMode(const QString &profileMode);
    QString hermesProfilePath() const;
    void setHermesProfilePath(const QString &profilePath);

    Q_INVOKABLE QString presetPrompt(const QString &presetKey) const;
    Q_INVOKABLE QString presetPromptPreview(const QString &presetKey,
                                            const QString &userDefaultContext) const;
    Q_INVOKABLE QString presetDescription(const QString &presetKey) const;
    Q_INVOKABLE bool presetIsBuiltIn(const QString &presetKey) const;
    Q_INVOKABLE QString saveCustomPreset(const QString &presetKey,
                                         const QString &name,
                                         const QString &prompt);
    Q_INVOKABLE bool deleteCustomPreset(const QString &presetKey);
    Q_INVOKABLE void ensureSession();
    Q_INVOKABLE void applyConfig(const QString &providerKey,
                                 const QString &safetyKey,
                                 const QString &presetKey = QString(),
                                 const QString &userDefaultContext = QString(),
                                 const QString &modelId = QString(),
                                 const QString &apiKeyEnvVarName = QString(),
                                 const QString &apiKeyValue = QString(),
                                 const QString &hermesProfileMode = QString(),
                                 const QString &hermesProfilePath = QString());
    Q_INVOKABLE void relaunchSession();

signals:
    void stateChanged();

private:
    AppState *m_appState;
};

}  // namespace hydra::ui
