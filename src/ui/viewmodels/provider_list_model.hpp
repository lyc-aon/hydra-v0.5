#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "domain/ports/provider_adapter.hpp"

namespace hydra::ui {

class ProviderListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        ProviderKeyRole = Qt::UserRole + 1,
        DisplayNameRole,
        AvailableRole,
        StatusMessageRole,
        SelectedRole,
    };

    explicit ProviderListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setProviders(const QVector<domain::ports::ProviderProbe> &providers);
    void setSelectedProviderKey(const QString &providerKey);

private:
    struct ProviderEntry {
        QString providerKey;
        QString displayName;
        bool available = false;
        QString statusMessage;
        bool selected = false;
    };

    QVector<ProviderEntry> m_providers;
    QString m_selectedProviderKey;
};

}  // namespace hydra::ui
