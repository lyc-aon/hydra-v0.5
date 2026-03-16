#pragma once

#include <QDateTime>
#include <QString>

#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

enum class SessionSignalKind {
    LaunchRequested,
    TerminateRequested,
    BootstrapLive,
    BootstrapExited,
    MonitorAttached,
    PaneOutputObserved,
    PromptDetected,
    ApprovalRequested,
    WaitingForInputDetected,
    InputSubmitted,
    SessionExited,
    MonitorError,
    ManualRefresh,
};

struct SessionSignal {
    QString sessionId;
    QString sessionName;
    QString paneId;
    SessionSignalKind kind = SessionSignalKind::ManualRefresh;
    QString payload;
    QString paneTail;
    QString paneTitle;
    QDateTime occurredAt;
    StatusProvenance provenance = StatusProvenance::Derived;
    QString sourceRef;
};

QString sessionSignalKindToStorage(SessionSignalKind kind);
SessionSignalKind sessionSignalKindFromStorage(const QString &value);
QString sessionSignalKindDisplayName(SessionSignalKind kind);

}  // namespace hydra::domain
