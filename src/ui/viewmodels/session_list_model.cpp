#include "ui/viewmodels/session_list_model.hpp"

#include <QVariantMap>

#include "domain/models/launch_safety.hpp"
#include "domain/models/provider_identity.hpp"
#include "domain/models/session_category.hpp"
#include "domain/models/session_state.hpp"
#include "domain/models/status_provenance.hpp"

namespace hydra::ui {

namespace {

bool isHomeLaunchSession(const hydra::domain::SessionRecord &session)
{
    return session.name == QStringLiteral("Home shell")
           || session.name.startsWith(QStringLiteral("Home ["));
}

QString safetyDisplayName(const QString &launchSafetyKey)
{
    return domain::launchSafetyDisplayName(domain::launchSafetyFromKey(launchSafetyKey));
}

QString activityLabelForSession(const domain::SessionRecord &session,
                                const QVector<domain::SessionTimelineEvent> &timeline)
{
    if (session.state == domain::SessionState::AwaitingApproval) {
        return QStringLiteral("PENDING");
    }
    if (session.state == domain::SessionState::WaitingForInput) {
        return QStringLiteral("INPUT");
    }
    if (session.state == domain::SessionState::RunningTool
        || session.state == domain::SessionState::Thinking)
    {
        return QStringLiteral("ACTIVE");
    }
    if (!timeline.isEmpty()
        && timeline.first().createdAt.secsTo(QDateTime::currentDateTimeUtc()) <= 90
        && timeline.first().signalKind == domain::SessionSignalKind::PaneOutputObserved)
    {
        return QStringLiteral("ACTIVE");
    }

    return QStringLiteral("QUIET");
}

QVariantList timelineEntryMaps(const QVector<domain::SessionTimelineEvent> &timeline)
{
    QVariantList entries;
    entries.reserve(timeline.size());
    for (int i = 0; i < timeline.size(); ++i) {
        const domain::SessionTimelineEvent &event = timeline.at(i);
        QVariantMap map;
        map.insert(QStringLiteral("summary"), event.summary);
        map.insert(QStringLiteral("detail"), event.detail);
        map.insert(QStringLiteral("stateLabel"), domain::sessionStateDisplayName(event.state));
        map.insert(QStringLiteral("stateTone"), domain::sessionStateToneKey(event.state));
        map.insert(QStringLiteral("provenanceLabel"),
                   domain::statusProvenanceDisplayName(event.provenance));
        map.insert(QStringLiteral("provenanceTone"),
                   domain::statusProvenanceToneKey(event.provenance));
        map.insert(QStringLiteral("occurredAtText"),
                   event.createdAt.toLocalTime().toString(QStringLiteral("MMM d, h:mm:ss ap")));
        qint64 durationSecs = 60;
        if (i + 1 < timeline.size()) {
            durationSecs = qMax(static_cast<qint64>(1),
                                timeline.at(i + 1).createdAt.secsTo(event.createdAt));
        }
        map.insert(QStringLiteral("durationSecs"), durationSecs);
        entries.push_back(map);
    }
    return entries;
}

}  // namespace

SessionListModel::SessionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SessionListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_sessions.size();
}

QVariant SessionListModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sessions.size()) {
        return {};
    }

    const SessionEntry &entry = m_sessions.at(index.row());
    switch (role) {
    case SessionIdRole:
        return entry.sessionId;
    case NameRole:
        return entry.name;
    case RepoNameRole:
        return entry.repoName;
    case DetailTextRole:
        return entry.detailText;
    case StatusDetailRole:
        return entry.statusDetail;
    case StateLabelRole:
        return entry.stateLabel;
    case StateToneRole:
        return entry.stateTone;
    case SafetyLabelRole:
        return entry.safetyLabel;
    case SafetyToneRole:
        return entry.safetyTone;
    case ProvenanceLabelRole:
        return entry.provenanceLabel;
    case ProvenanceToneRole:
        return entry.provenanceTone;
    case ActivityLabelRole:
        return entry.activityLabel;
    case ApprovalPendingRole:
        return entry.approvalPending;
    case UpdatedAtTextRole:
        return entry.updatedAtText;
    case TimelineEntriesRole:
        return entry.timelineEntries;
    case CanTerminateRole:
        return entry.canTerminate;
    case SelectedRole:
        return entry.selected;
    case AliasRole:
        return entry.alias;
    case CategoryKeyRole:
        return entry.categoryKey;
    default:
        return {};
    }
}

QHash<int, QByteArray> SessionListModel::roleNames() const
{
    return {
        {SessionIdRole, "sessionId"},
        {NameRole, "name"},
        {RepoNameRole, "repoName"},
        {DetailTextRole, "detailText"},
        {StatusDetailRole, "statusDetail"},
        {StateLabelRole, "stateLabel"},
        {StateToneRole, "stateTone"},
        {SafetyLabelRole, "safetyLabel"},
        {SafetyToneRole, "safetyTone"},
        {ProvenanceLabelRole, "provenanceLabel"},
        {ProvenanceToneRole, "provenanceTone"},
        {ActivityLabelRole, "activityLabel"},
        {ApprovalPendingRole, "approvalPending"},
        {UpdatedAtTextRole, "updatedAtText"},
        {TimelineEntriesRole, "timelineEntries"},
        {CanTerminateRole, "canTerminate"},
        {SelectedRole, "selected"},
        {AliasRole, "alias"},
        {CategoryKeyRole, "categoryKey"},
    };
}

int SessionListModel::sessionCountValue() const
{
    return m_sessions.size();
}

QString SessionListModel::sessionIdAt(const int row) const
{
    if (row < 0 || row >= m_sessions.size()) {
        return {};
    }

    return m_sessions.at(row).sessionId;
}

int SessionListModel::indexOfSessionId(const QString &sessionId) const
{
    if (sessionId.isEmpty()) {
        return -1;
    }

    for (int row = 0; row < m_sessions.size(); ++row) {
        if (m_sessions.at(row).sessionId == sessionId) {
            return row;
        }
    }

    return -1;
}

void SessionListModel::setSessions(const QVector<domain::SessionRecord> &sessions,
                                   const QHash<QString, domain::Repository> &repositoriesById,
                                   const QHash<QString, QVector<domain::SessionTimelineEvent>> &timelineBySession,
                                   const QString &selectedSessionId)
{
    m_selectedSessionId = selectedSessionId;
    QVector<SessionEntry> nextSessions;
    nextSessions.reserve(sessions.size());

    for (const domain::SessionRecord &session : sessions) {
        if (session.state == domain::SessionState::Exited) {
            continue;
        }
        if (session.category == domain::SessionCategory::Router) {
            continue;
        }

        const auto repository = repositoriesById.constFind(session.repoId);

        SessionEntry entry;
        entry.sessionId = session.id;
        entry.name = session.name;
        entry.repoName = isHomeLaunchSession(session)
                             ? QStringLiteral("HOME")
                             : (repository == repositoriesById.cend()
                                    ? QStringLiteral("Unknown repo")
                                    : repository->name);
        entry.detailText = QStringLiteral("%1  |  %2  |  %3")
                               .arg(domain::providerDisplayNameForKey(session.providerKey),
                                    safetyDisplayName(session.launchSafetyKey),
                                    session.workingDirectory);
        entry.statusDetail = session.statusDetail;
        entry.stateLabel = domain::sessionStateDisplayName(session.state);
        entry.stateTone = domain::sessionStateToneKey(session.state);
        entry.safetyLabel = safetyDisplayName(session.launchSafetyKey);
        entry.safetyTone = session.launchSafetyKey == QStringLiteral("bypass")
                               ? QStringLiteral("danger")
                               : QStringLiteral("ready");
        entry.provenanceLabel = domain::statusProvenanceDisplayName(session.statusProvenance);
        entry.provenanceTone = domain::statusProvenanceToneKey(session.statusProvenance);
        const QVector<domain::SessionTimelineEvent> timeline =
            timelineBySession.value(session.id);
        entry.activityLabel = activityLabelForSession(session, timeline);
        entry.approvalPending = session.state == domain::SessionState::AwaitingApproval;
        entry.updatedAtText =
            session.updatedAt.toLocalTime().toString(QStringLiteral("MMM d, h:mm ap"));
        entry.timelineEntries = timelineEntryMaps(timeline);
        entry.canTerminate = !session.tmuxSessionName.isEmpty() &&
                             session.state != domain::SessionState::Exited;
        entry.selected = session.id == m_selectedSessionId;
        entry.alias = session.alias;
        entry.categoryKey = domain::sessionCategoryToKey(session.category);
        nextSessions.push_back(entry);
    }

    auto sameSessionIdentityOrder = [this, &nextSessions]() {
        if (m_sessions.size() != nextSessions.size()) {
            return false;
        }

        for (int index = 0; index < m_sessions.size(); ++index) {
            if (m_sessions.at(index).sessionId != nextSessions.at(index).sessionId) {
                return false;
            }
        }

        return true;
    };
    auto entriesEqual = [](const SessionEntry &lhs, const SessionEntry &rhs) {
        return lhs.sessionId == rhs.sessionId && lhs.name == rhs.name
               && lhs.repoName == rhs.repoName && lhs.detailText == rhs.detailText
               && lhs.statusDetail == rhs.statusDetail && lhs.stateLabel == rhs.stateLabel
               && lhs.stateTone == rhs.stateTone && lhs.safetyLabel == rhs.safetyLabel
               && lhs.safetyTone == rhs.safetyTone
               && lhs.provenanceLabel == rhs.provenanceLabel
               && lhs.provenanceTone == rhs.provenanceTone
               && lhs.activityLabel == rhs.activityLabel
               && lhs.approvalPending == rhs.approvalPending
               && lhs.updatedAtText == rhs.updatedAtText
               && lhs.timelineEntries == rhs.timelineEntries
               && lhs.canTerminate == rhs.canTerminate && lhs.selected == rhs.selected
               && lhs.alias == rhs.alias && lhs.categoryKey == rhs.categoryKey;
    };

    if (sameSessionIdentityOrder()) {
        for (int row = 0; row < nextSessions.size(); ++row) {
            if (entriesEqual(m_sessions.at(row), nextSessions.at(row))) {
                continue;
            }

            m_sessions[row] = nextSessions.at(row);
            const QModelIndex changedIndex = index(row, 0);
            emit dataChanged(changedIndex, changedIndex);
        }
        return;
    }

    beginResetModel();
    m_sessions = nextSessions;
    endResetModel();
}

void SessionListModel::setSelectedSessionId(const QString &selectedSessionId)
{
    if (m_selectedSessionId == selectedSessionId) {
        return;
    }

    m_selectedSessionId = selectedSessionId;
    if (m_sessions.isEmpty()) {
        return;
    }

    QVector<int> changedRows;
    changedRows.reserve(m_sessions.size());
    for (int row = 0; row < m_sessions.size(); ++row) {
        const bool nextSelected = m_sessions.at(row).sessionId == m_selectedSessionId;
        if (m_sessions.at(row).selected == nextSelected) {
            continue;
        }

        changedRows.push_back(row);
        m_sessions[row].selected = nextSelected;
    }

    if (changedRows.isEmpty()) {
        return;
    }

    for (const int row : changedRows) {
        const QModelIndex changedIndex = index(row, 0);
        emit dataChanged(changedIndex, changedIndex, {SelectedRole});
    }
}

}  // namespace hydra::ui
