#pragma once

#include <optional>

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include "domain/models/session_record.hpp"
#include "domain/models/session_timeline_event.hpp"

namespace hydra::domain::ports {

class SessionStore {
public:
    virtual ~SessionStore() = default;

    virtual QVector<domain::SessionRecord> listSessions() const = 0;
    virtual std::optional<domain::SessionRecord> sessionById(const QString &sessionId) const = 0;
    virtual bool upsertSession(const domain::SessionRecord &session, QString *errorMessage = nullptr) = 0;
    virtual bool deleteSessions(const QStringList &sessionIds,
                                QString *errorMessage = nullptr) = 0;
    virtual QHash<QString, QVector<domain::SessionTimelineEvent>> recentTimelineBySession(
        const QStringList &sessionIds,
        int limitPerSession) const = 0;
    virtual bool appendTimelineEvent(const domain::SessionTimelineEvent &event,
                                     int retainPerSession,
                                     QString *errorMessage = nullptr) = 0;
};

}  // namespace hydra::domain::ports
