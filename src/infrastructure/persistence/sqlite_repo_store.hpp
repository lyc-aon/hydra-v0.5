#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>

#include "domain/ports/repo_store.hpp"

namespace hydra::infrastructure {

class SqliteRepoStore final : public domain::ports::RepoStore {
public:
    explicit SqliteRepoStore(QSqlDatabase database);

    QVector<domain::Repository> listRepositories() const override;
    std::optional<domain::Repository> repositoryById(const QString &repositoryId) const override;
    std::optional<domain::Repository> repositoryByPath(const QString &path) const override;
    bool upsertRepository(const domain::Repository &repository,
                          QString *errorMessage = nullptr) override;

private:
    static domain::Repository readRepositoryRow(const QSqlQuery &query);

    QSqlDatabase m_database;
};

}  // namespace hydra::infrastructure
