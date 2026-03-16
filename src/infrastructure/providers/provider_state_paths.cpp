#include "infrastructure/providers/provider_state_paths.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace hydra::infrastructure {

namespace {

QString preferredHydraStateBasePath()
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (root.trimmed().isEmpty()) {
        root = QDir::homePath() + QStringLiteral("/.local/share");
    }

    return QDir(root).filePath(QStringLiteral("hydra"));
}

}  // namespace

QString providerStateRootPath(const QString &providerKey)
{
    const QString trimmedProviderKey = providerKey.trimmed();
    if (trimmedProviderKey.isEmpty()) {
        return {};
    }

    const QString preferredPath = QDir(preferredHydraStateBasePath())
                                      .filePath(QStringLiteral("providers/%1").arg(trimmedProviderKey));
    return preferredPath;
}

}  // namespace hydra::infrastructure
