#pragma once

#include <QString>

#include "domain/ports/mux_adapter.hpp"
#include "infrastructure/process/process_runner.hpp"

namespace hydra::infrastructure {

class TmuxAdapter final : public domain::ports::MuxAdapter {
public:
    TmuxAdapter();

    domain::ports::MuxProbeResult probeAvailability() const override;
    domain::ports::MuxLaunchResult launchDetachedSession(
        const domain::ports::MuxLaunchRequest &request) override;
    domain::ports::MuxTerminateResult terminateSession(const QString &sessionName) const override;
    bool hasLiveSession(const QString &sessionName) const override;

private:
    using CommandResult = process::ProcessResult;

    CommandResult runTmuxCommand(const QStringList &arguments) const;

    QString m_tmuxPath;
};

}  // namespace hydra::infrastructure
