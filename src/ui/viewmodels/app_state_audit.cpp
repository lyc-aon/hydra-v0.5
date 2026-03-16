#include "ui/viewmodels/app_state.hpp"

#include "domain/services/session_audit_exporter.hpp"

namespace hydra::ui {

bool AppState::exportSelectedSessionAudit()
{
    const domain::SessionRecord *session = findSession(m_selectedSessionId);
    if (session == nullptr) {
        showWarningStatus(QStringLiteral("Select a live session before exporting its audit."));
        return false;
    }

    domain::SessionAuditExportPayload payload;
    payload.session = *session;
    if (const domain::Repository *repository = findRepository(session->repoId)) {
        payload.repositoryId = repository->id;
        payload.repositoryName = repository->name;
        payload.repositoryPath = repository->path;
    }
    payload.timeline = m_sessionSupervisor.recentTimelineBySession(QStringList{session->id}, 24)
                           .value(session->id);
    payload.transcriptPath = session->transcriptPath;

    const domain::SessionAuditExportResult outcome = domain::exportSessionAudit(payload);
    if (!outcome.ok) {
        showWarningStatus(QStringLiteral("Failed to export session audit: %1")
                              .arg(outcome.errorMessage));
        return false;
    }

    showInfoStatus(QStringLiteral("Exported session audit to %1")
                       .arg(outcome.path),
                   4200);
    recordActivity(QStringLiteral("audit"));
    return true;
}

}  // namespace hydra::ui
