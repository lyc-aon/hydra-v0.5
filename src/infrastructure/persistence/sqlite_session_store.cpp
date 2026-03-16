#include "infrastructure/persistence/sqlite_session_store.hpp"

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>

#include "domain/models/session_category.hpp"

namespace hydra::infrastructure {

namespace {

constexpr auto kSessionSelectColumns =
    "id, repo_id, name, provider_key, provider_session_id, owner_instance_id, launch_safety_key, state, tmux_session_name, "
    "tmux_pane_id, working_directory, model_override, launch_environment_json, provider_profile_mode, "
    "provider_profile_path, last_error, status_detail, status_provenance_key, status_source_ref, "
    "transcript_path, audit_path, created_at, updated_at, alias, category_key";

QString nonNullText(const QString &value)
{
    return value.isNull() ? QStringLiteral("") : value;
}

QString isoTimestampText(const QDateTime &value)
{
    return nonNullText(value.toString(Qt::ISODateWithMs));
}

QString launchEnvironmentJson(const QHash<QString, QString> &environment)
{
    QJsonObject object;
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        object.insert(it.key(), it.value());
    }
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QHash<QString, QString> launchEnvironmentFromJson(const QString &jsonText)
{
    const QJsonDocument document = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!document.isObject()) {
        return {};
    }

    QHash<QString, QString> environment;
    const QJsonObject object = document.object();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.value().isString()) {
            environment.insert(it.key(), it.value().toString());
        }
    }
    return environment;
}

}  // namespace

SqliteSessionStore::SqliteSessionStore(QSqlDatabase database)
    : m_database(std::move(database))
{
}

QVector<domain::SessionRecord> SqliteSessionStore::listSessions() const
{
    QVector<domain::SessionRecord> sessions;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT %1 FROM sessions ORDER BY updated_at DESC")
                      .arg(QString::fromUtf8(kSessionSelectColumns)));
    if (!query.exec()) {
        return sessions;
    }

    while (query.next()) {
        sessions.push_back(readSessionRow(query));
    }

    return sessions;
}

std::optional<domain::SessionRecord> SqliteSessionStore::sessionById(const QString &sessionId) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT %1 FROM sessions WHERE id = ?")
                      .arg(QString::fromUtf8(kSessionSelectColumns)));
    query.addBindValue(sessionId);
    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return readSessionRow(query);
}

bool SqliteSessionStore::upsertSession(const domain::SessionRecord &session, QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO sessions (id, repo_id, name, provider_key, provider_session_id, owner_instance_id, launch_safety_key, state, "
        "tmux_session_name, tmux_pane_id, working_directory, model_override, launch_environment_json, "
        "provider_profile_mode, provider_profile_path, last_error, status_detail, "
        "status_provenance_key, status_source_ref, transcript_path, audit_path, created_at, updated_at, alias, category_key) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "repo_id = excluded.repo_id, "
        "name = excluded.name, "
        "provider_key = excluded.provider_key, "
        "provider_session_id = excluded.provider_session_id, "
        "owner_instance_id = excluded.owner_instance_id, "
        "launch_safety_key = excluded.launch_safety_key, "
        "state = excluded.state, "
        "tmux_session_name = excluded.tmux_session_name, "
        "tmux_pane_id = excluded.tmux_pane_id, "
        "working_directory = excluded.working_directory, "
        "model_override = excluded.model_override, "
        "launch_environment_json = excluded.launch_environment_json, "
        "provider_profile_mode = excluded.provider_profile_mode, "
        "provider_profile_path = excluded.provider_profile_path, "
        "last_error = excluded.last_error, "
        "status_detail = excluded.status_detail, "
        "status_provenance_key = excluded.status_provenance_key, "
        "status_source_ref = excluded.status_source_ref, "
        "transcript_path = excluded.transcript_path, "
        "audit_path = excluded.audit_path, "
        "alias = excluded.alias, "
        "category_key = excluded.category_key, "
        "updated_at = excluded.updated_at"));
    query.addBindValue(nonNullText(session.id));
    query.addBindValue(nonNullText(session.repoId));
    query.addBindValue(nonNullText(session.name));
    query.addBindValue(nonNullText(session.providerKey));
    query.addBindValue(nonNullText(session.providerSessionId));
    query.addBindValue(nonNullText(session.ownerInstanceId));
    query.addBindValue(nonNullText(session.launchSafetyKey));
    query.addBindValue(nonNullText(domain::sessionStateToStorage(session.state)));
    query.addBindValue(session.tmuxSessionName);
    query.addBindValue(session.tmuxPaneId);
    query.addBindValue(nonNullText(session.workingDirectory));
    query.addBindValue(nonNullText(session.modelOverride));
    query.addBindValue(launchEnvironmentJson(session.launchEnvironment));
    query.addBindValue(nonNullText(session.providerProfileMode));
    query.addBindValue(nonNullText(session.providerProfilePath));
    query.addBindValue(nonNullText(session.lastError));
    query.addBindValue(nonNullText(session.statusDetail));
    query.addBindValue(nonNullText(domain::statusProvenanceToStorage(session.statusProvenance)));
    query.addBindValue(nonNullText(session.statusSourceRef));
    query.addBindValue(nonNullText(session.transcriptPath));
    query.addBindValue(nonNullText(session.auditPath));
    query.addBindValue(isoTimestampText(session.createdAt));
    query.addBindValue(isoTimestampText(session.updatedAt));
    query.addBindValue(nonNullText(session.alias));
    query.addBindValue(nonNullText(domain::sessionCategoryToKey(session.category)));

    const bool ok = query.exec();
    if (!ok && errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }

    return ok;
}

bool SqliteSessionStore::deleteSessions(const QStringList &sessionIds, QString *errorMessage)
{
    if (sessionIds.isEmpty()) {
        return true;
    }

    QStringList placeholders;
    placeholders.reserve(sessionIds.size());
    for (qsizetype index = 0; index < sessionIds.size(); ++index) {
        placeholders.push_back(QStringLiteral("?"));
    }

    if (!m_database.transaction()) {
        if (errorMessage != nullptr) {
            *errorMessage = m_database.lastError().text();
        }
        return false;
    }

    auto rollbackWith = [&](const QString &message) {
        m_database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
        return false;
    };

    QSqlQuery timelineDelete(m_database);
    timelineDelete.prepare(QStringLiteral("DELETE FROM session_timeline WHERE session_id IN (%1)")
                               .arg(placeholders.join(QStringLiteral(", "))));
    for (const QString &sessionId : sessionIds) {
        timelineDelete.addBindValue(sessionId);
    }
    if (!timelineDelete.exec()) {
        return rollbackWith(timelineDelete.lastError().text());
    }

    QSqlQuery sessionDelete(m_database);
    sessionDelete.prepare(QStringLiteral("DELETE FROM sessions WHERE id IN (%1)")
                              .arg(placeholders.join(QStringLiteral(", "))));
    for (const QString &sessionId : sessionIds) {
        sessionDelete.addBindValue(sessionId);
    }
    if (!sessionDelete.exec()) {
        return rollbackWith(sessionDelete.lastError().text());
    }

    if (!m_database.commit()) {
        return rollbackWith(m_database.lastError().text());
    }

    return true;
}

QHash<QString, QVector<domain::SessionTimelineEvent>> SqliteSessionStore::recentTimelineBySession(
    const QStringList &sessionIds,
    const int limitPerSession) const
{
    QHash<QString, QVector<domain::SessionTimelineEvent>> timelines;
    if (sessionIds.isEmpty() || limitPerSession <= 0) {
        return timelines;
    }

    QStringList placeholders;
    placeholders.reserve(sessionIds.size());
    for (qsizetype index = 0; index < sessionIds.size(); ++index) {
        placeholders.push_back(QStringLiteral("?"));
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id, session_id, signal_kind, state, summary, detail, provenance_key, "
        "source_ref, created_at "
        "FROM ("
        "  SELECT id, session_id, signal_kind, state, summary, detail, provenance_key, "
        "         source_ref, created_at, "
        "         ROW_NUMBER() OVER (PARTITION BY session_id ORDER BY created_at DESC) AS row_rank "
        "  FROM session_timeline WHERE session_id IN (%1)"
        ") WHERE row_rank <= ? "
        "ORDER BY created_at DESC")
                      .arg(placeholders.join(QStringLiteral(", "))));
    for (const QString &sessionId : sessionIds) {
        query.addBindValue(sessionId);
    }
    query.addBindValue(limitPerSession);

    if (!query.exec()) {
        return timelines;
    }

    while (query.next()) {
        const domain::SessionTimelineEvent event = readTimelineRow(query);
        timelines[event.sessionId].push_back(event);
    }

    return timelines;
}

bool SqliteSessionStore::appendTimelineEvent(const domain::SessionTimelineEvent &event,
                                             const int retainPerSession,
                                             QString *errorMessage)
{
    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(QStringLiteral(
        "INSERT INTO session_timeline (id, session_id, signal_kind, state, summary, detail, "
        "provenance_key, source_ref, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    insertQuery.addBindValue(nonNullText(event.id));
    insertQuery.addBindValue(nonNullText(event.sessionId));
    insertQuery.addBindValue(nonNullText(domain::sessionSignalKindToStorage(event.signalKind)));
    insertQuery.addBindValue(nonNullText(domain::sessionStateToStorage(event.state)));
    insertQuery.addBindValue(nonNullText(event.summary));
    insertQuery.addBindValue(nonNullText(event.detail));
    insertQuery.addBindValue(nonNullText(domain::statusProvenanceToStorage(event.provenance)));
    insertQuery.addBindValue(nonNullText(event.sourceRef));
    insertQuery.addBindValue(isoTimestampText(event.createdAt));
    if (!insertQuery.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = insertQuery.lastError().text();
        }
        return false;
    }

    QSqlQuery pruneQuery(m_database);
    pruneQuery.prepare(QStringLiteral(
        "DELETE FROM session_timeline "
        "WHERE session_id = ? "
        "AND id NOT IN ("
        "  SELECT id FROM session_timeline "
        "  WHERE session_id = ? "
        "  ORDER BY created_at DESC "
        "  LIMIT ?"
        ")"));
    pruneQuery.addBindValue(event.sessionId);
    pruneQuery.addBindValue(event.sessionId);
    pruneQuery.addBindValue(retainPerSession);
    if (!pruneQuery.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = pruneQuery.lastError().text();
        }
        return false;
    }

    return true;
}

domain::SessionRecord SqliteSessionStore::readSessionRow(const QSqlQuery &query)
{
    domain::SessionRecord session;
    session.id = query.value(0).toString();
    session.repoId = query.value(1).toString();
    session.name = query.value(2).toString();
    session.providerKey = query.value(3).toString();
    session.providerSessionId = query.value(4).toString();
    session.ownerInstanceId = query.value(5).toString();
    session.launchSafetyKey = query.value(6).toString();
    session.state = domain::sessionStateFromStorage(query.value(7).toString());
    session.tmuxSessionName = query.value(8).toString();
    session.tmuxPaneId = query.value(9).toString();
    session.workingDirectory = query.value(10).toString();
    session.modelOverride = query.value(11).toString();
    session.launchEnvironment = launchEnvironmentFromJson(query.value(12).toString());
    session.providerProfileMode = query.value(13).toString();
    session.providerProfilePath = query.value(14).toString();
    session.lastError = query.value(15).toString();
    session.statusDetail = query.value(16).toString();
    session.statusProvenance = domain::statusProvenanceFromStorage(query.value(17).toString());
    session.statusSourceRef = query.value(18).toString();
    session.transcriptPath = query.value(19).toString();
    session.auditPath = query.value(20).toString();
    session.createdAt = QDateTime::fromString(query.value(21).toString(), Qt::ISODateWithMs);
    session.updatedAt = QDateTime::fromString(query.value(22).toString(), Qt::ISODateWithMs);
    session.alias = query.value(23).toString();
    session.category = domain::sessionCategoryFromKey(query.value(24).toString());
    return session;
}

domain::SessionTimelineEvent SqliteSessionStore::readTimelineRow(const QSqlQuery &query)
{
    domain::SessionTimelineEvent event;
    event.id = query.value(0).toString();
    event.sessionId = query.value(1).toString();
    event.signalKind = domain::sessionSignalKindFromStorage(query.value(2).toString());
    event.state = domain::sessionStateFromStorage(query.value(3).toString());
    event.summary = query.value(4).toString();
    event.detail = query.value(5).toString();
    event.provenance = domain::statusProvenanceFromStorage(query.value(6).toString());
    event.sourceRef = query.value(7).toString();
    event.createdAt = QDateTime::fromString(query.value(8).toString(), Qt::ISODateWithMs);
    return event;
}

}  // namespace hydra::infrastructure
