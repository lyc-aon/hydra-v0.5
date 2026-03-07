#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/models/session_record.hpp"

namespace hydra::ui {

class SessionListModel final : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        SessionIdRole = Qt::UserRole + 1,
        NameRole,
        RepoNameRole,
        DetailTextRole,
        StateLabelRole,
        StateColorRole,
        UpdatedAtTextRole,
        CanTerminateRole,
    };

    explicit SessionListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSessions(const QVector<domain::SessionRecord> &sessions,
                     const QHash<QString, domain::Repository> &repositoriesById);

private:
    struct SessionEntry {
        QString sessionId;
        QString name;
        QString repoName;
        QString detailText;
        QString stateLabel;
        QString stateColor;
        QString updatedAtText;
        bool canTerminate = false;
    };

    QVector<SessionEntry> m_sessions;
};

}  // namespace hydra::ui
