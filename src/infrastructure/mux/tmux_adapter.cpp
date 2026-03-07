#include "infrastructure/mux/tmux_adapter.hpp"

#include <QStandardPaths>

namespace hydra::infrastructure {

TmuxAdapter::TmuxAdapter()
    : m_tmuxPath(QStandardPaths::findExecutable(QStringLiteral("tmux")))
{
}

domain::ports::MuxProbeResult TmuxAdapter::probeAvailability() const
{
    if (m_tmuxPath.isEmpty()) {
        return {.available = false,
                .executablePath = QString(),
                .message = QStringLiteral("tmux is not on PATH.")};
    }

    return {.available = true,
            .executablePath = m_tmuxPath,
            .message = QStringLiteral("tmux is available.")};
}

domain::ports::MuxLaunchResult TmuxAdapter::launchDetachedSession(
    const domain::ports::MuxLaunchRequest &request)
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return {.ok = false,
                .sessionName = QString(),
                .paneId = QString(),
                .errorMessage = probe.message};
    }

    QStringList launchArguments = {
        QStringLiteral("new-session"),
        QStringLiteral("-d"),
        QStringLiteral("-s"),
        request.sessionName,
        QStringLiteral("-c"),
        request.workingDirectory,
    };

    if (!request.shellCommand.isEmpty()) {
        launchArguments.append(request.shellCommand);
    }

    const CommandResult launch = runTmuxCommand(launchArguments);
    if (!launch.ok) {
        return {.ok = false,
                .sessionName = QString(),
                .paneId = QString(),
                .errorMessage = QStringLiteral("tmux new-session failed: %1")
                                    .arg(launch.standardError.trimmed())};
    }

    const CommandResult inspect = runTmuxCommand(
        {QStringLiteral("display-message"),
         QStringLiteral("-p"),
         QStringLiteral("-t"),
         request.sessionName,
         QStringLiteral("#{pane_id}")});

    if (!inspect.ok) {
        return {.ok = true,
                .sessionName = request.sessionName,
                .paneId = QString(),
                .errorMessage = QString()};
    }

    return {.ok = true,
            .sessionName = request.sessionName,
            .paneId = inspect.standardOutput.trimmed(),
            .errorMessage = QString()};
}

domain::ports::MuxTerminateResult TmuxAdapter::terminateSession(const QString &sessionName) const
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return {.ok = false, .errorMessage = probe.message};
    }

    const CommandResult result =
        runTmuxCommand({QStringLiteral("kill-session"), QStringLiteral("-t"), sessionName});
    if (result.ok) {
        return {.ok = true, .errorMessage = QString()};
    }

    const QString detail = result.standardError.trimmed().isEmpty()
                               ? QStringLiteral("tmux kill-session failed")
                               : result.standardError.trimmed();
    return {.ok = false, .errorMessage = detail};
}

bool TmuxAdapter::hasLiveSession(const QString &sessionName) const
{
    const auto probe = probeAvailability();
    if (!probe.available) {
        return false;
    }

    const CommandResult result =
        runTmuxCommand({QStringLiteral("has-session"), QStringLiteral("-t"), sessionName});
    return result.ok && result.exitCode == 0;
}

TmuxAdapter::CommandResult TmuxAdapter::runTmuxCommand(const QStringList &arguments) const
{
    if (m_tmuxPath.isEmpty()) {
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = QStringLiteral("tmux is unavailable")};
    }

    CommandResult result = process::runProcess(m_tmuxPath, arguments);
    if (!result.ok && result.standardError == QStringLiteral("process timed out")) {
        result.standardError = QStringLiteral("tmux command timed out");
    }

    return result;
}

}  // namespace hydra::infrastructure
