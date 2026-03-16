#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

#include "domain/models/launch_safety.hpp"
#include "domain/models/session_category.hpp"
#include "domain/models/session_state.hpp"

namespace hydra::domain::ports {

struct ProviderProbe {
    QString providerKey;
    QString displayName;
    bool available = false;
    QString executablePath;
    QString versionString;
    QString statusMessage;
};

struct ProviderLaunchPlan {
    QString executable;
    QStringList arguments;
    QHash<QString, QString> environment;
};

struct ProviderInvocationOptions {
    QString resumeToken;
    QString modelOverride;
    domain::SessionCategory sessionCategory = domain::SessionCategory::Worker;
    QString providerProfileMode;
    QString providerProfilePath;
    QString workingDirectory;
};

struct ProviderObservabilityHints {
    QStringList promptPatterns;
    QStringList promptTitlePatterns;
    QStringList approvalPatterns;
    QStringList waitingInputPatterns;
    QStringList activePatterns;
    QStringList approvalTitlePatterns;
    QStringList waitingTitlePatterns;
    QStringList activeTitlePatterns;
    QStringList ignoredTailPatterns;
    bool restrictActiveTailToPromptSegment = false;
    bool allowGenericActiveTailFallback = true;
    bool preferRecentInputHoldBeforeTailSignals = false;
    bool preferActiveTitleDuringRecentInputHold = false;
    int recentInputStateHoldMs = 2500;
    domain::SessionState activeState = domain::SessionState::RunningTool;
};

class ProviderAdapter {
public:
    virtual ~ProviderAdapter() = default;

    virtual QString providerKey() const = 0;
    virtual QString displayName() const = 0;
    virtual ProviderProbe probe() const = 0;
    virtual bool supportsResume() const { return false; }
    virtual bool supportsModelOverride() const { return false; }
    virtual QString defaultApiKeyEnvironmentVariable() const { return {}; }
    virtual QString initialResumeToken() const { return {}; }
    virtual QString expectedResolvedResumeToken(const QString &initialResumeToken) const
    {
        Q_UNUSED(initialResumeToken);
        return {};
    }
    virtual ProviderLaunchPlan buildLaunchPlan(
        domain::LaunchSafety safety,
        ProviderInvocationOptions options = {}) const = 0;
    virtual ProviderLaunchPlan buildResumePlan(
        domain::LaunchSafety safety,
        ProviderInvocationOptions options) const
    {
        Q_UNUSED(safety);
        Q_UNUSED(options);
        return {};
    }
    virtual QString launchSafetySummary(domain::LaunchSafety safety) const = 0;
    virtual ProviderObservabilityHints observabilityHints() const = 0;
    virtual QString postLaunchBootstrapText(ProviderInvocationOptions options) const
    {
        Q_UNUSED(options);
        return {};
    }
    virtual QHash<QString, QString> promptEnvironment(const QString &promptText) const
    {
        Q_UNUSED(promptText);
        return {};
    }
    virtual bool prepareSystemPromptWorkspace(const QString &workspaceDir,
                                              const QString &promptText) const
    {
        Q_UNUSED(workspaceDir);
        Q_UNUSED(promptText);
        return false;
    }
};

}  // namespace hydra::domain::ports
