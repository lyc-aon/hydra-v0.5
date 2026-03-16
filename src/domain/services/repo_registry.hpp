#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/ports/repo_store.hpp"

namespace hydra::domain {

class RepoRegistry final {
public:
    struct EnsureRepositoryOutcome {
        bool ok = false;
        bool created = false;
        Repository repository;
        QString errorMessage;
    };

    explicit RepoRegistry(ports::RepoStore &repoStore);

    QVector<Repository> repositories() const;
    std::optional<Repository> repositoryById(const QString &repositoryId) const;
    std::optional<Repository> repositoryByPath(const QString &path) const;
    bool deleteRepository(const QString &repositoryId, QString *errorMessage = nullptr);
    EnsureRepositoryOutcome ensureRepository(const QString &path,
                                            const QString &name,
                                            const QString &description,
                                            const QString &accentColor);
    bool ensureBootstrapRepository(const QString &path,
                                   const QString &name,
                                   const QString &description,
                                   const QString &accentColor,
                                   QString *errorMessage = nullptr);

private:
    ports::RepoStore &m_repoStore;
};

}  // namespace hydra::domain
