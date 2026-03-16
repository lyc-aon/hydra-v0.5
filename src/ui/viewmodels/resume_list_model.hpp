#pragma once

#include <QAbstractListModel>
#include <QVector>

namespace hydra::ui {

class ResumeListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        SessionIdRole = Qt::UserRole + 1,
        NameRole,
        RepoNameRole,
        ProviderNameRole,
        DetailTextRole,
        StatusDetailRole,
        UpdatedAtTextRole,
    };

    struct Entry {
        QString sessionId;
        QString name;
        QString repoName;
        QString providerName;
        QString detailText;
        QString statusDetail;
        QString updatedAtText;
    };

    explicit ResumeListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QString sessionIdAt(int row) const;
    void setEntries(const QVector<Entry> &entries);

private:
    QVector<Entry> m_entries;
};

}  // namespace hydra::ui
