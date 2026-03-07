#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "domain/models/repository.hpp"

namespace hydra::ui {

class RepoListModel final : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        RepoIdRole = Qt::UserRole + 1,
        NameRole,
        PathRole,
        DescriptionRole,
        AccentColorRole,
        SelectedRole,
    };

    explicit RepoListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRepositories(const QVector<domain::Repository> &repositories);
    void setSelectedRepositoryId(const QString &selectedRepositoryId);

private:
    QVector<domain::Repository> m_repositories;
    QString m_selectedRepositoryId;
};

}  // namespace hydra::ui
