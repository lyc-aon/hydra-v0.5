#pragma once

#include <QDateTime>
#include <QString>

#include "domain/models/session_signal.hpp"
#include "domain/models/session_state.hpp"
#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

struct SessionTimelineEvent {
    QString id;
    QString sessionId;
    SessionSignalKind signalKind = SessionSignalKind::ManualRefresh;
    SessionState state = SessionState::Idle;
    QString summary;
    QString detail;
    StatusProvenance provenance = StatusProvenance::Derived;
    QString sourceRef;
    QDateTime createdAt;
};

inline bool operator==(const SessionTimelineEvent &lhs, const SessionTimelineEvent &rhs)
{
    return lhs.id == rhs.id && lhs.sessionId == rhs.sessionId
           && lhs.signalKind == rhs.signalKind && lhs.state == rhs.state
           && lhs.summary == rhs.summary && lhs.detail == rhs.detail
           && lhs.provenance == rhs.provenance && lhs.sourceRef == rhs.sourceRef
           && lhs.createdAt == rhs.createdAt;
}

}  // namespace hydra::domain
