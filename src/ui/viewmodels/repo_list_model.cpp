#include "ui/viewmodels/repo_list_model.hpp"

namespace hydra::ui {

RepoListModel::RepoListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RepoListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_repositories.size();
}

QVariant RepoListModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_repositories.size()) {
        return {};
    }

    const domain::Repository &repository = m_repositories.at(index.row());
    switch (role) {
    case RepoIdRole:
        return repository.id;
    case NameRole:
        return repository.name;
    case PathRole:
        return repository.path;
    case DescriptionRole:
        return repository.description;
    case AccentColorRole:
        return repository.accentColor;
    case SelectedRole:
        return repository.id == m_selectedRepositoryId;
    default:
        return {};
    }
}

QHash<int, QByteArray> RepoListModel::roleNames() const
{
    return {
        {RepoIdRole, "repoId"},
        {NameRole, "name"},
        {PathRole, "path"},
        {DescriptionRole, "description"},
        {AccentColorRole, "accentColor"},
        {SelectedRole, "selected"},
    };
}

void RepoListModel::setRepositories(const QVector<domain::Repository> &repositories)
{
    beginResetModel();
    m_repositories = repositories;
    endResetModel();
}

void RepoListModel::setSelectedRepositoryId(const QString &selectedRepositoryId)
{
    m_selectedRepositoryId = selectedRepositoryId;
    if (!m_repositories.isEmpty()) {
        emit dataChanged(index(0, 0),
                         index(m_repositories.size() - 1, 0),
                         {SelectedRole});
    }
}

}  // namespace hydra::ui
