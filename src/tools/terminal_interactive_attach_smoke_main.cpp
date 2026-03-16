#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QUuid>

#include <iostream>

#include "domain/ports/mux_adapter.hpp"
#include "infrastructure/mux/tmux_adapter.hpp"
#include "infrastructure/terminal/tmux_terminal_backend.hpp"

namespace {

QString runTmuxQuery(const QStringList &arguments, QString *errorMessage)
{
    QProcess process;
    process.start(QStringLiteral("tmux"), arguments);
    if (!process.waitForFinished(5000)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("tmux query timed out.");
        }
        return {};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage != nullptr) {
            const QString stderrText = QString::fromUtf8(process.readAllStandardError()).trimmed();
            *errorMessage = stderrText.isEmpty() ? QStringLiteral("tmux query failed.") : stderrText;
        }
        return {};
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

bool require(bool condition, const QString &message)
{
    if (condition) {
        return true;
    }

    std::cerr << message.toStdString() << '\n';
    return false;
}

}  // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QString socketName =
        QStringLiteral("hydra-interactive-%1").arg(QUuid::createUuid().toString(QUuid::Id128));
    qputenv("HYDRA_TMUX_SOCKET_NAME", socketName.toUtf8());

    hydra::infrastructure::TmuxAdapter tmuxAdapter;
    hydra::infrastructure::TmuxTerminalBackend terminalBackend;

    const QString sessionName =
        QStringLiteral("hydra-scroll-%1").arg(QUuid::createUuid().toString(QUuid::Id128).first(8));

    const hydra::domain::ports::MuxLaunchRequest request{
        .sessionName = sessionName,
        .workingDirectory = QDir::homePath(),
        .executable = QStringLiteral("/bin/sh"),
        .arguments = {QStringLiteral("-lc"),
                      QStringLiteral("printf 'line-1\\nline-2\\nline-3\\n'; exec sleep 30")},
        .environment = {},
    };

    const auto launch = tmuxAdapter.launchDetachedSession(request);
    if (!require(launch.ok, QStringLiteral("Failed to launch tmux smoke session: %1").arg(launch.errorMessage))) {
        return 1;
    }

    const auto cleanup = [&]() {
        tmuxAdapter.terminateSession(sessionName);
    };

    QString errorMessage;
    if (!require(terminalBackend.prepareInteractiveAttach(sessionName, false, &errorMessage),
                 QStringLiteral("prepareInteractiveAttach failed: %1").arg(errorMessage)))
    {
        cleanup();
        return 1;
    }

    QString queryError;
    const QString mouseSetting =
        runTmuxQuery({QStringLiteral("-L"),
                      socketName,
                      QStringLiteral("show-options"),
                      QStringLiteral("-v"),
                      QStringLiteral("-t"),
                      sessionName,
                      QStringLiteral("mouse")},
                     &queryError);
    if (!require(queryError.isEmpty(), queryError)) {
        cleanup();
        return 1;
    }
    if (!require(mouseSetting == QStringLiteral("off"),
                 QStringLiteral("Expected tmux mouse=off for interactive attach, got '%1'.").arg(mouseSetting)))
    {
        cleanup();
        return 1;
    }

    const QString statusSetting =
        runTmuxQuery({QStringLiteral("-L"),
                      socketName,
                      QStringLiteral("show-options"),
                      QStringLiteral("-v"),
                      QStringLiteral("-t"),
                      sessionName,
                      QStringLiteral("status")},
                     &queryError);
    if (!require(queryError.isEmpty(), queryError)) {
        cleanup();
        return 1;
    }
    if (!require(statusSetting == QStringLiteral("off"),
                 QStringLiteral("Expected tmux status=off for interactive attach, got '%1'.").arg(statusSetting)))
    {
        cleanup();
        return 1;
    }

    if (!require(terminalBackend.scrollHistory(sessionName, 5, &errorMessage),
                 QStringLiteral("scrollHistory(up) failed: %1").arg(errorMessage)))
    {
        cleanup();
        return 1;
    }

    const QString scrollUpState =
        runTmuxQuery({QStringLiteral("-L"),
                      socketName,
                      QStringLiteral("display-message"),
                      QStringLiteral("-p"),
                      QStringLiteral("-t"),
                      sessionName,
                      QStringLiteral("#{pane_in_mode} #{scroll_position}")},
                     &queryError);
    if (!require(queryError.isEmpty(), queryError)) {
        cleanup();
        return 1;
    }
    if (!require(scrollUpState.startsWith(QStringLiteral("1 ")),
                 QStringLiteral("Expected copy mode after scroll up, got '%1'.").arg(scrollUpState)))
    {
        cleanup();
        return 1;
    }

    if (!require(terminalBackend.scrollHistory(sessionName, -5, &errorMessage),
                 QStringLiteral("scrollHistory(down) failed: %1").arg(errorMessage)))
    {
        cleanup();
        return 1;
    }

    const QString scrollDownState =
        runTmuxQuery({QStringLiteral("-L"),
                      socketName,
                      QStringLiteral("display-message"),
                      QStringLiteral("-p"),
                      QStringLiteral("-t"),
                      sessionName,
                      QStringLiteral("#{pane_in_mode} #{scroll_position}")},
                     &queryError);
    if (!require(queryError.isEmpty(), queryError)) {
        cleanup();
        return 1;
    }
    if (!require(scrollDownState == QStringLiteral("0"),
                 QStringLiteral("Expected scroll down to return to live mode, got '%1'.").arg(scrollDownState)))
    {
        cleanup();
        return 1;
    }

    cleanup();
    return 0;
}
