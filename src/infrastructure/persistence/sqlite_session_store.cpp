#include "infrastructure/persistence/sqlite_session_store.hpp"

#include <QSqlError>
#include <QSqlQuery>

namespace hydra::infrastructure {

namespace {

QString nonNullText(const QString &value)
{
    return value.isNull() ? QStringLiteral("") : value;
}

QString isoTimestampText(const QDateTime &value)
{
    return nonNullText(value.toString(Qt::ISODateWithMs));
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
    query.prepare(QStringLiteral(
        "SELECT id, repo_id, name, provider_key, state, tmux_session_name, tmux_pane_id, "
        "working_directory, last_error, created_at, updated_at "
        "FROM sessions ORDER BY updated_at DESC"));
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
    query.prepare(QStringLiteral(
        "SELECT id, repo_id, name, provider_key, state, tmux_session_name, tmux_pane_id, "
        "working_directory, last_error, created_at, updated_at "
        "FROM sessions WHERE id = ?"));
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
        "INSERT INTO sessions (id, repo_id, name, provider_key, state, tmux_session_name, "
        "tmux_pane_id, working_directory, last_error, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "repo_id = excluded.repo_id, "
        "name = excluded.name, "
        "provider_key = excluded.provider_key, "
        "state = excluded.state, "
        "tmux_session_name = excluded.tmux_session_name, "
        "tmux_pane_id = excluded.tmux_pane_id, "
        "working_directory = excluded.working_directory, "
        "last_error = excluded.last_error, "
        "updated_at = excluded.updated_at"));
    query.addBindValue(nonNullText(session.id));
    query.addBindValue(nonNullText(session.repoId));
    query.addBindValue(nonNullText(session.name));
    query.addBindValue(nonNullText(session.providerKey));
    query.addBindValue(nonNullText(domain::sessionStateToStorage(session.state)));
    query.addBindValue(session.tmuxSessionName);
    query.addBindValue(session.tmuxPaneId);
    query.addBindValue(nonNullText(session.workingDirectory));
    query.addBindValue(nonNullText(session.lastError));
    query.addBindValue(isoTimestampText(session.createdAt));
    query.addBindValue(isoTimestampText(session.updatedAt));

    const bool ok = query.exec();
    if (!ok && errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }

    return ok;
}

domain::SessionRecord SqliteSessionStore::readSessionRow(const QSqlQuery &query)
{
    domain::SessionRecord session;
    session.id = query.value(0).toString();
    session.repoId = query.value(1).toString();
    session.name = query.value(2).toString();
    session.providerKey = query.value(3).toString();
    session.state = domain::sessionStateFromStorage(query.value(4).toString());
    session.tmuxSessionName = query.value(5).toString();
    session.tmuxPaneId = query.value(6).toString();
    session.workingDirectory = query.value(7).toString();
    session.lastError = query.value(8).toString();
    session.createdAt = QDateTime::fromString(query.value(9).toString(), Qt::ISODateWithMs);
    session.updatedAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODateWithMs);
    return session;
}

}  // namespace hydra::infrastructure
