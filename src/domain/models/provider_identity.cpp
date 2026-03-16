#include "domain/models/provider_identity.hpp"

namespace hydra::domain {

QString providerDisplayNameForKey(const QString &providerKey)
{
    if (providerKey == QStringLiteral("codex")) {
        return QStringLiteral("Codex");
    }
    if (providerKey == QStringLiteral("gemini")) {
        return QStringLiteral("Gemini");
    }
    if (providerKey == QStringLiteral("claude")) {
        return QStringLiteral("Claude Code");
    }
    if (providerKey == QStringLiteral("hermes")) {
        return QStringLiteral("Hermes");
    }
    if (providerKey == QStringLiteral("opencode")) {
        return QStringLiteral("OpenCode");
    }
    return providerKey;
}

}  // namespace hydra::domain
