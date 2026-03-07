#include "infrastructure/persistence/database_manager.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

#include <stdexcept>

namespace hydra::infrastructure {

DatabaseManager::DatabaseManager(const QString &databasePath)
    : m_connectionName(QStringLiteral("hydra-main"))
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
        "state TEXT NOT NULL,"
        "tmux_session_name TEXT,"
        "tmux_pane_id TEXT,"
        "working_directory TEXT NOT NULL,"
        "last_error TEXT NOT NULL DEFAULT '',"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "FOREIGN KEY(repo_id) REFERENCES repos(id) ON DELETE CASCADE"
        ")"));
    execOrThrow(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_sessions_repo_id ON sessions(repo_id)"));
    execOrThrow(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_sessions_updated_at ON sessions(updated_at DESC)"));
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

}  // namespace hydra::infrastructure
