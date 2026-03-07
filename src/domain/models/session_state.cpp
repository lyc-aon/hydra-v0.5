#include "domain/models/session_state.hpp"

namespace hydra::domain {

QString sessionStateToStorage(const SessionState state)
{
    switch (state) {
    case SessionState::Starting:
        return QStringLiteral("starting");
    case SessionState::Idle:
        return QStringLiteral("idle");
    case SessionState::Thinking:
        return QStringLiteral("thinking");
    case SessionState::RunningTool:
        return QStringLiteral("running_tool");
    case SessionState::AwaitingApproval:
        return QStringLiteral("awaiting_approval");
    case SessionState::WaitingForInput:
        return QStringLiteral("waiting_for_input");
    case SessionState::Backgrounded:
        return QStringLiteral("backgrounded");
    case SessionState::Exited:
        return QStringLiteral("exited");
    case SessionState::Error:
        return QStringLiteral("error");
    }

    return QStringLiteral("error");
}

SessionState sessionStateFromStorage(const QString &value)
{
    if (value == QStringLiteral("starting")) {
        return SessionState::Starting;
    }
    if (value == QStringLiteral("idle")) {
        return SessionState::Idle;
    }
    if (value == QStringLiteral("thinking")) {
        return SessionState::Thinking;
    }
    if (value == QStringLiteral("running_tool")) {
        return SessionState::RunningTool;
    }
    if (value == QStringLiteral("awaiting_approval")) {
        return SessionState::AwaitingApproval;
    }
    if (value == QStringLiteral("waiting_for_input")) {
        return SessionState::WaitingForInput;
    }
    if (value == QStringLiteral("backgrounded")) {
        return SessionState::Backgrounded;
    }
    if (value == QStringLiteral("exited")) {
        return SessionState::Exited;
    }
    return SessionState::Error;
}

QString sessionStateDisplayName(const SessionState state)
{
    switch (state) {
    case SessionState::Starting:
        return QStringLiteral("Starting");
    case SessionState::Idle:
        return QStringLiteral("Idle");
    case SessionState::Thinking:
        return QStringLiteral("Thinking");
    case SessionState::RunningTool:
        return QStringLiteral("Running Tool");
    case SessionState::AwaitingApproval:
        return QStringLiteral("Awaiting Approval");
    case SessionState::WaitingForInput:
        return QStringLiteral("Waiting For Input");
    case SessionState::Backgrounded:
        return QStringLiteral("Backgrounded");
    case SessionState::Exited:
        return QStringLiteral("Exited");
    case SessionState::Error:
        return QStringLiteral("Error");
    }

    return QStringLiteral("Error");
}

QString sessionStateAccentColor(const SessionState state)
{
    switch (state) {
    case SessionState::Starting:
        return QStringLiteral("#c98d43");
    case SessionState::Idle:
        return QStringLiteral("#3f7d58");
    case SessionState::Thinking:
        return QStringLiteral("#4d7599");
    case SessionState::RunningTool:
        return QStringLiteral("#1f5f8b");
    case SessionState::AwaitingApproval:
        return QStringLiteral("#b55d3d");
    case SessionState::WaitingForInput:
        return QStringLiteral("#8f6b1e");
    case SessionState::Backgrounded:
        return QStringLiteral("#5f646d");
    case SessionState::Exited:
        return QStringLiteral("#70655f");
    case SessionState::Error:
        return QStringLiteral("#ad3f3f");
    }

    return QStringLiteral("#ad3f3f");
}

}  // namespace hydra::domain
