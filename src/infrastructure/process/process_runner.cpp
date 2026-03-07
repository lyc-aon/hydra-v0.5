#include "infrastructure/process/process_runner.hpp"

#include <QProcess>

namespace hydra::infrastructure::process {

ProcessResult runProcess(const QString &executable,
                         const QStringList &arguments,
                         const QString &workingDirectory,
                         const int startTimeoutMs,
                         const int finishTimeoutMs)
{
    if (executable.isEmpty()) {
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = QStringLiteral("executable is unavailable")};
    }

    QProcess process;
    if (!workingDirectory.isEmpty()) {
        process.setWorkingDirectory(workingDirectory);
    }

    process.start(executable, arguments);
    if (!process.waitForStarted(startTimeoutMs)) {
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = process.errorString()};
    }

    if (!process.waitForFinished(finishTimeoutMs)) {
        process.kill();
        process.waitForFinished();
        return {.ok = false,
                .exitCode = -1,
                .standardOutput = QString(),
                .standardError = QStringLiteral("process timed out")};
    }

    return {.ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0,
            .exitCode = process.exitCode(),
            .standardOutput = QString::fromUtf8(process.readAllStandardOutput()),
            .standardError = QString::fromUtf8(process.readAllStandardError())};
}

}  // namespace hydra::infrastructure::process
