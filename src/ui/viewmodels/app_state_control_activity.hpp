#pragma once

#include <QDateTime>
#include <QHash>
#include <QVector>

#include "domain/models/session_record.hpp"
#include "domain/models/session_timeline_event.hpp"

namespace hydra::ui {

inline QString activityLabelForControlSession(
    const domain::SessionRecord *session,
    const QHash<QString, QVector<domain::SessionTimelineEvent>> &timelineBySession)
{
    if (session == nullptr) {
        return QStringLiteral("QUIET");
    }

    if (session->state == domain::SessionState::AwaitingApproval) {
        return QStringLiteral("PENDING");
    }
    if (session->state == domain::SessionState::WaitingForInput) {
        return QStringLiteral("INPUT");
    }
    if (session->state == domain::SessionState::RunningTool
        || session->state == domain::SessionState::Thinking)
    {
        return QStringLiteral("ACTIVE");
    }

    const QVector<domain::SessionTimelineEvent> timeline = timelineBySession.value(session->id);
    if (!timeline.isEmpty()
        && timeline.first().createdAt.secsTo(QDateTime::currentDateTimeUtc()) <= 90
        && timeline.first().signalKind == domain::SessionSignalKind::PaneOutputObserved)
    {
        return QStringLiteral("ACTIVE");
    }

    return QStringLiteral("QUIET");
}

}  // namespace hydra::ui
