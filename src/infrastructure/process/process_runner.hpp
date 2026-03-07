#pragma once

#include <QString>
#include <QStringList>

namespace hydra::infrastructure::process {

struct ProcessResult {
    bool ok = false;
    int exitCode = -1;
    QString standardOutput;
    QString standardError;
};

[[nodiscard]] ProcessResult runProcess(const QString &executable,
                                       const QStringList &arguments,
                                       const QString &workingDirectory = QString(),
                                       int startTimeoutMs = 3000,
                                       int finishTimeoutMs = 5000);

}  // namespace hydra::infrastructure::process
