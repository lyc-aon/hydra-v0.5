#pragma once

#include <QString>
#include <QSqlDatabase>

namespace hydra::infrastructure {

class DatabaseManager final {
public:
    explicit DatabaseManager(const QString &databasePath);
    ~DatabaseManager();

    QSqlDatabase database() const;

private:
    void runMigrations();
    void execOrThrow(const QString &sql);
    bool tableHasColumn(const QString &tableName, const QString &columnName);
    void ensureColumnExists(const QString &tableName,
                            const QString &columnName,
                            const QString &columnDefinition);

    QString m_connectionName;
    QSqlDatabase m_database;
};

}  // namespace hydra::infrastructure
