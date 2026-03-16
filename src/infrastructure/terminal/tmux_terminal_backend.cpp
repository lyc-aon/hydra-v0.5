#include "infrastructure/terminal/tmux_terminal_backend.hpp"

#include <algorithm>

#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUuid>

namespace hydra::infrastructure {

namespace {

struct TerminalProgram {
    QString executable;
    QStringList prefixArguments;
};

QString commandErrorDetail(const process::ProcessResult &result)
{
    if (!result.standardError.trimmed().isEmpty()) {
        return result.standardError.trimmed();
    }
    if (!result.standardOutput.trimmed().isEmpty()) {
        return result.standardOutput.trimmed();
    }
    return QStringLiteral("command failed");
}

QStringList tmuxClientArguments(const QStringList &arguments)
{
    QStringList effectiveArguments;

    const QString socketPath = qEnvironmentVariable("HYDRA_TMUX_SOCKET_PATH").trimmed();
    if (!socketPath.isEmpty()) {
        effectiveArguments << QStringLiteral("-S") << socketPath;
    } else {
        const QString socketName = qEnvironmentVariable("HYDRA_TMUX_SOCKET_NAME").trimmed();
        if (!socketName.isEmpty()) {
            effectiveArguments << QStringLiteral("-L") << socketName;
        }
    }

    effectiveArguments << arguments;
    return effectiveArguments;
}

}  // namespace

TmuxTerminalBackend::TmuxTerminalBackend()
    : m_tmuxPath(QStandardPaths::findExecutable(QStringLiteral("tmux")))
{
}

bool TmuxTerminalBackend::pasteText(const QString &target,
                                    const QString &text,
                                    QString *errorMessage) const
{
    if (target.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux target is selected.");
        }
        return false;
    }
    if (text.isEmpty()) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    QString bufferName;
    if (!loadTextBuffer(text, &bufferName, errorMessage)) {
        return false;
    }

    QString localError;
    const bool pasted =
        commandSucceeded(runTmuxCommand({QStringLiteral("paste-buffer"),
                                         QStringLiteral("-d"),
                                         QStringLiteral("-p"),
                                         QStringLiteral("-r"),
                                         QStringLiteral("-b"),
                                         bufferName,
                                         QStringLiteral("-t"),
                                         target}),
                         &localError);
    if (!pasted) {
        runTmuxCommand({QStringLiteral("delete-buffer"), QStringLiteral("-b"), bufferName});
        if (errorMessage != nullptr) {
            *errorMessage = localError;
        }
        return false;
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool TmuxTerminalBackend::pasteTextAndSubmit(const QString &target,
                                             const QString &text,
                                             QString *errorMessage) const
{
    if (target.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux target is selected.");
        }
        return false;
    }
    if (text.isEmpty()) {
        return sendSpecialKey(target, QStringLiteral("Enter"), 1, errorMessage);
    }

    QString bufferName;
    if (!loadTextBuffer(text, &bufferName, errorMessage)) {
        return false;
    }

    QString localError;
    const bool submitted =
        commandSucceeded(runTmuxCommand({QStringLiteral("paste-buffer"),
                                         QStringLiteral("-d"),
                                         QStringLiteral("-p"),
                                         QStringLiteral("-r"),
                                         QStringLiteral("-b"),
                                         bufferName,
                                         QStringLiteral("-t"),
                                         target,
                                         QStringLiteral(";"),
                                         QStringLiteral("send-keys"),
                                         QStringLiteral("-t"),
                                         target,
                                         QStringLiteral("Enter")}),
                         &localError);
    if (!submitted) {
        runTmuxCommand({QStringLiteral("delete-buffer"), QStringLiteral("-b"), bufferName});
        if (errorMessage != nullptr) {
            *errorMessage = localError;
        }
        return false;
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool TmuxTerminalBackend::loadTextBuffer(const QString &text,
                                         QString *bufferName,
                                         QString *errorMessage) const
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(true);
    if (!tempFile.open()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not create a temporary paste buffer file.");
        }
        return false;
    }

    const QByteArray utf8 = text.toUtf8();
    qint64 totalWritten = 0;
    while (totalWritten < utf8.size()) {
        const qint64 written =
            tempFile.write(utf8.constData() + totalWritten, utf8.size() - totalWritten);
        if (written <= 0) {
            break;
        }
        totalWritten += written;
    }

    if (totalWritten != utf8.size() || !tempFile.flush()) {
        if (errorMessage != nullptr) {
            const QString detail = tempFile.errorString().trimmed();
            *errorMessage = detail.isEmpty()
                                ? QStringLiteral("Could not write clipboard contents for tmux paste.")
                                : QStringLiteral("Could not write clipboard contents for tmux paste: %1")
                                      .arg(detail);
        }
        return false;
    }

    const QString localBufferName =
        QStringLiteral("hydra-paste-%1").arg(QUuid::createUuid().toString(QUuid::Id128));
    if (!commandSucceeded(runTmuxCommand({QStringLiteral("load-buffer"),
                                          QStringLiteral("-b"),
                                          localBufferName,
                                          tempFile.fileName()}),
                          errorMessage))
    {
        return false;
    }

    if (bufferName != nullptr) {
        *bufferName = localBufferName;
    }
    return true;
}

bool TmuxTerminalBackend::sendSpecialKey(const QString &target,
                                         const QString &keySequence,
                                         const int repeatCount,
                                         QString *errorMessage) const
{
    if (target.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux target is selected.");
        }
        return false;
    }
    if (keySequence.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    QStringList arguments = {
        QStringLiteral("send-keys"),
        QStringLiteral("-t"),
        target,
    };
    if (repeatCount > 1) {
        arguments << QStringLiteral("-N") << QString::number(repeatCount);
    }
    arguments << keySequence;
    return commandSucceeded(runTmuxCommand(arguments), errorMessage);
}

bool TmuxTerminalBackend::prepareInteractiveAttach(const QString &sessionName,
                                                   const bool enableMouse,
                                                   QString *errorMessage) const
{
    if (sessionName.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux session is selected.");
        }
        return false;
    }

    const bool mouseReady =
        commandSucceeded(runTmuxCommand({QStringLiteral("set-option"),
                                         QStringLiteral("-t"),
                                         sessionName,
                                         QStringLiteral("mouse"),
                                         enableMouse ? QStringLiteral("on")
                                                     : QStringLiteral("off")}),
                         errorMessage);
    if (!mouseReady) {
        return false;
    }

    return commandSucceeded(runTmuxCommand({QStringLiteral("set-option"),
                                            QStringLiteral("-t"),
                                            sessionName,
                                            QStringLiteral("status"),
                                            QStringLiteral("off")}),
                            errorMessage);
}

bool TmuxTerminalBackend::scrollHistory(const QString &target,
                                        const int lineDelta,
                                        QString *errorMessage) const
{
    if (target.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux target is selected.");
        }
        return false;
    }

    if (lineDelta == 0) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    ScrollState state = queryScrollState(target, errorMessage);
    if (errorMessage != nullptr && !errorMessage->isEmpty()) {
        return false;
    }

    const int lines = std::max(1, std::abs(lineDelta));
    QString localError;
    if (lineDelta > 0) {
        if (!state.inCopyMode
            && !commandSucceeded(runTmuxCommand({QStringLiteral("copy-mode"),
                                                 QStringLiteral("-e"),
                                                 QStringLiteral("-t"),
                                                 target}),
                                 &localError))
        {
            if (errorMessage != nullptr) {
                *errorMessage = localError;
            }
            return false;
        }

        const bool ok = commandSucceeded(runTmuxCommand({QStringLiteral("send-keys"),
                                                         QStringLiteral("-X"),
                                                         QStringLiteral("-N"),
                                                         QString::number(lines),
                                                         QStringLiteral("-t"),
                                                         target,
                                                         QStringLiteral("scroll-up")}),
                                         &localError);
        if (errorMessage != nullptr) {
            *errorMessage = ok ? QString() : localError;
        }
        return ok;
    }

    if (!state.inCopyMode) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    if (!commandSucceeded(runTmuxCommand({QStringLiteral("send-keys"),
                                          QStringLiteral("-X"),
                                          QStringLiteral("-N"),
                                          QString::number(lines),
                                          QStringLiteral("-t"),
                                          target,
                                          QStringLiteral("scroll-down")}),
                          &localError))
    {
        if (errorMessage != nullptr) {
            *errorMessage = localError;
        }
        return false;
    }

    state = queryScrollState(target, &localError);
    if (!localError.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = localError;
        }
        return false;
    }

    if (state.inCopyMode && state.scrollPosition <= 0) {
        commandSucceeded(runTmuxCommand({QStringLiteral("send-keys"),
                                         QStringLiteral("-X"),
                                         QStringLiteral("-t"),
                                         target,
                                         QStringLiteral("cancel")}),
                         nullptr);
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool TmuxTerminalBackend::openExternalAttach(const QString &sessionName,
                                             const QString &workingDirectory,
                                             QString *errorMessage) const
{
    if (m_tmuxPath.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("tmux is not on PATH.");
        }
        return false;
    }
    if (sessionName.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("No tmux session is selected.");
        }
        return false;
    }

#if defined(Q_OS_MACOS)
    const QString tmuxCommand = QStringLiteral("%1 attach -t %2")
                                    .arg(m_tmuxPath, sessionName);
    const QString script =
        QStringLiteral("tell application \"Terminal\" to activate\n"
                       "tell application \"Terminal\" to do script %1")
            .arg(QStringLiteral("\"%1\"").arg(tmuxCommand));
    if (QProcess::startDetached(QStringLiteral("/usr/bin/osascript"),
                                {QStringLiteral("-e"), script},
                                workingDirectory))
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }
#else
    const QVector<TerminalProgram> candidates = {
        {QStringLiteral("x-terminal-emulator"), {QStringLiteral("-e")}},
        {QStringLiteral("kgx"), {QStringLiteral("--")}},
        {QStringLiteral("gnome-terminal"), {QStringLiteral("--")}},
        {QStringLiteral("ptyxis"), {QStringLiteral("--")}},
        {QStringLiteral("konsole"), {QStringLiteral("-e")}},
        {QStringLiteral("xterm"), {QStringLiteral("-e")}},
        {QStringLiteral("kitty"), {}},
        {QStringLiteral("wezterm"), {QStringLiteral("start"), QStringLiteral("--")}},
    };

    for (const TerminalProgram &candidate : candidates) {
        const QString executable = QStandardPaths::findExecutable(candidate.executable);
        if (executable.isEmpty()) {
            continue;
        }

        QStringList arguments = candidate.prefixArguments;
        arguments << m_tmuxPath << tmuxClientArguments({QStringLiteral("attach"),
                                                       QStringLiteral("-t"),
                                                       sessionName});
        if (QProcess::startDetached(executable,
                                    arguments,
                                    workingDirectory.isEmpty() ? QDir::homePath()
                                                               : workingDirectory))
        {
            if (errorMessage != nullptr) {
                errorMessage->clear();
            }
            return true;
        }
    }
#endif

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("No supported external terminal launcher was found.");
    }
    return false;
}

TmuxTerminalBackend::CommandResult TmuxTerminalBackend::runTmuxCommand(
    const QStringList &arguments) const
{
    if (m_tmuxPath.isEmpty()) {
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = QStringLiteral("tmux is unavailable")};
    }

    CommandResult result = process::runProcess(m_tmuxPath, tmuxClientArguments(arguments));
    if (!result.ok && result.standardError == QStringLiteral("process timed out")) {
        result.standardError = QStringLiteral("tmux command timed out");
    }
    return result;
}

TmuxTerminalBackend::ScrollState TmuxTerminalBackend::queryScrollState(
    const QString &target,
    QString *errorMessage) const
{
    ScrollState state;
    const CommandResult result =
        runTmuxCommand({QStringLiteral("display-message"),
                        QStringLiteral("-p"),
                        QStringLiteral("-t"),
                        target,
                        QStringLiteral("#{pane_in_mode} #{scroll_position}")});
    if (!result.ok) {
        if (errorMessage != nullptr) {
            *errorMessage = commandErrorDetail(result);
        }
        return state;
    }

    const QStringList parts =
        result.standardOutput.trimmed().split(QChar::Space, Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        state.inCopyMode = parts.constFirst() == QStringLiteral("1");
    }
    if (parts.size() >= 2) {
        bool ok = false;
        const int parsed = parts.at(1).toInt(&ok);
        state.scrollPosition = ok ? parsed : 0;
    }
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return state;
}

bool TmuxTerminalBackend::commandSucceeded(const CommandResult &result, QString *errorMessage) const
{
    if (result.ok) {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = commandErrorDetail(result);
    }
    return false;
}

}  // namespace hydra::infrastructure
