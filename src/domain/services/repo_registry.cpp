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

std::optional<Repository> RepoRegistry::repositoryByPath(const QString &path) const
{
    return m_repoStore.repositoryByPath(path);
}

bool RepoRegistry::deleteRepository(const QString &repositoryId, QString *errorMessage)
{
    return m_repoStore.deleteRepository(repositoryId, errorMessage);
}

RepoRegistry::EnsureRepositoryOutcome RepoRegistry::ensureRepository(const QString &path,
                                                                    const QString &name,
                                                                    const QString &description,
                                                                    const QString &accentColor)
{
    EnsureRepositoryOutcome outcome;
    const QString normalizedPath = QDir::cleanPath(path);
    const QString normalizedName = name.isNull() ? QString() : name;
    const QString normalizedDescription =
        description.isNull() ? QStringLiteral("") : description;
    const QString normalizedAccentColor =
        accentColor.isNull() ? QStringLiteral("#b76e3c") : accentColor;

    if (normalizedPath.isEmpty() || !QDir(normalizedPath).exists()) {
        outcome.errorMessage = QStringLiteral("Repository path does not exist: %1")
                                   .arg(normalizedPath);
        return outcome;
    }

    const std::optional<Repository> existingRepository =
        m_repoStore.repositoryByPath(normalizedPath);
    if (existingRepository.has_value()) {
        Repository repository = *existingRepository;
        if (repository.name == normalizedName
            && repository.description == normalizedDescription
            && repository.accentColor == normalizedAccentColor)
        {
            outcome.repository = repository;
            outcome.ok = true;
            return outcome;
        }

        repository.name = normalizedName;
        repository.description = normalizedDescription;
        repository.accentColor = normalizedAccentColor;
        repository.updatedAt = QDateTime::currentDateTimeUtc();
        outcome.ok = m_repoStore.upsertRepository(repository, &outcome.errorMessage);
        outcome.repository = repository;
        return outcome;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    Repository repository;
    repository.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    repository.name = normalizedName;
    repository.path = normalizedPath;
    repository.description = normalizedDescription;
    repository.accentColor = normalizedAccentColor;
    repository.createdAt = now;
    repository.updatedAt = now;

    outcome.ok = m_repoStore.upsertRepository(repository, &outcome.errorMessage);
    outcome.created = outcome.ok;
    outcome.repository = repository;
    return outcome;
}

bool RepoRegistry::ensureBootstrapRepository(const QString &path,
                                             const QString &name,
                                             const QString &description,
                                             const QString &accentColor,
                                             QString *errorMessage)
{
    const EnsureRepositoryOutcome outcome = ensureRepository(path, name, description, accentColor);
    if (!outcome.ok && errorMessage != nullptr) {
        *errorMessage = outcome.errorMessage;
    }
    return outcome.ok;
}

}  // namespace hydra::domain
