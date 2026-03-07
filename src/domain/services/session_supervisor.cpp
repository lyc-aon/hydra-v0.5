#include "domain/services/session_supervisor.hpp"

#include <QDateTime>
#include <QUuid>

#include "domain/support/slug.hpp"

namespace hydra::domain {

SessionSupervisor::SessionSupervisor(ports::RepoStore &repoStore,
                                     ports::SessionStore &sessionStore,
                                     ports::MuxAdapter &muxAdapter)
    : m_repoStore(repoStore),
      m_sessionStore(sessionStore),
      m_muxAdapter(muxAdapter)
{
}

QVector<SessionRecord> SessionSupervisor::sessions() const
{
    return m_sessionStore.listSessions();
}

QVector<SessionRecord> SessionSupervisor::refreshSessionStates()
{
    QVector<SessionRecord> currentSessions = m_sessionStore.listSessions();
    for (SessionRecord &session : currentSessions) {
        const bool isLive = !session.tmuxSessionName.isEmpty() &&
                            m_muxAdapter.hasLiveSession(session.tmuxSessionName);

        const SessionState nextState =
            isLive ? (session.state == SessionState::Error ? SessionState::Error : SessionState::Idle)
                   : SessionState::Exited;

        if (nextState == session.state) {
            continue;
        }

        session.state = nextState;
        session.updatedAt = QDateTime::currentDateTimeUtc();
        QString errorMessage;
        m_sessionStore.upsertSession(session, &errorMessage);
    }

    return m_sessionStore.listSessions();
}

LaunchOutcome SessionSupervisor::launchGenericShell(const QString &repositoryId,
                                                    const QString &workingDirectory)
{
    const auto repository = m_repoStore.repositoryById(repositoryId);
    if (!repository.has_value()) {
        return {.ok = false,
                .message = QStringLiteral("Selected repository could not be found."),
                .session = std::nullopt};
    }

    const auto probe = m_muxAdapter.probeAvailability();
    if (!probe.available) {
        return {.ok = false,
                .message = QStringLiteral("tmux is required for the first Hydra slice. %1")
                               .arg(probe.message),
                .session = std::nullopt};
    }

    ports::MuxLaunchRequest launchRequest;
    launchRequest.sessionName = buildSessionName(repository.value());
    launchRequest.workingDirectory =
        workingDirectory.trimmed().isEmpty() ? repository->path : workingDirectory.trimmed();

    const auto launchResult = m_muxAdapter.launchDetachedSession(launchRequest);
    if (!launchResult.ok) {
        return {.ok = false, .message = launchResult.errorMessage, .session = std::nullopt};
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();

    SessionRecord session;
    session.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.repoId = repository->id;
    session.name = QStringLiteral("%1 shell").arg(repository->name);
    session.providerKey = QStringLiteral("generic-shell");
    session.state = SessionState::Idle;
    session.tmuxSessionName = launchResult.sessionName;
    session.tmuxPaneId = launchResult.paneId;
    session.workingDirectory = launchRequest.workingDirectory;
    session.createdAt = now;
    session.updatedAt = now;

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        QString cleanupSuffix;
        if (!launchResult.sessionName.isEmpty()) {
            const auto cleanup = m_muxAdapter.terminateSession(launchResult.sessionName);
            if (!cleanup.ok) {
                cleanupSuffix =
                    QStringLiteral(" tmux cleanup also failed: %1").arg(cleanup.errorMessage);
            }
        }

        return {.ok = false,
                .message = QStringLiteral("tmux session launched but persistence failed: %1")
                               .arg(errorMessage) +
                               cleanupSuffix,
                .session = std::nullopt};
    }

    return {.ok = true,
            .message = QStringLiteral("Launched %1 in tmux session %2.")
                           .arg(repository->name, launchResult.sessionName),
            .session = session};
}

SessionTerminationOutcome SessionSupervisor::terminateSession(const QString &sessionId)
{
    const auto sessionRecord = m_sessionStore.sessionById(sessionId);
    if (!sessionRecord.has_value()) {
        return {.ok = false,
                .message = QStringLiteral("Session could not be found."),
                .session = std::nullopt};
    }

    SessionRecord session = sessionRecord.value();
    const bool isLive = !session.tmuxSessionName.isEmpty() &&
                        m_muxAdapter.hasLiveSession(session.tmuxSessionName);

    if (isLive) {
        const auto terminate = m_muxAdapter.terminateSession(session.tmuxSessionName);
        if (!terminate.ok) {
            return {.ok = false,
                    .message = QStringLiteral("Failed to end tmux session %1: %2")
                                   .arg(session.tmuxSessionName, terminate.errorMessage),
                    .session = std::nullopt};
        }
    }

    session.state = SessionState::Exited;
    session.lastError.clear();
    session.updatedAt = QDateTime::currentDateTimeUtc();

    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        return {.ok = false,
                .message = QStringLiteral("tmux session ended but persistence failed: %1")
                               .arg(errorMessage),
                .session = std::nullopt};
    }

    return {.ok = true,
            .message = isLive
                           ? QStringLiteral("Ended tmux session %1.").arg(session.tmuxSessionName)
                           : QStringLiteral("Session was already inactive; metadata marked exited."),
            .session = session};
}

QString SessionSupervisor::buildSessionName(const Repository &repository)
{
    QString slug = support::slugifyLowerDash(repository.name);
    if (slug.isEmpty()) {
        slug = QStringLiteral("repo");
    }

    const QString suffix = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    return QStringLiteral("hydra-%1-%2").arg(slug, suffix);
}

}  // namespace hydra::domain
