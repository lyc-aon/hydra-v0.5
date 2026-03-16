#include "domain/services/session_supervisor.hpp"

#include <QDebug>

namespace hydra::domain {

bool SessionSupervisor::isPendingResumeToken(const QString &providerKey, const QString &resumeToken)
{
    const QString trimmedToken = resumeToken.trimmed();
    if (trimmedToken.isEmpty()) {
        return false;
    }

    if (providerKey == QStringLiteral("codex")) {
        return trimmedToken.startsWith(QStringLiteral("codex-pending-"));
    }
    if (providerKey == QStringLiteral("gemini")) {
        return trimmedToken.startsWith(QStringLiteral("gemini-pending-"));
    }
    if (providerKey == QStringLiteral("hermes")) {
        return trimmedToken.startsWith(QStringLiteral("hermes-pending-"));
    }
    if (providerKey == QStringLiteral("opencode")) {
        return trimmedToken.startsWith(QStringLiteral("opencode-pending-"));
    }

    return false;
}

const ports::ProviderSessionResolver *SessionSupervisor::resolverForProvider(
    const QString &providerKey) const
{
    for (const ports::ProviderSessionResolver *resolver : m_sessionResolvers) {
        if (resolver != nullptr && resolver->providerKey() == providerKey) {
            return resolver;
        }
    }
    return nullptr;
}

QSet<QString> SessionSupervisor::claimedProviderSessionIdsExcluding(const QString &sessionId) const
{
    QSet<QString> claimedProviderSessionIds;
    const QVector<SessionRecord> storedSessions = m_sessionStore.listSessions();
    for (const SessionRecord &storedSession : storedSessions) {
        if (storedSession.id == sessionId) {
            continue;
        }

        const QString storedToken = storedSession.providerSessionId.trimmed();
        if (storedToken.isEmpty()
            || isPendingResumeToken(storedSession.providerKey, storedToken))
        {
            continue;
        }

        claimedProviderSessionIds.insert(storedToken);
    }

    return claimedProviderSessionIds;
}

QString SessionSupervisor::resolveResumeToken(const SessionRecord &session,
                                              const QSet<QString> &excludedSessionIds)
{
    const QString storedToken = session.providerSessionId.trimmed();
    if (!storedToken.isEmpty() && !isPendingResumeToken(session.providerKey, storedToken)) {
        return storedToken;
    }

    const ports::ProviderSessionResolver *resolver = resolverForProvider(session.providerKey);
    if (resolver == nullptr) {
        return {};
    }

    QSet<QString> effectiveExcludedSessionIds = excludedSessionIds;
    effectiveExcludedSessionIds.unite(claimedProviderSessionIdsExcluding(session.id));

    if (const ports::ProviderAdapter *adapter = m_providerCatalog.adapterFor(session.providerKey)) {
        const QString expectedToken = adapter->expectedResolvedResumeToken(storedToken).trimmed();
        if (!expectedToken.isEmpty()
            && !effectiveExcludedSessionIds.contains(expectedToken)
            && resolver->sessionExists(expectedToken, session.workingDirectory))
        {
            return expectedToken;
        }
    }

    if (const auto pending = m_pendingResumeResolutions.constFind(session.id);
        pending != m_pendingResumeResolutions.cend())
    {
        const QString discovered = resolver->findNewSessionId(
            session.workingDirectory,
            pending->knownSessionIds,
            effectiveExcludedSessionIds);
        if (!discovered.isEmpty()) {
            return discovered;
        }

        return {};
    }

    const QSet<QString> currentKnownSessionIds =
        resolver->knownSessionIdsForWorkingDirectory(session.workingDirectory);
    QSet<QString> recoveryCandidates = currentKnownSessionIds;
    for (const QString &excludedId : effectiveExcludedSessionIds) {
        recoveryCandidates.remove(excludedId);
    }
    const bool allowSingleCandidateRecovery = session.state != SessionState::Exited
                                              || !session.ownerInstanceId.trimmed().isEmpty()
                                              || !session.tmuxSessionName.trimmed().isEmpty();
    if (allowSingleCandidateRecovery && recoveryCandidates.size() == 1) {
        return *recoveryCandidates.cbegin();
    }

    m_pendingResumeResolutions.insert(
        session.id,
        PendingResumeResolution{
            .providerKey = session.providerKey,
            .workingDirectory = session.workingDirectory,
            .knownSessionIds = currentKnownSessionIds,
        });
    return {};
}

bool SessionSupervisor::resolvePendingResumeToken(SessionRecord &session,
                                                  const QSet<QString> &excludedSessionIds)
{
    if (!session.providerSessionId.trimmed().isEmpty()
        && !isPendingResumeToken(session.providerKey, session.providerSessionId))
    {
        m_pendingResumeResolutions.remove(session.id);
        return false;
    }

    const QString resolvedToken = resolveResumeToken(session, excludedSessionIds);
    if (resolvedToken.isEmpty() || resolvedToken == session.providerSessionId) {
        return false;
    }

    session.providerSessionId = resolvedToken;
    QString errorMessage;
    if (!m_sessionStore.upsertSession(session, &errorMessage)) {
        qWarning().noquote()
            << QStringLiteral("Hydra failed to persist provider resume token for %1: %2")
                   .arg(session.id, errorMessage);
        return false;
    }

    m_pendingResumeResolutions.remove(session.id);
    return true;
}

}  // namespace hydra::domain
