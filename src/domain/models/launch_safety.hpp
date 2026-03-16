#pragma once

#include <QString>

namespace hydra::domain {

enum class LaunchSafety {
    WorkspaceSafe,
    Bypass,
};

QString launchSafetyKey(LaunchSafety safety);
QString launchSafetyDisplayName(LaunchSafety safety);
LaunchSafety launchSafetyFromKey(const QString &key);

}  // namespace hydra::domain
