#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/session_record.hpp"

namespace hydra::domain::ports {

class SessionStore {
public:
    virtual ~SessionStore() = default;

    virtual QVector<domain::SessionRecord> listSessions() const = 0;
    virtual std::optional<domain::SessionRecord> sessionById(const QString &sessionId) const = 0;
    virtual bool upsertSession(const domain::SessionRecord &session, QString *errorMessage = nullptr) = 0;
};

}  // namespace hydra::domain::ports
