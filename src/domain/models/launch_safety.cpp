#include "domain/models/launch_safety.hpp"

namespace hydra::domain {

QString launchSafetyKey(const LaunchSafety safety)
{
    switch (safety) {
    case LaunchSafety::WorkspaceSafe:
        return QStringLiteral("workspace-safe");
    case LaunchSafety::Bypass:
        return QStringLiteral("bypass");
    }

    return QStringLiteral("workspace-safe");
}

QString launchSafetyDisplayName(const LaunchSafety safety)
{
    switch (safety) {
    case LaunchSafety::WorkspaceSafe:
        return QStringLiteral("Sandboxed");
    case LaunchSafety::Bypass:
        return QStringLiteral("Bypass");
    }

    return QStringLiteral("Sandboxed");
}

LaunchSafety launchSafetyFromKey(const QString &key)
{
    return key == QStringLiteral("bypass") ? LaunchSafety::Bypass : LaunchSafety::WorkspaceSafe;
}

}  // namespace hydra::domain
