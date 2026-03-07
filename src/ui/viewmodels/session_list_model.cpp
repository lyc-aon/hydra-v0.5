#include "ui/viewmodels/session_list_model.hpp"

#include "domain/models/session_state.hpp"

namespace hydra::ui {

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
    case StateLabelRole:
        return entry.stateLabel;
    case StateColorRole:
        return entry.stateColor;
    case UpdatedAtTextRole:
        return entry.updatedAtText;
    case CanTerminateRole:
        return entry.canTerminate;
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
        {StateLabelRole, "stateLabel"},
        {StateColorRole, "stateColor"},
        {UpdatedAtTextRole, "updatedAtText"},
        {CanTerminateRole, "canTerminate"},
    };
}

void SessionListModel::setSessions(const QVector<domain::SessionRecord> &sessions,
                                   const QHash<QString, domain::Repository> &repositoriesById)
{
    QVector<SessionEntry> nextSessions;
    nextSessions.reserve(sessions.size());

    for (const domain::SessionRecord &session : sessions) {
        if (session.state == domain::SessionState::Exited) {
            continue;
        }

        const auto repository = repositoriesById.constFind(session.repoId);

        SessionEntry entry;
        entry.sessionId = session.id;
        entry.name = session.name;
        entry.repoName = repository == repositoriesById.cend()
                             ? QStringLiteral("Unknown repo")
                             : repository->name;
        entry.detailText =
            QStringLiteral("%1  |  %2").arg(session.providerKey, session.workingDirectory);
        entry.stateLabel = domain::sessionStateDisplayName(session.state);
        entry.stateColor = domain::sessionStateAccentColor(session.state);
        entry.updatedAtText =
            session.updatedAt.toLocalTime().toString(QStringLiteral("MMM d, h:mm ap"));
        entry.canTerminate = !session.tmuxSessionName.isEmpty() &&
                             session.state != domain::SessionState::Exited;
        nextSessions.push_back(entry);
    }

    beginResetModel();
    m_sessions = nextSessions;
    endResetModel();
}

}  // namespace hydra::ui
