#pragma once

#include <QString>

namespace hydra::domain {

enum class SessionState {
    Starting,
    Idle,
    Thinking,
    RunningTool,
    AwaitingApproval,
    WaitingForInput,
    Backgrounded,
    Exited,
    Error
};

QString sessionStateToStorage(SessionState state);
SessionState sessionStateFromStorage(const QString &value);
QString sessionStateDisplayName(SessionState state);
QString sessionStateAccentColor(SessionState state);

}  // namespace hydra::domain
