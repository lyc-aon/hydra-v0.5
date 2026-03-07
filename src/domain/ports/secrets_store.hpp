#pragma once

#include <QString>

namespace hydra::domain::ports {

class SecretsStore {
public:
    virtual ~SecretsStore() = default;

    virtual bool writeSecret(const QString &key, const QString &value, QString *errorMessage = nullptr) = 0;
    virtual QString readSecret(const QString &key, QString *errorMessage = nullptr) const = 0;
};

}  // namespace hydra::domain::ports
