#pragma once

#include "domain/ports/provider_session_resolver.hpp"

namespace hydra::infrastructure {

class CodexSessionResolver final : public domain::ports::ProviderSessionResolver {
public:
    QString providerKey() const override;
    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override;
    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds = {}) const override;
    bool sessionExists(const QString &sessionId,
                       const QString &workingDirectory = QString()) const override;

    static QString sharedCodexHomePath();
    static QString codexHomePathForWorkingDirectory(const QString &workingDirectory);
    static QString sessionsDirectoryForWorkingDirectory(const QString &workingDirectory);

private:
    struct SessionMeta {
        QString sessionId;
        QString workingDirectory;
        QString filePath;
        qint64 mtimeMs = 0;
    };

    static QString normalizeWorkingDirectory(const QString &workingDirectory);
    static QList<SessionMeta> listSessions(const QString &workingDirectory);
    static SessionMeta readSessionMeta(const QString &filePath);
};

}  // namespace hydra::infrastructure
