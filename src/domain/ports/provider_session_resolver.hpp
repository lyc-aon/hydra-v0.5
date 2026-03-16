#pragma once

#include <QSet>
#include <QString>

namespace hydra::domain::ports {

class ProviderSessionResolver {
public:
    virtual ~ProviderSessionResolver() = default;

    virtual QString providerKey() const = 0;
    virtual QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const = 0;
    virtual QString findNewSessionId(const QString &workingDirectory,
                                     const QSet<QString> &knownSessionIds,
                                     const QSet<QString> &excludedSessionIds = {}) const = 0;
    virtual bool sessionExists(const QString &sessionId,
                               const QString &workingDirectory = QString()) const = 0;
};

}  // namespace hydra::domain::ports
