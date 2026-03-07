#pragma once

#include <QString>

namespace hydra::domain::ports {

struct MuxProbeResult {
    bool available = false;
    QString executablePath;
    QString message;
};

struct MuxLaunchRequest {
    QString sessionName;
    QString workingDirectory;
    QString shellCommand;
};

struct MuxLaunchResult {
    bool ok = false;
    QString sessionName;
    QString paneId;
    QString errorMessage;
};

struct MuxTerminateResult {
    bool ok = false;
    QString errorMessage;
};

class MuxAdapter {
public:
    virtual ~MuxAdapter() = default;

    virtual MuxProbeResult probeAvailability() const = 0;
    virtual MuxLaunchResult launchDetachedSession(const MuxLaunchRequest &request) = 0;
    virtual MuxTerminateResult terminateSession(const QString &sessionName) const = 0;
    virtual bool hasLiveSession(const QString &sessionName) const = 0;
};

}  // namespace hydra::domain::ports
