#include "infrastructure/providers/provider_support.hpp"

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>

#include "infrastructure/process/process_runner.hpp"

namespace hydra::infrastructure::providers::support {

namespace {

constexpr qint64 kProbeCacheTtlMs = 10000;

struct CachedExecutableProbe {
    ExecutableProbe probe;
    QDateTime capturedAt;
};

QString firstNonEmptyLine(const QString &text)
{
    const QStringList lines = text.split('\n');
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            return trimmed;
        }
    }

    return QString();
}

}  // namespace

ExecutableProbe probeExecutable(const QString &command, const QStringList &versionArguments)
{
    const QString cacheKey = QStringLiteral("%1\0%2").arg(command, versionArguments.join(u'\x1f'));
    static QMutex cacheMutex;
    static QHash<QString, CachedExecutableProbe> cache;
    {
        QMutexLocker locker(&cacheMutex);
        const auto cached = cache.constFind(cacheKey);
        const QDateTime now = QDateTime::currentDateTimeUtc();
        if (cached != cache.cend() && cached->capturedAt.isValid()
            && cached->capturedAt.msecsTo(now) <= kProbeCacheTtlMs)
        {
            return cached->probe;
        }
    }

    ExecutableProbe probe;
    const QString executablePath = QStandardPaths::findExecutable(command);
    if (executablePath.isEmpty()) {
        probe = {.available = false,
                 .executablePath = QString(),
                 .versionString = QString(),
                 .statusMessage = QStringLiteral("%1 is not on PATH.").arg(command)};
    } else {
        const process::ProcessResult versionResult =
            process::runProcess(executablePath, versionArguments);
        const QString versionText =
            firstNonEmptyLine(versionResult.standardOutput.isEmpty() ? versionResult.standardError
                                                                     : versionResult.standardOutput);

        if (!versionResult.ok) {
            const QString failureText = versionText.isEmpty()
                                            ? QStringLiteral("%1 exists on PATH but the version probe failed.")
                                                  .arg(command)
                                            : QStringLiteral("%1 exists on PATH but the version probe failed: %2")
                                                  .arg(command, versionText);
            probe = {.available = false,
                     .executablePath = executablePath,
                     .versionString = versionText,
                     .statusMessage = failureText};
        } else {
            probe = {.available = true,
                     .executablePath = executablePath,
                     .versionString = versionText,
                     .statusMessage = QStringLiteral("%1 is available.").arg(command)};
        }
    }

    {
        QMutexLocker locker(&cacheMutex);
        cache.insert(cacheKey,
                     CachedExecutableProbe{
                         .probe = probe,
                         .capturedAt = QDateTime::currentDateTimeUtc(),
                     });
    }
    return probe;
}

}  // namespace hydra::infrastructure::providers::support
