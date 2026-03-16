#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "domain/models/worktree.hpp"

namespace hydra::ui {

class WorktreeListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        PathRole = Qt::UserRole + 1,
        BranchNameRole,
        MainRole,
        SelectedRole,
    };

    explicit WorktreeListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWorktrees(const QVector<domain::Worktree> &worktrees);
    void setSelectedWorktreePath(const QString &selectedWorktreePath);

private:
    QVector<domain::Worktree> m_worktrees;
    QString m_selectedWorktreePath;
};

}  // namespace hydra::ui
