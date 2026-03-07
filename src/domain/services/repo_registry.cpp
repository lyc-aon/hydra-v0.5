#include "domain/services/repo_registry.hpp"

#include <QDateTime>
#include <QDir>
#include <QUuid>

namespace hydra::domain {

RepoRegistry::RepoRegistry(ports::RepoStore &repoStore)
    : m_repoStore(repoStore)
{
}

QVector<Repository> RepoRegistry::repositories() const
{
    return m_repoStore.listRepositories();
}

std::optional<Repository> RepoRegistry::repositoryById(const QString &repositoryId) const
{
    return m_repoStore.repositoryById(repositoryId);
}

bool RepoRegistry::ensureBootstrapRepository(const QString &path,
                                             const QString &name,
                                             const QString &description,
                                             const QString &accentColor,
                                             QString *errorMessage)
{
    if (path.isEmpty() || !QDir(path).exists()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Bootstrap repository path does not exist: %1").arg(path);
        }
        return false;
    }

    if (m_repoStore.repositoryByPath(path).has_value()) {
        return true;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    Repository repository;
    repository.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    repository.name = name;
    repository.path = path;
    repository.description = description;
    repository.accentColor = accentColor;
    repository.createdAt = now;
    repository.updatedAt = now;

    return m_repoStore.upsertRepository(repository, errorMessage);
}

}  // namespace hydra::domain
