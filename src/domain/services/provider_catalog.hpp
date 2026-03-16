#pragma once

#include <optional>

#include <QVector>

#include "domain/ports/provider_adapter.hpp"

namespace hydra::domain {

class ProviderCatalog final {
public:
    explicit ProviderCatalog(const QVector<ports::ProviderAdapter *> &adapters);

    QVector<ports::ProviderProbe> placeholderProbes() const;
    QVector<ports::ProviderProbe> probes() const;
    std::optional<ports::ProviderProbe> probeFor(const QString &providerKey) const;
    const ports::ProviderAdapter *adapterFor(const QString &providerKey) const;
    QString firstProviderKey() const;
    QString defaultProviderKey() const;

private:
    QVector<ports::ProviderAdapter *> m_adapters;
};

}  // namespace hydra::domain
