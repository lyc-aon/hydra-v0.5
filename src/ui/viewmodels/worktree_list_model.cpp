#include "ui/viewmodels/worktree_list_model.hpp"

namespace hydra::ui {

WorktreeListModel::WorktreeListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WorktreeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_worktrees.size();
}

QVariant WorktreeListModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_worktrees.size()) {
        return {};
    }

    const domain::Worktree &worktree = m_worktrees.at(index.row());
    switch (role) {
    case PathRole:
        return worktree.path;
    case BranchNameRole:
        return worktree.branchName;
    case MainRole:
        return worktree.isMain;
    case SelectedRole:
        return worktree.path == m_selectedWorktreePath;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorktreeListModel::roleNames() const
{
    return {
        {PathRole, "path"},
        {BranchNameRole, "branchName"},
        {MainRole, "isMain"},
        {SelectedRole, "selected"},
    };
}

void WorktreeListModel::setWorktrees(const QVector<domain::Worktree> &worktrees)
{
    beginResetModel();
    m_worktrees = worktrees;
    endResetModel();
}

void WorktreeListModel::setSelectedWorktreePath(const QString &selectedWorktreePath)
{
    m_selectedWorktreePath = selectedWorktreePath;
    if (!m_worktrees.isEmpty()) {
        emit dataChanged(index(0, 0), index(m_worktrees.size() - 1, 0), {SelectedRole});
    }
}

}  // namespace hydra::ui
