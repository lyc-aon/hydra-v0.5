#include "infrastructure/providers/hermes_session_resolver.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QUuid>

#include "infrastructure/providers/provider_state_paths.hpp"

namespace hydra::infrastructure {

namespace {

constexpr char kHermesPendingPrefix[] = "hermes-pending-";
constexpr char kHermesHydraTokenPrefix[] = "hydra-hermes-";

QString defaultHermesDatabasePath()
{
    const QString hermesHome = qEnvironmentVariable("HERMES_HOME");
    if (!hermesHome.trimmed().isEmpty()) {
        return QDir(hermesHome.trimmed()).filePath(QStringLiteral("state.db"));
    }

    return QDir(QDir::homePath() + QStringLiteral("/.hermes")).filePath(QStringLiteral("state.db"));
}

QString normalizedHydraHermesToken(const QString &token)
{
    const QString trimmed = token.trimmed();
    if (trimmed.startsWith(QLatin1StringView(kHermesPendingPrefix))) {
        const QString stripped = trimmed.mid(qsizetype(sizeof(kHermesPendingPrefix) - 1));
        return stripped.startsWith(QLatin1StringView(kHermesHydraTokenPrefix)) ? stripped : QString();
    }
    if (trimmed.startsWith(QLatin1StringView(kHermesHydraTokenPrefix))) {
        return trimmed;
    }
    return {};
}

QString hydraHermesDatabasePathForToken(const QString &token)
{
    const QString normalizedToken = normalizedHydraHermesToken(token);
    if (normalizedToken.isEmpty()) {
        return {};
    }

    return QDir(providerStateRootPath(QStringLiteral("hermes"))).filePath(
        QStringLiteral("%1/state.db").arg(normalizedToken));
}

QString escapeSqlLikeToken(const QString &value)
{
    QString escaped = value;
    escaped.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    escaped.replace(QStringLiteral("%"), QStringLiteral("\\%"));
    escaped.replace(QStringLiteral("_"), QStringLiteral("\\_"));
    return escaped;
}

QString baseResumeTitle(const QString &title)
{
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const qsizetype suffixMarker = trimmed.lastIndexOf(QStringLiteral(" #"));
    if (suffixMarker > 0) {
        bool ok = false;
        trimmed.mid(suffixMarker + 2).toInt(&ok);
        if (ok) {
            return trimmed.left(suffixMarker);
        }
    }
    return trimmed;
}

QString managedResumeTitle(const QString &title)
{
    const QString baseTitle = baseResumeTitle(title);
    if (baseTitle.startsWith(QLatin1StringView(kHermesHydraTokenPrefix))) {
        return baseTitle;
    }
    return {};
}

QSqlDatabase openHermesDatabase(const QString &databasePath, QString *connectionName)
{
    const QString name =
        QStringLiteral("hydra-hermes-session-resolver-%1").arg(QUuid::createUuid().toString(QUuid::Id128));
    if (connectionName != nullptr) {
        *connectionName = name;
    }

    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
    database.setDatabaseName(databasePath);
    if (!database.open()) {
        return {};
    }
    return database;
}

bool databaseHasAnySession(const QString &databasePath)
{
    const QFileInfo info(databasePath);
    if (!info.exists() || !info.isFile()) {
        return false;
    }

    QString connectionName;
    bool found = false;
    {
        QSqlDatabase database = openHermesDatabase(databasePath, &connectionName);
        if (!database.isValid() || !database.isOpen()) {
            return false;
        }

        QSqlQuery query(database);
        found = query.exec(QStringLiteral("SELECT 1 FROM sessions LIMIT 1")) && query.next();
        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return found;
}

}  // namespace

HermesSessionResolver::HermesSessionResolver(QString databasePath)
    : m_databasePath(std::move(databasePath))
{
}

QString HermesSessionResolver::providerKey() const
{
    return QStringLiteral("hermes");
}

QSet<QString> HermesSessionResolver::knownSessionIdsForWorkingDirectory(const QString &workingDirectory) const
{
    Q_UNUSED(workingDirectory);

    const QList<QString> titles = listManagedResumeTokens();
    return QSet<QString>(titles.cbegin(), titles.cend());
}

QString HermesSessionResolver::findNewSessionId(const QString &workingDirectory,
                                                const QSet<QString> &knownSessionIds,
                                                const QSet<QString> &excludedSessionIds) const
{
    Q_UNUSED(workingDirectory);

    for (const QString &token : listManagedResumeTokens()) {
        if (knownSessionIds.contains(token) || excludedSessionIds.contains(token)) {
            continue;
        }
        return token;
    }
    return {};
}

bool HermesSessionResolver::sessionExists(const QString &sessionId,
                                          const QString &workingDirectory) const
{
    Q_UNUSED(workingDirectory);

    const QString token = sessionId.trimmed();
    if (token.isEmpty()) {
        return false;
    }

    const QString hydraManagedDatabasePath = hydraHermesDatabasePathForToken(token);
    if (!hydraManagedDatabasePath.isEmpty()) {
        return databaseHasAnySession(hydraManagedDatabasePath);
    }

    const QString path = databasePath();
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return false;
    }

    QString connectionName;
    bool found = false;
    {
        QSqlDatabase database = openHermesDatabase(path, &connectionName);
        if (!database.isValid() || !database.isOpen()) {
            return false;
        }

        const QString escapedTitle = escapeSqlLikeToken(token);
        {
            QSqlQuery query(database);
            query.prepare(QStringLiteral(
                "SELECT 1 "
                "FROM sessions "
                "WHERE id = ? OR title = ? OR title LIKE ? ESCAPE '\\' "
                "LIMIT 1"));
            query.addBindValue(token);
            query.addBindValue(token);
            query.addBindValue(QStringLiteral("%1 #%").arg(escapedTitle));
            found = query.exec() && query.next();
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return found;
}

QString HermesSessionResolver::databasePath() const
{
    const QString trimmed = m_databasePath.trimmed();
    return trimmed.isEmpty() ? defaultHermesDatabasePath() : trimmed;
}

QList<QString> HermesSessionResolver::listManagedResumeTokens() const
{
    QList<QString> tokens;
    QSet<QString> seen;

    const QDir hydraRoot(providerStateRootPath(QStringLiteral("hermes")));
    const QFileInfoList hydraEntries = hydraRoot.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Time | QDir::Reversed);
    for (const QFileInfo &entry : hydraEntries) {
        const QString token = normalizedHydraHermesToken(entry.fileName());
        if (token.isEmpty() || seen.contains(token)) {
            continue;
        }
        if (!databaseHasAnySession(QDir(entry.absoluteFilePath()).filePath(QStringLiteral("state.db")))) {
            continue;
        }
        seen.insert(token);
        tokens.push_back(token);
    }

    const QString path = databasePath();
    const QFileInfo info(path);
    if (info.exists() && info.isFile()) {
        QString connectionName;
        bool queryOk = false;
        {
            QSqlDatabase database = openHermesDatabase(path, &connectionName);
            if (!database.isValid() || !database.isOpen()) {
                return tokens;
            }

            {
                QSqlQuery query(database);
                queryOk = query.exec(QStringLiteral(
                    "SELECT title "
                    "FROM sessions "
                    "WHERE title IS NOT NULL "
                    "ORDER BY started_at DESC"));
                if (queryOk) {
                    while (query.next()) {
                        const QString token = managedResumeTitle(query.value(0).toString());
                        if (token.isEmpty() || seen.contains(token)) {
                            continue;
                        }
                        seen.insert(token);
                        tokens.push_back(token);
                    }
                }
            }
            database.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
        if (!queryOk) {
            return {};
        }
    }

    return tokens;
}

}  // namespace hydra::infrastructure
