#include "ui/viewmodels/resume_list_model.hpp"

namespace hydra::ui {

ResumeListModel::ResumeListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ResumeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_entries.size();
}

QVariant ResumeListModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case SessionIdRole:
        return entry.sessionId;
    case NameRole:
        return entry.name;
    case RepoNameRole:
        return entry.repoName;
    case ProviderNameRole:
        return entry.providerName;
    case DetailTextRole:
        return entry.detailText;
    case StatusDetailRole:
        return entry.statusDetail;
    case UpdatedAtTextRole:
        return entry.updatedAtText;
    default:
        return {};
    }
}

QHash<int, QByteArray> ResumeListModel::roleNames() const
{
    return {
        {SessionIdRole, "sessionId"},
        {NameRole, "name"},
        {RepoNameRole, "repoName"},
        {ProviderNameRole, "providerName"},
        {DetailTextRole, "detailText"},
        {StatusDetailRole, "statusDetail"},
        {UpdatedAtTextRole, "updatedAtText"},
    };
}

QString ResumeListModel::sessionIdAt(const int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }

    return m_entries.at(row).sessionId;
}

void ResumeListModel::setEntries(const QVector<Entry> &entries)
{
    auto sameEntryOrder = [this, &entries]() {
        if (m_entries.size() != entries.size()) {
            return false;
        }

        for (int index = 0; index < m_entries.size(); ++index) {
            if (m_entries.at(index).sessionId != entries.at(index).sessionId) {
                return false;
            }
        }

        return true;
    };
    auto entriesEqual = [](const Entry &lhs, const Entry &rhs) {
        return lhs.sessionId == rhs.sessionId && lhs.name == rhs.name
               && lhs.repoName == rhs.repoName && lhs.providerName == rhs.providerName
               && lhs.detailText == rhs.detailText
               && lhs.statusDetail == rhs.statusDetail
               && lhs.updatedAtText == rhs.updatedAtText;
    };

    if (sameEntryOrder()) {
        for (int row = 0; row < entries.size(); ++row) {
            if (entriesEqual(m_entries.at(row), entries.at(row))) {
                continue;
            }

            m_entries[row] = entries.at(row);
            const QModelIndex changedIndex = index(row, 0);
            emit dataChanged(changedIndex, changedIndex);
        }
        return;
    }

    beginResetModel();
    m_entries = entries;
    endResetModel();
}

}  // namespace hydra::ui
