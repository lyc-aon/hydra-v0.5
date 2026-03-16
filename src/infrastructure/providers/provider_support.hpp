#pragma once

#include <QString>
#include <QStringList>

namespace hydra::infrastructure::providers::support {

struct ExecutableProbe {
    bool available = false;
    QString executablePath;
    QString versionString;
    QString statusMessage;
};

ExecutableProbe probeExecutable(const QString &command, const QStringList &versionArguments);

}  // namespace hydra::infrastructure::providers::support
