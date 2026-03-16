#include "infrastructure/providers/opencode_session_resolver.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>

#include <algorithm>

#include "infrastructure/providers/provider_state_paths.hpp"

namespace hydra::infrastructure {

namespace {

QHash<QString, QString> isolatedEnvironmentForWorkspaceRoot(const QString &workspaceRoot)
{
    QHash<QString, QString> environment;
    if (workspaceRoot.trimmed().isEmpty()) {
        return environment;
    }

    environment.insert(QStringLiteral("XDG_DATA_HOME"),
                       QDir(workspaceRoot).filePath(QStringLiteral("xdg-data")));
    environment.insert(QStringLiteral("XDG_STATE_HOME"),
                       QDir(workspaceRoot).filePath(QStringLiteral("xdg-state")));
    environment.insert(QStringLiteral("XDG_CACHE_HOME"),
                       QDir(workspaceRoot).filePath(QStringLiteral("xdg-cache")));
    return environment;
}

QList<OpenCodeSessionResolver::SessionMeta> parseSessionList(const QByteArray &json)
{
    QList<OpenCodeSessionResolver::SessionMeta> sessions;
    const QJsonDocument document = QJsonDocument::fromJson(json);
    if (!document.isArray()) {
        return sessions;
    }

    const QJsonArray root = document.array();
    sessions.reserve(root.size());
    for (const QJsonValue &entryValue : root) {
        if (!entryValue.isObject()) {
            continue;
        }

        const QJsonObject entry = entryValue.toObject();
        const QString sessionId = entry.value(QStringLiteral("id")).toString().trimmed();
        const QString workingDirectory =
            entry.value(QStringLiteral("directory")).toString().trimmed();
        if (sessionId.isEmpty() || workingDirectory.isEmpty()) {
            continue;
        }

        sessions.push_back({
            .sessionId = sessionId,
            .workingDirectory = workingDirectory,
            .updatedAtMs = static_cast<qint64>(
                entry.value(QStringLiteral("updated")).toVariant().toLongLong()),
        });
    }

    std::sort(sessions.begin(), sessions.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.updatedAtMs > rhs.updatedAtMs;
    });
    return sessions;
}

QList<OpenCodeSessionResolver::SessionMeta> querySessionsWithEnvironment(
    const QHash<QString, QString> &environment)
{
    const QString executablePath = QStandardPaths::findExecutable(QStringLiteral("opencode"));
    if (executablePath.isEmpty()) {
        return {};
    }

    QProcess process;
    QProcessEnvironment processEnvironment = QProcessEnvironment::systemEnvironment();
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        processEnvironment.insert(it.key(), it.value());
    }
    process.setProcessEnvironment(processEnvironment);
    process.start(executablePath,
                  {QStringLiteral("session"),
                   QStringLiteral("list"),
                   QStringLiteral("--format"),
                   QStringLiteral("json"),
                   QStringLiteral("-n"),
                   QStringLiteral("200")});
    if (!process.waitForStarted(3000)) {
        return {};
    }
    if (!process.waitForFinished(10000)) {
        process.kill();
        process.waitForFinished();
        return {};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    return parseSessionList(process.readAllStandardOutput());
}

QList<OpenCodeSessionResolver::SessionMeta> deduplicateNewest(
    const QList<OpenCodeSessionResolver::SessionMeta> &sessions)
{
    QHash<QString, OpenCodeSessionResolver::SessionMeta> newestById;
    for (const auto &session : sessions) {
        const auto existing = newestById.constFind(session.sessionId);
        if (existing == newestById.cend() || existing->updatedAtMs < session.updatedAtMs) {
            newestById.insert(session.sessionId, session);
        }
    }

    QList<OpenCodeSessionResolver::SessionMeta> deduplicated = newestById.values();
    std::sort(deduplicated.begin(), deduplicated.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.updatedAtMs > rhs.updatedAtMs;
    });
    return deduplicated;
}

}  // namespace

QString OpenCodeSessionResolver::providerKey() const
{
    return QStringLiteral("opencode");
}

QSet<QString> OpenCodeSessionResolver::knownSessionIdsForWorkingDirectory(
    const QString &workingDirectory) const
{
    QSet<QString> sessionIds;
    for (const auto &session : listSessionsForWorkingDirectory(workingDirectory)) {
        sessionIds.insert(session.sessionId);
    }
    return sessionIds;
}

QString OpenCodeSessionResolver::findNewSessionId(const QString &workingDirectory,
                                                  const QSet<QString> &knownSessionIds,
                                                  const QSet<QString> &excludedSessionIds) const
{
    for (const auto &session : listSessionsForWorkingDirectory(workingDirectory)) {
        if (knownSessionIds.contains(session.sessionId)
            || excludedSessionIds.contains(session.sessionId))
        {
            continue;
        }
        return session.sessionId;
    }

    return {};
}

bool OpenCodeSessionResolver::sessionExists(const QString &sessionId,
                                            const QString &workingDirectory) const
{
    const QString trimmedSessionId = sessionId.trimmed();
    if (trimmedSessionId.isEmpty()) {
        return false;
    }

    const QList<SessionMeta> sessions = workingDirectory.trimmed().isEmpty()
                                            ? listAllSessions()
                                            : listSessionsForWorkingDirectory(workingDirectory);
    for (const auto &session : sessions) {
        if (session.sessionId == trimmedSessionId) {
            return true;
        }
    }

    return false;
}

QString OpenCodeSessionResolver::workspaceRootPathForWorkingDirectory(const QString &workingDirectory)
{
    const QString normalized = normalizeWorkingDirectory(workingDirectory);
    if (normalized.isEmpty()) {
        return {};
    }

    return QDir(stateRootPath()).filePath(workspaceSlug(normalized));
}

QString OpenCodeSessionResolver::xdgDataHomePathForWorkingDirectory(const QString &workingDirectory)
{
    const QString workspaceRoot = workspaceRootPathForWorkingDirectory(workingDirectory);
    return workspaceRoot.isEmpty() ? QString() : QDir(workspaceRoot).filePath(QStringLiteral("xdg-data"));
}

QString OpenCodeSessionResolver::xdgStateHomePathForWorkingDirectory(const QString &workingDirectory)
{
    const QString workspaceRoot = workspaceRootPathForWorkingDirectory(workingDirectory);
    return workspaceRoot.isEmpty() ? QString() : QDir(workspaceRoot).filePath(QStringLiteral("xdg-state"));
}

QString OpenCodeSessionResolver::xdgCacheHomePathForWorkingDirectory(const QString &workingDirectory)
{
    const QString workspaceRoot = workspaceRootPathForWorkingDirectory(workingDirectory);
    return workspaceRoot.isEmpty() ? QString() : QDir(workspaceRoot).filePath(QStringLiteral("xdg-cache"));
}

QString OpenCodeSessionResolver::normalizeWorkingDirectory(const QString &workingDirectory)
{
    const QString trimmed = workingDirectory.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QFileInfo info(trimmed);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

QList<OpenCodeSessionResolver::SessionMeta> OpenCodeSessionResolver::listSessionsForWorkingDirectory(
    const QString &workingDirectory)
{
    const QString target = normalizeWorkingDirectory(workingDirectory);
    if (target.isEmpty()) {
        return {};
    }

    QList<SessionMeta> sessions = querySessionsWithEnvironment({});
    const QString workspaceRoot = workspaceRootPathForWorkingDirectory(target);
    if (!workspaceRoot.isEmpty()) {
        sessions.append(querySessionsWithEnvironment(isolatedEnvironmentForWorkspaceRoot(workspaceRoot)));
    }

    QList<SessionMeta> filtered;
    for (const auto &session : deduplicateNewest(sessions)) {
        if (normalizeWorkingDirectory(session.workingDirectory) == target) {
            filtered.push_back(session);
        }
    }

    return filtered;
}

QList<OpenCodeSessionResolver::SessionMeta> OpenCodeSessionResolver::listAllSessions()
{
    QList<SessionMeta> sessions = querySessionsWithEnvironment({});

    const QDir stateRoot(stateRootPath());
    if (stateRoot.exists()) {
        const QFileInfoList workspaceRoots =
            stateRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
        for (const QFileInfo &workspaceRoot : workspaceRoots) {
            sessions.append(
                querySessionsWithEnvironment(isolatedEnvironmentForWorkspaceRoot(
                    workspaceRoot.absoluteFilePath())));
        }
    }

    return deduplicateNewest(sessions);
}

QString OpenCodeSessionResolver::stateRootPath()
{
    return providerStateRootPath(QStringLiteral("opencode"));
}

QString OpenCodeSessionResolver::workspaceSlug(const QString &normalizedWorkingDirectory)
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

}  // namespace hydra::infrastructure
