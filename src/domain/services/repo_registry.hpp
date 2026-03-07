#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/ports/repo_store.hpp"

namespace hydra::domain {

class RepoRegistry final {
public:
    explicit RepoRegistry(ports::RepoStore &repoStore);

    QVector<Repository> repositories() const;
    std::optional<Repository> repositoryById(const QString &repositoryId) const;
    bool ensureBootstrapRepository(const QString &path,
                                   const QString &name,
                                   const QString &description,
                                   const QString &accentColor,
                                   QString *errorMessage = nullptr);

private:
    ports::RepoStore &m_repoStore;
};

}  // namespace hydra::domain
