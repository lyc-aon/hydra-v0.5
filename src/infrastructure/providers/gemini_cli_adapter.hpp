#pragma once

#include "domain/ports/provider_adapter.hpp"

namespace hydra::infrastructure {

class GeminiCliAdapter final : public domain::ports::ProviderAdapter {
public:
    QString providerKey() const override;
    QString displayName() const override;
    domain::ports::ProviderProbe probe() const override;
    bool supportsResume() const override;
    bool supportsModelOverride() const override;
    QString defaultApiKeyEnvironmentVariable() const override;
    QString initialResumeToken() const override;
    domain::ports::ProviderLaunchPlan buildLaunchPlan(
        domain::LaunchSafety safety,
        domain::ports::ProviderInvocationOptions options = {}) const override;
    domain::ports::ProviderLaunchPlan buildResumePlan(
        domain::LaunchSafety safety,
        domain::ports::ProviderInvocationOptions options) const override;
    QString launchSafetySummary(domain::LaunchSafety safety) const override;
    domain::ports::ProviderObservabilityHints observabilityHints() const override;
    bool prepareSystemPromptWorkspace(const QString &workspaceDir,
                                      const QString &promptText) const override;
};

}  // namespace hydra::infrastructure
