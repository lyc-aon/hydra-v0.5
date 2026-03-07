#pragma once

#include <optional>

#include <QString>
#include <QVector>

#include "domain/models/session_record.hpp"
#include "domain/ports/mux_adapter.hpp"
#include "domain/ports/repo_store.hpp"
#include "domain/ports/session_store.hpp"

namespace hydra::domain {

struct LaunchOutcome {
    bool ok = false;
    QString message;
    std::optional<SessionRecord> session;
};

struct SessionTerminationOutcome {
    bool ok = false;
    QString message;
    std::optional<SessionRecord> session;
};

class SessionSupervisor final {
public:
    SessionSupervisor(ports::RepoStore &repoStore,
                      ports::SessionStore &sessionStore,
                      ports::MuxAdapter &muxAdapter);

    QVector<SessionRecord> sessions() const;
    QVector<SessionRecord> refreshSessionStates();
    LaunchOutcome launchGenericShell(const QString &repositoryId,
                                     const QString &workingDirectory = QString());
    SessionTerminationOutcome terminateSession(const QString &sessionId);

private:
    static QString buildSessionName(const Repository &repository);

    ports::RepoStore &m_repoStore;
    ports::SessionStore &m_sessionStore;
    ports::MuxAdapter &m_muxAdapter;
};

}  // namespace hydra::domain
