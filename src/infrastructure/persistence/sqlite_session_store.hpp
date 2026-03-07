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

private:
    static domain::SessionRecord readSessionRow(const QSqlQuery &query);

    QSqlDatabase m_database;
};

}  // namespace hydra::infrastructure
