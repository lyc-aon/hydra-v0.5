#include "infrastructure/providers/codex_session_resolver.hpp"
#include "infrastructure/providers/provider_state_paths.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>

namespace hydra::infrastructure {

namespace {

QString codexStateRootPath()
{
    return providerStateRootPath(QStringLiteral("codex"));
}

QString codexHomeSlug(const QString &normalizedWorkingDirectory)
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

QString sharedSessionsDirectoryPath()
{
    return QDir(CodexSessionResolver::sharedCodexHomePath()).filePath(QStringLiteral("sessions"));
}

QStringList sessionDirectoryCandidates(const QString &workingDirectory)
{
    QStringList candidates;

    const QString normalized = CodexSessionResolver::codexHomePathForWorkingDirectory(workingDirectory);
    if (!normalized.isEmpty()) {
        const QString isolatedSessions =
            QDir(normalized).filePath(QStringLiteral("sessions"));
        if (QDir(isolatedSessions).exists()) {
            candidates.push_back(isolatedSessions);
        }
    }

    if (candidates.isEmpty()) {
        candidates.push_back(sharedSessionsDirectoryPath());
    }

    return candidates;
}

}  // namespace

QString CodexSessionResolver::providerKey() const
{
    return QStringLiteral("codex");
}

QString CodexSessionResolver::sharedCodexHomePath()
{
    const QString configuredSharedHome =
        qEnvironmentVariable("HYDRA_CODEX_SHARED_HOME").trimmed();
    if (!configuredSharedHome.isEmpty()) {
        return QDir::cleanPath(configuredSharedHome);
    }

    const QString inheritedCodexHome = qEnvironmentVariable("CODEX_HOME").trimmed();
    if (!inheritedCodexHome.isEmpty()) {
        return QDir::cleanPath(inheritedCodexHome);
    }

    return QDir::cleanPath(QDir::homePath() + QStringLiteral("/.codex"));
}

QString CodexSessionResolver::codexHomePathForWorkingDirectory(const QString &workingDirectory)
{
    const QString normalized = normalizeWorkingDirectory(workingDirectory);
    if (normalized.isEmpty()) {
        return {};
    }

    return QDir(codexStateRootPath()).filePath(codexHomeSlug(normalized));
}

QString CodexSessionResolver::sessionsDirectoryForWorkingDirectory(const QString &workingDirectory)
{
    const QString codexHome = codexHomePathForWorkingDirectory(workingDirectory);
    if (codexHome.isEmpty()) {
        return sharedSessionsDirectoryPath();
    }

    return QDir(codexHome).filePath(QStringLiteral("sessions"));
}

QSet<QString> CodexSessionResolver::knownSessionIdsForWorkingDirectory(
    const QString &workingDirectory) const
{
    const QString target = normalizeWorkingDirectory(workingDirectory);
    QSet<QString> sessionIds;
    for (const SessionMeta &session : listSessions(workingDirectory)) {
        if (normalizeWorkingDirectory(session.workingDirectory) == target) {
            sessionIds.insert(session.sessionId);
        }
    }
    return sessionIds;
}

QString CodexSessionResolver::findNewSessionId(const QString &workingDirectory,
                                               const QSet<QString> &knownSessionIds,
                                               const QSet<QString> &excludedSessionIds) const
{
    const QString target = normalizeWorkingDirectory(workingDirectory);
    for (const SessionMeta &session : listSessions(workingDirectory)) {
        if (normalizeWorkingDirectory(session.workingDirectory) != target) {
            continue;
        }
        if (knownSessionIds.contains(session.sessionId) || excludedSessionIds.contains(session.sessionId)) {
            continue;
        }
        return session.sessionId;
    }
    return {};
}

bool CodexSessionResolver::sessionExists(const QString &sessionId,
                                         const QString &workingDirectory) const
{
    const QString trimmedSessionId = sessionId.trimmed();
    if (trimmedSessionId.isEmpty()) {
        return false;
    }

    const QString target = normalizeWorkingDirectory(workingDirectory);
    for (const SessionMeta &session : listSessions(workingDirectory)) {
        if (session.sessionId != trimmedSessionId) {
            continue;
        }
        if (target.isEmpty() || normalizeWorkingDirectory(session.workingDirectory) == target) {
            return true;
        }
    }
    return false;
}

QString CodexSessionResolver::normalizeWorkingDirectory(const QString &workingDirectory)
{
    const QString trimmed = workingDirectory.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QFileInfo info(trimmed);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QList<CodexSessionResolver::SessionMeta> CodexSessionResolver::listSessions(
    const QString &workingDirectory)
{
    QList<SessionMeta> sessions;
    for (const QString &sessionsDirectory : sessionDirectoryCandidates(workingDirectory)) {
        const QDir directory(sessionsDirectory);
        if (!directory.exists()) {
            continue;
        }

        QDirIterator it(directory.absolutePath(),
                        {QStringLiteral("*.jsonl")},
                        QDir::Files,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString path = it.next();
            const SessionMeta meta = readSessionMeta(path);
            if (!meta.sessionId.isEmpty() && !meta.workingDirectory.isEmpty()) {
                sessions.push_back(meta);
            }
        }
    }

    std::sort(sessions.begin(), sessions.end(), [](const SessionMeta &lhs, const SessionMeta &rhs) {
        return lhs.mtimeMs > rhs.mtimeMs;
    });
    return sessions;
}

CodexSessionResolver::SessionMeta CodexSessionResolver::readSessionMeta(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    const QString firstLine = stream.readLine().trimmed();
    if (firstLine.isEmpty()) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(firstLine.toUtf8());
    if (!document.isObject()) {
        return {};
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("type")).toString() != QStringLiteral("session_meta")) {
        return {};
    }

    const QJsonObject payload = object.value(QStringLiteral("payload")).toObject();
    const QString sessionId = payload.value(QStringLiteral("id")).toString().trimmed();
    const QString workingDirectory = payload.value(QStringLiteral("cwd")).toString().trimmed();
    if (sessionId.isEmpty() || workingDirectory.isEmpty()) {
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
