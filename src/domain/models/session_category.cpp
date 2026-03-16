#include "domain/models/session_category.hpp"

namespace hydra::domain {

QString sessionCategoryToKey(const SessionCategory cat)
{
    switch (cat) {
    case SessionCategory::Worker:
        return QStringLiteral("worker");
    case SessionCategory::Master:
        return QStringLiteral("master");
    case SessionCategory::Router:
        return QStringLiteral("router");
    }

    return QStringLiteral("worker");
}

SessionCategory sessionCategoryFromKey(const QString &key)
{
    if (key == QStringLiteral("master")) {
        return SessionCategory::Master;
    }
    if (key == QStringLiteral("router")) {
        return SessionCategory::Router;
    }
    return SessionCategory::Worker;
}

QString sessionCategoryDisplayName(const SessionCategory cat)
{
    switch (cat) {
    case SessionCategory::Worker:
        return QStringLiteral("Worker");
    case SessionCategory::Master:
        return QStringLiteral("Master");
    case SessionCategory::Router:
        return QStringLiteral("Router");
    }

    return QStringLiteral("Worker");
}

}  // namespace hydra::domain
