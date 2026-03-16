#pragma once

#include <QSet>
#include <QString>

#include "domain/ports/provider_session_resolver.hpp"

namespace hydra::infrastructure {

class OpenCodeSessionResolver final : public domain::ports::ProviderSessionResolver {
public:
    struct SessionMeta {
        QString sessionId;
        QString workingDirectory;
        qint64 updatedAtMs = 0;
    };

    QString providerKey() const override;
    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override;
    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds = {}) const override;
    bool sessionExists(const QString &sessionId,
                       const QString &workingDirectory = QString()) const override;

    static QString workspaceRootPathForWorkingDirectory(const QString &workingDirectory);
    static QString xdgDataHomePathForWorkingDirectory(const QString &workingDirectory);
    static QString xdgStateHomePathForWorkingDirectory(const QString &workingDirectory);
    static QString xdgCacheHomePathForWorkingDirectory(const QString &workingDirectory);

private:
    static QString normalizeWorkingDirectory(const QString &workingDirectory);
    static QList<SessionMeta> listSessionsForWorkingDirectory(const QString &workingDirectory);
    static QList<SessionMeta> listAllSessions();
    static QString stateRootPath();
    static QString workspaceSlug(const QString &normalizedWorkingDirectory);
};

}  // namespace hydra::infrastructure
