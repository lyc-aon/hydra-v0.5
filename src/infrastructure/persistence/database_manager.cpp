#include "infrastructure/persistence/database_manager.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <stdexcept>

namespace hydra::infrastructure {

DatabaseManager::DatabaseManager(const QString &databasePath)
    : m_connectionName(QStringLiteral("hydra-%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
    QDir().mkpath(QFileInfo(databasePath).absolutePath());

    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(databasePath);
    if (!m_database.open()) {
        throw std::runtime_error(
            QStringLiteral("Failed to open Hydra database: %1").arg(m_database.lastError().text())
                .toStdString());
    }

    runMigrations();
}

DatabaseManager::~DatabaseManager()
{
    const QString connectionName = m_connectionName;
    m_database.close();
    m_database = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

void DatabaseManager::runMigrations()
{
    execOrThrow(QStringLiteral("PRAGMA foreign_keys = ON"));
    execOrThrow(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS repos ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "path TEXT NOT NULL UNIQUE,"
        "description TEXT NOT NULL DEFAULT '',"
        "accent_color TEXT NOT NULL DEFAULT '#b76e3c',"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL"
        ")"));
    execOrThrow(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS sessions ("
        "id TEXT PRIMARY KEY,"
        "repo_id TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "provider_key TEXT NOT NULL,"
        "provider_session_id TEXT NOT NULL DEFAULT '',"
        "owner_instance_id TEXT NOT NULL DEFAULT '',"
        "launch_safety_key TEXT NOT NULL DEFAULT 'workspace-safe',"
        "state TEXT NOT NULL,"
        "tmux_session_name TEXT,"
        "tmux_pane_id TEXT,"
        "working_directory TEXT NOT NULL,"
        "model_override TEXT NOT NULL DEFAULT '',"
        "launch_environment_json TEXT NOT NULL DEFAULT '{}',"
        "provider_profile_mode TEXT NOT NULL DEFAULT '',"
        "provider_profile_path TEXT NOT NULL DEFAULT '',"
        "last_error TEXT NOT NULL DEFAULT '',"
        "status_detail TEXT NOT NULL DEFAULT '',"
        "status_provenance_key TEXT NOT NULL DEFAULT 'derived',"
        "status_source_ref TEXT NOT NULL DEFAULT '',"
        "transcript_path TEXT NOT NULL DEFAULT '',"
        "audit_path TEXT NOT NULL DEFAULT '',"
        "alias TEXT NOT NULL DEFAULT '',"
        "category_key TEXT NOT NULL DEFAULT 'worker',"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "FOREIGN KEY(repo_id) REFERENCES repos(id) ON DELETE CASCADE"
        ")"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("provider_session_id"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("owner_instance_id"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("last_error"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("model_override"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("launch_environment_json"),
                       QStringLiteral("TEXT NOT NULL DEFAULT '{}'"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("provider_profile_mode"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("provider_profile_path"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("launch_safety_key"),
                       QStringLiteral("TEXT NOT NULL DEFAULT 'workspace-safe'"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("status_detail"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("status_provenance_key"),
                       QStringLiteral("TEXT NOT NULL DEFAULT 'derived'"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("status_source_ref"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("transcript_path"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("audit_path"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("alias"),
                       QStringLiteral("TEXT NOT NULL DEFAULT ''"));
    ensureColumnExists(QStringLiteral("sessions"),
                       QStringLiteral("category_key"),
                       QStringLiteral("TEXT NOT NULL DEFAULT 'worker'"));
    execOrThrow(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS session_timeline ("
        "id TEXT PRIMARY KEY,"
        "session_id TEXT NOT NULL,"
        "signal_kind TEXT NOT NULL,"
        "state TEXT NOT NULL,"
        "summary TEXT NOT NULL,"
        "detail TEXT NOT NULL,"
        "provenance_key TEXT NOT NULL,"
        "source_ref TEXT NOT NULL DEFAULT '',"
        "created_at TEXT NOT NULL,"
        "FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE"
        ")"));
    execOrThrow(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_sessions_repo_id ON sessions(repo_id)"));
    execOrThrow(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_sessions_updated_at ON sessions(updated_at DESC)"));
    execOrThrow(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_session_timeline_session_created "
        "ON session_timeline(session_id, created_at DESC)"));
}

void DatabaseManager::execOrThrow(const QString &sql)
{
    QSqlQuery query(m_database);
    if (!query.exec(sql)) {
        throw std::runtime_error(
            QStringLiteral("Database migration failed: %1").arg(query.lastError().text())
                .toStdString());
    }
}

bool DatabaseManager::tableHasColumn(const QString &tableName, const QString &columnName)
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(%1)").arg(tableName))) {
        throw std::runtime_error(
            QStringLiteral("Failed to inspect SQLite table %1: %2")
                .arg(tableName, query.lastError().text())
                .toStdString());
    }

    while (query.next()) {
        if (query.value(1).toString() == columnName) {
            return true;
        }
    }

    return false;
}

void DatabaseManager::ensureColumnExists(const QString &tableName,
                                         const QString &columnName,
                                         const QString &columnDefinition)
{
    if (tableHasColumn(tableName, columnName)) {
        return;
    }

    execOrThrow(QStringLiteral("ALTER TABLE %1 ADD COLUMN %2 %3")
                    .arg(tableName, columnName, columnDefinition));
}

}  // namespace hydra::infrastructure
