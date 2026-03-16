#include "domain/models/session_signal.hpp"

namespace hydra::domain {

QString sessionSignalKindToStorage(const SessionSignalKind kind)
{
    switch (kind) {
    case SessionSignalKind::LaunchRequested:
        return QStringLiteral("launch-requested");
    case SessionSignalKind::TerminateRequested:
        return QStringLiteral("terminate-requested");
    case SessionSignalKind::BootstrapLive:
        return QStringLiteral("bootstrap-live");
    case SessionSignalKind::BootstrapExited:
        return QStringLiteral("bootstrap-exited");
    case SessionSignalKind::MonitorAttached:
        return QStringLiteral("monitor-attached");
    case SessionSignalKind::PaneOutputObserved:
        return QStringLiteral("pane-output");
    case SessionSignalKind::PromptDetected:
        return QStringLiteral("prompt-detected");
    case SessionSignalKind::ApprovalRequested:
        return QStringLiteral("approval-requested");
    case SessionSignalKind::WaitingForInputDetected:
        return QStringLiteral("waiting-for-input");
    case SessionSignalKind::InputSubmitted:
        return QStringLiteral("input-submitted");
    case SessionSignalKind::SessionExited:
        return QStringLiteral("session-exited");
    case SessionSignalKind::MonitorError:
        return QStringLiteral("monitor-error");
    case SessionSignalKind::ManualRefresh:
        return QStringLiteral("manual-refresh");
    }

    return QStringLiteral("manual-refresh");
}

SessionSignalKind sessionSignalKindFromStorage(const QString &value)
{
    if (value == QStringLiteral("launch-requested")) {
        return SessionSignalKind::LaunchRequested;
    }
    if (value == QStringLiteral("terminate-requested")) {
        return SessionSignalKind::TerminateRequested;
    }
    if (value == QStringLiteral("bootstrap-live")) {
        return SessionSignalKind::BootstrapLive;
    }
    if (value == QStringLiteral("bootstrap-exited")) {
        return SessionSignalKind::BootstrapExited;
    }
    if (value == QStringLiteral("monitor-attached")) {
        return SessionSignalKind::MonitorAttached;
    }
    if (value == QStringLiteral("pane-output")) {
        return SessionSignalKind::PaneOutputObserved;
    }
    if (value == QStringLiteral("prompt-detected")) {
        return SessionSignalKind::PromptDetected;
    }
    if (value == QStringLiteral("approval-requested")) {
        return SessionSignalKind::ApprovalRequested;
    }
    if (value == QStringLiteral("waiting-for-input")) {
        return SessionSignalKind::WaitingForInputDetected;
    }
    if (value == QStringLiteral("input-submitted")) {
        return SessionSignalKind::InputSubmitted;
    }
    if (value == QStringLiteral("session-exited")) {
        return SessionSignalKind::SessionExited;
    }
    if (value == QStringLiteral("monitor-error")) {
        return SessionSignalKind::MonitorError;
    }

    return SessionSignalKind::ManualRefresh;
}

QString sessionSignalKindDisplayName(const SessionSignalKind kind)
{
    switch (kind) {
    case SessionSignalKind::LaunchRequested:
        return QStringLiteral("Launch");
    case SessionSignalKind::TerminateRequested:
        return QStringLiteral("Terminate");
    case SessionSignalKind::BootstrapLive:
        return QStringLiteral("Live");
    case SessionSignalKind::BootstrapExited:
        return QStringLiteral("Exited");
    case SessionSignalKind::MonitorAttached:
        return QStringLiteral("Monitor attached");
    case SessionSignalKind::PaneOutputObserved:
        return QStringLiteral("Activity");
    case SessionSignalKind::PromptDetected:
        return QStringLiteral("Prompt ready");
    case SessionSignalKind::ApprovalRequested:
        return QStringLiteral("Approval requested");
    case SessionSignalKind::WaitingForInputDetected:
        return QStringLiteral("Waiting for input");
    case SessionSignalKind::InputSubmitted:
        return QStringLiteral("Input submitted");
    case SessionSignalKind::SessionExited:
        return QStringLiteral("Session exited");
    case SessionSignalKind::MonitorError:
        return QStringLiteral("Monitor error");
    case SessionSignalKind::ManualRefresh:
        return QStringLiteral("Manual refresh");
    }

    return QStringLiteral("Manual refresh");
}

}  // namespace hydra::domain
