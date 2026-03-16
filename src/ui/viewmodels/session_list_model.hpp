#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QVariantList>
#include <QVector>

#include "domain/models/repository.hpp"
#include "domain/models/session_record.hpp"
#include "domain/models/session_timeline_event.hpp"

namespace hydra::ui {

class SessionListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        SessionIdRole = Qt::UserRole + 1,
        NameRole,
        RepoNameRole,
        DetailTextRole,
        StatusDetailRole,
        StateLabelRole,
        StateToneRole,
        SafetyLabelRole,
        SafetyToneRole,
        ProvenanceLabelRole,
        ProvenanceToneRole,
        ActivityLabelRole,
        ApprovalPendingRole,
        UpdatedAtTextRole,
        TimelineEntriesRole,
        CanTerminateRole,
        SelectedRole,
        AliasRole,
        CategoryKeyRole,
    };

    explicit SessionListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE int sessionCountValue() const;
    Q_INVOKABLE QString sessionIdAt(int row) const;
    Q_INVOKABLE int indexOfSessionId(const QString &sessionId) const;

    void setSessions(const QVector<domain::SessionRecord> &sessions,
                     const QHash<QString, domain::Repository> &repositoriesById,
                     const QHash<QString, QVector<domain::SessionTimelineEvent>> &timelineBySession,
                     const QString &selectedSessionId);
    void setSelectedSessionId(const QString &selectedSessionId);

private:
    struct SessionEntry {
        QString sessionId;
        QString name;
        QString repoName;
        QString detailText;
        QString statusDetail;
        QString stateLabel;
        QString stateTone;
        QString safetyLabel;
        QString safetyTone;
        QString provenanceLabel;
        QString provenanceTone;
        QString activityLabel;
        bool approvalPending = false;
        QString updatedAtText;
        QVariantList timelineEntries;
        bool canTerminate = false;
        bool selected = false;
        QString alias;
        QString categoryKey;
    };

    QVector<SessionEntry> m_sessions;
    QString m_selectedSessionId;
};

}  // namespace hydra::ui
