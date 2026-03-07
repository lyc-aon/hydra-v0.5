#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/repository.hpp"

namespace hydra::domain::ports {

class RepoStore {
public:
    virtual ~RepoStore() = default;

    virtual QVector<domain::Repository> listRepositories() const = 0;
    virtual std::optional<domain::Repository> repositoryById(const QString &repositoryId) const = 0;
    virtual std::optional<domain::Repository> repositoryByPath(const QString &path) const = 0;
    virtual bool upsertRepository(const domain::Repository &repository, QString *errorMessage = nullptr) = 0;
};

}  // namespace hydra::domain::ports
