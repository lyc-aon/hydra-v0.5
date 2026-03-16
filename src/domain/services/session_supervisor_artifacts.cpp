#include "domain/services/session_supervisor.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSaveFile>

#include "domain/services/session_audit_exporter.hpp"

namespace hydra::domain {

namespace {

constexpr int kTimelineRetentionPerSession = 24;
constexpr int kTranscriptCaptureLineCount = 200000;

bool writeTextArtifact(const QString &path, const QString &content, QString *errorMessage)
{
    const QString resolvedPath = QFileInfo(path).absoluteFilePath();
    QFileInfo(resolvedPath).dir().mkpath(QStringLiteral("."));

    QSaveFile file(resolvedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    const QByteArray bytes = content.toUtf8();
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

}  // namespace

QString SessionSupervisor::transcriptCaptureTarget(const SessionRecord &session) const
{
    const QString paneId = session.tmuxPaneId.trimmed();
    if (!paneId.isEmpty()) {
        return paneId;
    }

    return session.tmuxSessionName.trimmed();
}

QString SessionSupervisor::captureSessionTranscript(const SessionRecord &session) const
{
    const QString target = transcriptCaptureTarget(session);
    if (target.isEmpty()) {
        return {};
    }

    return m_muxAdapter.capturePaneTail(target, kTranscriptCaptureLineCount);
}

SessionSupervisor::SessionArtifactExportOutcome SessionSupervisor::exportSessionArtifacts(
    const SessionRecord &session,
    const QString &capturedTranscript) const
{
    SessionArtifactExportOutcome outcome;

    SessionAuditExportPayload payload;
    payload.session = session;
    if (const auto repository = m_repoStore.repositoryById(session.repoId); repository.has_value()) {
        payload.repositoryId = repository->id;
        payload.repositoryName = repository->name;
        payload.repositoryPath = repository->path;
    }
    payload.timeline = recentTimelineBySession(QStringList{session.id}, kTimelineRetentionPerSession)
                           .value(session.id);

    const QString auditPath = defaultSessionAuditExportPath(payload);
    const QFileInfo auditInfo(auditPath);
    const QString transcriptPath =
        auditInfo.dir().filePath(QStringLiteral("%1.log").arg(auditInfo.completeBaseName()));

    if (!capturedTranscript.isNull()) {
        QString transcriptError;
        if (writeTextArtifact(transcriptPath, capturedTranscript, &transcriptError)) {
            outcome.transcriptPath = transcriptPath;
            payload.transcriptPath = transcriptPath;
        } else {
            outcome.warnings.push_back(
                QStringLiteral("transcript export failed for %1: %2")
                    .arg(session.name, transcriptError));
        }
    }

    const SessionAuditExportResult auditExport = exportSessionAudit(payload, auditPath);
    if (auditExport.ok) {
        outcome.auditPath = auditExport.path;
    } else {
        outcome.warnings.push_back(
            QStringLiteral("audit export failed for %1: %2")
                .arg(session.name, auditExport.errorMessage));
    }

    return outcome;
}

}  // namespace hydra::domain
