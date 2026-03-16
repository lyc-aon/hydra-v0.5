#pragma once

#include <QString>

namespace hydra::domain {

enum class SessionCategory {
    Worker,    // Normal agent/shell sessions (default)
    Master,    // The orchestrator terminal (Phase 2)
    Router     // Invisible routing agent (Phase 3)
};

QString sessionCategoryToKey(SessionCategory cat);
SessionCategory sessionCategoryFromKey(const QString &key);
QString sessionCategoryDisplayName(SessionCategory cat);

}  // namespace hydra::domain
