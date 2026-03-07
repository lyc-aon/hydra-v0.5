#include "infrastructure/persistence/sqlite_repo_store.hpp"

#include <QSqlError>
#include <QSqlQuery>

namespace hydra::infrastructure {

SqliteRepoStore::SqliteRepoStore(QSqlDatabase database)
    : m_database(std::move(database))
{
}

QVector<domain::Repository> SqliteRepoStore::listRepositories() const
{
    QVector<domain::Repository> repositories;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id, name, path, description, accent_color, created_at, updated_at "
        "FROM repos ORDER BY updated_at DESC, name ASC"));
    if (!query.exec()) {
        return repositories;
    }

    while (query.next()) {
        repositories.push_back(readRepositoryRow(query));
    }

    return repositories;
}

std::optional<domain::Repository> SqliteRepoStore::repositoryById(const QString &repositoryId) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id, name, path, description, accent_color, created_at, updated_at "
        "FROM repos WHERE id = ?"));
    query.addBindValue(repositoryId);
    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return readRepositoryRow(query);
}

std::optional<domain::Repository> SqliteRepoStore::repositoryByPath(const QString &path) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id, name, path, description, accent_color, created_at, updated_at "
        "FROM repos WHERE path = ?"));
    query.addBindValue(path);
    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return readRepositoryRow(query);
}

bool SqliteRepoStore::upsertRepository(const domain::Repository &repository, QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO repos (id, name, path, description, accent_color, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name, "
        "path = excluded.path, "
        "description = excluded.description, "
        "accent_color = excluded.accent_color, "
        "updated_at = excluded.updated_at"));
    query.addBindValue(repository.id);
    query.addBindValue(repository.name);
    query.addBindValue(repository.path);
    query.addBindValue(repository.description);
    query.addBindValue(repository.accentColor);
    query.addBindValue(repository.createdAt.toString(Qt::ISODateWithMs));
    query.addBindValue(repository.updatedAt.toString(Qt::ISODateWithMs));

    const bool ok = query.exec();
    if (!ok && errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }

    return ok;
}

domain::Repository SqliteRepoStore::readRepositoryRow(const QSqlQuery &query)
{
    domain::Repository repository;
    repository.id = query.value(0).toString();
    repository.name = query.value(1).toString();
    repository.path = query.value(2).toString();
    repository.description = query.value(3).toString();
    repository.accentColor = query.value(4).toString();
    repository.createdAt = QDateTime::fromString(query.value(5).toString(), Qt::ISODateWithMs);
    repository.updatedAt = QDateTime::fromString(query.value(6).toString(), Qt::ISODateWithMs);
    return repository;
}

}  // namespace hydra::infrastructure
