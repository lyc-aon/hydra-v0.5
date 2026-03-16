#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>

#include "domain/ports/session_store.hpp"

namespace hydra::infrastructure {

class SqliteSessionStore final : public domain::ports::SessionStore {
public:
    explicit SqliteSessionStore(QSqlDatabase database);

    QVector<domain::SessionRecord> listSessions() const override;
    std::optional<domain::SessionRecord> sessionById(const QString &sessionId) const override;
    bool upsertSession(const domain::SessionRecord &session, QString *errorMessage = nullptr) override;
    bool deleteSessions(const QStringList &sessionIds, QString *errorMessage = nullptr) override;
    QHash<QString, QVector<domain::SessionTimelineEvent>> recentTimelineBySession(
        const QStringList &sessionIds,
        int limitPerSession) const override;
    bool appendTimelineEvent(const domain::SessionTimelineEvent &event,
                             int retainPerSession,
                             QString *errorMessage = nullptr) override;

private:
    static domain::SessionRecord readSessionRow(const QSqlQuery &query);
    static domain::SessionTimelineEvent readTimelineRow(const QSqlQuery &query);

    QSqlDatabase m_database;
};

}  // namespace hydra::infrastructure
