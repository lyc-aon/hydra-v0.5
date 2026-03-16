#include "infrastructure/providers/gemini_session_resolver.hpp"
#include "infrastructure/providers/provider_state_paths.hpp"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>

namespace hydra::infrastructure {

namespace {

QString sharedTmpDirectoryPath()
{
    return QDir(GeminiSessionResolver::sharedGeminiDirectoryPath())
        .filePath(QStringLiteral("tmp"));
}

}

GeminiSessionResolver::GeminiSessionResolver(QDir tmpDirectory)
    : m_tmpDirectory(std::move(tmpDirectory))
{
}

QString GeminiSessionResolver::providerKey() const
{
    return QStringLiteral("gemini");
}

QString GeminiSessionResolver::sharedGeminiCliHomePath()
{
    const QString configuredSharedHome =
        qEnvironmentVariable("HYDRA_GEMINI_SHARED_HOME").trimmed();
    if (!configuredSharedHome.isEmpty()) {
        return QDir::cleanPath(configuredSharedHome);
    }

    const QString inheritedGeminiCliHome =
        qEnvironmentVariable("GEMINI_CLI_HOME").trimmed();
    if (!inheritedGeminiCliHome.isEmpty()) {
        return QDir::cleanPath(inheritedGeminiCliHome);
    }

    return QDir::cleanPath(QDir::homePath());
}

QString GeminiSessionResolver::sharedGeminiDirectoryPath()
{
    return QDir(sharedGeminiCliHomePath()).filePath(QStringLiteral(".gemini"));
}

QString GeminiSessionResolver::stateRootPath()
{
    return providerStateRootPath(QStringLiteral("gemini"));
}

QString GeminiSessionResolver::workspaceSlug(const QString &normalizedWorkingDirectory)
{
    QFileInfo info(normalizedWorkingDirectory);
    QString baseName = info.fileName().trimmed();
    if (baseName.isEmpty()) {
        baseName = QStringLiteral("workspace");
    }

    baseName.replace(QRegularExpression(QStringLiteral(R"([^A-Za-z0-9._-]+)")),
                     QStringLiteral("-"));
    if (baseName.size() > 32) {
        baseName = baseName.left(32);
    }

    const QByteArray hash = QCryptographicHash::hash(normalizedWorkingDirectory.toUtf8(),
                                                     QCryptographicHash::Sha1)
                                .toHex()
                                .left(12);
    return QStringLiteral("%1-%2").arg(baseName, QString::fromLatin1(hash));
}

QString GeminiSessionResolver::geminiCliHomePathForWorkingDirectory(const QString &workingDirectory)
{
    const QString normalized = normalizeWorkingDirectory(workingDirectory);
    if (normalized.isEmpty()) {
        return {};
    }

    return QDir(stateRootPath()).filePath(workspaceSlug(normalized));
}

QString GeminiSessionResolver::tmpDirectoryForWorkingDirectory(const QString &workingDirectory)
{
    const QString geminiCliHome = geminiCliHomePathForWorkingDirectory(workingDirectory);
    if (geminiCliHome.isEmpty()) {
        return sharedTmpDirectoryPath();
    }

    return QDir(geminiCliHome).filePath(QStringLiteral(".gemini/tmp"));
}

QSet<QString> GeminiSessionResolver::knownSessionIdsForWorkingDirectory(
    const QString &workingDirectory) const
{
    QSet<QString> sessionIds;
    for (const SessionMeta &session : listSessionsForWorkingDirectory(workingDirectory)) {
        sessionIds.insert(session.sessionId);
    }
    return sessionIds;
}

QString GeminiSessionResolver::findNewSessionId(const QString &workingDirectory,
                                                const QSet<QString> &knownSessionIds,
                                                const QSet<QString> &excludedSessionIds) const
{
    for (const SessionMeta &session : listSessionsForWorkingDirectory(workingDirectory)) {
        if (knownSessionIds.contains(session.sessionId) || excludedSessionIds.contains(session.sessionId)) {
            continue;
        }
        return session.sessionId;
    }
    return {};
}

bool GeminiSessionResolver::sessionExists(const QString &sessionId,
                                          const QString &workingDirectory) const
{
    const QString trimmedSessionId = sessionId.trimmed();
    if (trimmedSessionId.isEmpty()) {
        return false;
    }

    const QList<SessionMeta> sessions = workingDirectory.trimmed().isEmpty()
                                            ? listAllSessions()
                                            : listSessionsForWorkingDirectory(workingDirectory);
    for (const SessionMeta &session : sessions) {
        if (session.sessionId == trimmedSessionId) {
            return true;
        }
    }
    return false;
}

QString GeminiSessionResolver::normalizeWorkingDirectory(const QString &workingDirectory)
{
    const QString trimmed = workingDirectory.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QFileInfo info(trimmed);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QString GeminiSessionResolver::projectWorkingDirectory(const QString &projectPath) const
{
    QFile file(projectPath + QStringLiteral("/.project_root"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString raw = QString::fromUtf8(file.readAll()).trimmed();
    return normalizeWorkingDirectory(raw);
}

bool GeminiSessionResolver::usesSharedHomeLayout() const
{
    return QDir::cleanPath(m_tmpDirectory.absolutePath())
           == QDir::cleanPath(sharedTmpDirectoryPath());
}

QList<GeminiSessionResolver::SessionMeta> GeminiSessionResolver::listSessionsForWorkingDirectory(
    const QString &workingDirectory) const
{
    const QString target = normalizeWorkingDirectory(workingDirectory);
    if (target.isEmpty()) {
        return {};
    }

    QList<SessionMeta> sessions;
    for (const SessionMeta &session : listAllSessions()) {
        if (normalizeWorkingDirectory(session.workingDirectory) == target) {
            sessions.push_back(session);
        }
    }
    return sessions;
}

QList<GeminiSessionResolver::SessionMeta> GeminiSessionResolver::listAllSessions() const
{
    QList<SessionMeta> sessions;
    const QStringList tmpDirectories = usesSharedHomeLayout()
                                           ? ([&]() {
                                                 QStringList dirs;
                                                 const QString sharedTmp = sharedTmpDirectoryPath();
                                                 if (QDir(sharedTmp).exists()) {
                                                     dirs.push_back(sharedTmp);
                                                 }

                                                 QDir stateRoot(stateRootPath());
                                                 if (stateRoot.exists()) {
                                                     const QFileInfoList workspaceHomes =
                                                         stateRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                                                 QDir::Time);
                                                     for (const QFileInfo &workspaceHome : workspaceHomes) {
                                                         const QString tmpPath =
                                                             QDir(workspaceHome.absoluteFilePath())
                                                                 .filePath(QStringLiteral(".gemini/tmp"));
                                                         if (!dirs.contains(tmpPath) && QDir(tmpPath).exists()) {
                                                             dirs.push_back(tmpPath);
                                                         }
                                                     }
                                                 }

                                                 return dirs;
                                             })()
                                           : QStringList{m_tmpDirectory.absolutePath()};

    for (const QString &tmpDirectoryPath : tmpDirectories) {
        QDir tmpDirectory(tmpDirectoryPath);
        if (!tmpDirectory.exists()) {
            continue;
        }

        const QFileInfoList projectDirectories =
            tmpDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
        for (const QFileInfo &projectDirectory : projectDirectories) {
            const QString workingDirectory = projectWorkingDirectory(projectDirectory.absoluteFilePath());
            if (workingDirectory.isEmpty()) {
                continue;
            }

            QDir chatsDirectory(projectDirectory.absoluteFilePath() + QStringLiteral("/chats"));
            if (!chatsDirectory.exists()) {
                continue;
            }

            const QFileInfoList sessionFiles =
                chatsDirectory.entryInfoList({QStringLiteral("session-*.json")}, QDir::Files, QDir::Time);
            for (const QFileInfo &sessionFile : sessionFiles) {
                const SessionMeta meta = readSessionMeta(sessionFile.absoluteFilePath(), workingDirectory);
                if (!meta.sessionId.isEmpty()) {
                    sessions.push_back(meta);
                }
            }
        }
    }

    std::sort(sessions.begin(), sessions.end(), [](const SessionMeta &lhs, const SessionMeta &rhs) {
        return lhs.mtimeMs > rhs.mtimeMs;
    });

    QHash<QString, SessionMeta> newestBySessionId;
    for (const SessionMeta &session : sessions) {
        auto existing = newestBySessionId.constFind(session.sessionId);
        if (existing == newestBySessionId.cend() || existing->mtimeMs < session.mtimeMs) {
            newestBySessionId.insert(session.sessionId, session);
        }
    }

    QList<SessionMeta> deduped = newestBySessionId.values();
    std::sort(deduped.begin(), deduped.end(), [](const SessionMeta &lhs, const SessionMeta &rhs) {
        return lhs.mtimeMs > rhs.mtimeMs;
    });
    return deduped;
}

GeminiSessionResolver::SessionMeta GeminiSessionResolver::readSessionMeta(const QString &filePath,
                                                                          const QString &workingDirectory)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return {};
    }

    const QString sessionId = document.object().value(QStringLiteral("sessionId")).toString().trimmed();
    if (sessionId.isEmpty()) {
        return {};
    }

    return {
        .sessionId = sessionId,
        .workingDirectory = workingDirectory,
        .filePath = filePath,
        .mtimeMs = QFileInfo(file).lastModified().toMSecsSinceEpoch(),
    };
}

}  // namespace hydra::infrastructure
