#include "domain/services/provider_catalog.hpp"

namespace hydra::domain {

ProviderCatalog::ProviderCatalog(const QVector<ports::ProviderAdapter *> &adapters)
    : m_adapters(adapters)
{
}

QVector<ports::ProviderProbe> ProviderCatalog::placeholderProbes() const
{
    QVector<ports::ProviderProbe> results;
    results.reserve(m_adapters.size());

    for (const ports::ProviderAdapter *adapter : m_adapters) {
        if (adapter == nullptr) {
            continue;
        }
        results.push_back({
            .providerKey = adapter->providerKey(),
            .displayName = adapter->displayName(),
            .available = true,
            .executablePath = QString(),
            .versionString = QString(),
            .statusMessage = QStringLiteral("Checking provider availability..."),
        });
    }

    return results;
}

QVector<ports::ProviderProbe> ProviderCatalog::probes() const
{
    QVector<ports::ProviderProbe> results;
    results.reserve(m_adapters.size());

    for (const ports::ProviderAdapter *adapter : m_adapters) {
        if (adapter == nullptr) {
            continue;
        }
        results.push_back(adapter->probe());
    }

    return results;
}

std::optional<ports::ProviderProbe> ProviderCatalog::probeFor(const QString &providerKey) const
{
    const ports::ProviderAdapter *adapter = adapterFor(providerKey);
    if (adapter == nullptr) {
        return std::nullopt;
    }

    return adapter->probe();
}

const ports::ProviderAdapter *ProviderCatalog::adapterFor(const QString &providerKey) const
{
    for (const ports::ProviderAdapter *adapter : m_adapters) {
        if (adapter != nullptr && adapter->providerKey() == providerKey) {
            return adapter;
        }
    }

    return nullptr;
}

QString ProviderCatalog::firstProviderKey() const
{
    for (const ports::ProviderAdapter *adapter : m_adapters) {
        if (adapter != nullptr) {
            return adapter->providerKey();
        }
    }

    return {};
}

QString ProviderCatalog::defaultProviderKey() const
{
    for (const ports::ProviderAdapter *adapter : m_adapters) {
        if (adapter != nullptr && adapter->probe().available) {
            return adapter->providerKey();
        }
    }

    return {};
}

}  // namespace hydra::domain
