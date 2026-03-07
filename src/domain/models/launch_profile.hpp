#pragma once

#include <QString>

namespace hydra::domain {

struct LaunchProfile {
    QString id;
    QString name;
    QString providerKey;
    QString riskProfileKey;
    bool useWorktree = false;
    bool networkEnabled = false;
};

}  // namespace hydra::domain
