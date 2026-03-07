#pragma once

#include <QString>
#include <QStringList>

namespace hydra::domain::ports {

class ProviderAdapter {
public:
    virtual ~ProviderAdapter() = default;

    virtual QString providerKey() const = 0;
    virtual QString displayName() const = 0;
    virtual bool isAvailable() const = 0;
    virtual QString versionString() const = 0;
    virtual QStringList supportedAuthLanes() const = 0;
};

}  // namespace hydra::domain::ports
