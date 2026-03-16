#pragma once

#include <QDir>

#include "domain/ports/provider_session_resolver.hpp"

namespace hydra::infrastructure {

class GeminiSessionResolver final : public domain::ports::ProviderSessionResolver {
public:
    explicit GeminiSessionResolver(QDir tmpDirectory = QDir(QDir::homePath() + QStringLiteral("/.gemini/tmp")));

    QString providerKey() const override;
    QSet<QString> knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const override;
    QString findNewSessionId(const QString &workingDirectory,
                             const QSet<QString> &knownSessionIds,
                             const QSet<QString> &excludedSessionIds = {}) const override;
    bool sessionExists(const QString &sessionId,
                       const QString &workingDirectory = QString()) const override;

    static QString sharedGeminiCliHomePath();
    static QString sharedGeminiDirectoryPath();
    static QString geminiCliHomePathForWorkingDirectory(const QString &workingDirectory);
    static QString tmpDirectoryForWorkingDirectory(const QString &workingDirectory);

private:
    struct SessionMeta {
        QString sessionId;
        QString workingDirectory;
        QString filePath;
        qint64 mtimeMs = 0;
    };

    static QString normalizeWorkingDirectory(const QString &workingDirectory);
    QString projectWorkingDirectory(const QString &projectPath) const;
    QList<SessionMeta> listSessionsForWorkingDirectory(const QString &workingDirectory) const;
    QList<SessionMeta> listAllSessions() const;
    static SessionMeta readSessionMeta(const QString &filePath, const QString &workingDirectory);
    static QString stateRootPath();
    static QString workspaceSlug(const QString &normalizedWorkingDirectory);
    bool usesSharedHomeLayout() const;

    QDir m_tmpDirectory;
};

}  // namespace hydra::infrastructure
