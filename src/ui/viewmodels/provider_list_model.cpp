#include "ui/viewmodels/provider_list_model.hpp"

namespace hydra::ui {

ProviderListModel::ProviderListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ProviderListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_providers.size();
}

QVariant ProviderListModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_providers.size()) {
        return {};
    }

    const ProviderEntry &entry = m_providers.at(index.row());
    switch (role) {
    case ProviderKeyRole:
        return entry.providerKey;
    case DisplayNameRole:
        return entry.displayName;
    case AvailableRole:
        return entry.available;
    case StatusMessageRole:
        return entry.statusMessage;
    case SelectedRole:
        return entry.selected;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProviderListModel::roleNames() const
{
    return {
        {ProviderKeyRole, "providerKey"},
        {DisplayNameRole, "displayName"},
        {AvailableRole, "available"},
        {StatusMessageRole, "statusMessage"},
        {SelectedRole, "selected"},
    };
}

void ProviderListModel::setProviders(const QVector<domain::ports::ProviderProbe> &providers)
{
    QVector<ProviderEntry> nextProviders;
    nextProviders.reserve(providers.size());

    for (const domain::ports::ProviderProbe &provider : providers) {
        nextProviders.push_back({
            .providerKey = provider.providerKey,
            .displayName = provider.displayName,
            .available = provider.available,
            .statusMessage = provider.statusMessage,
            .selected = provider.providerKey == m_selectedProviderKey,
        });
    }

    beginResetModel();
    m_providers = nextProviders;
    endResetModel();
}

void ProviderListModel::setSelectedProviderKey(const QString &providerKey)
{
    if (m_selectedProviderKey == providerKey) {
        return;
    }

    m_selectedProviderKey = providerKey;

    beginResetModel();
    for (ProviderEntry &entry : m_providers) {
        entry.selected = entry.providerKey == m_selectedProviderKey;
    }
    endResetModel();
}

}  // namespace hydra::ui
