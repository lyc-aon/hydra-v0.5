#pragma once

#include <QSet>
#include <QString>

#include "domain/ports/provider_session_resolver.hpp"

namespace hydra::infrastructure {

class HermesSessionResolver final : public domain::ports::ProviderSessionResolver {
public:
    explicit HermesSessionResolver(QString databasePath = QString());

    QString providerKey() const override;
    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override;
    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds = {}) const override;
    bool sessionExists(const QString &sessionId,
                       const QString &workingDirectory = QString()) const override;

private:
    QString databasePath() const;
    QList<QString> listManagedResumeTokens() const;

    QString m_databasePath;
};

}  // namespace hydra::infrastructure
