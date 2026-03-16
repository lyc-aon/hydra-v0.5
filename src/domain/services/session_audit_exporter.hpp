#pragma once

#include <QString>
#include <QVector>

#include "domain/models/session_record.hpp"
#include "domain/models/session_timeline_event.hpp"

namespace hydra::domain {

struct SessionAuditExportPayload {
    SessionRecord session;
    QString repositoryId;
    QString repositoryName;
    QString repositoryPath;
    QVector<SessionTimelineEvent> timeline;
    QString transcriptPath;
};

struct SessionAuditExportResult {
    bool ok = false;
    QString path;
    QString errorMessage;
};

QString defaultSessionAuditExportPath(const SessionAuditExportPayload &payload);
SessionAuditExportResult exportSessionAudit(const SessionAuditExportPayload &payload,
                                            const QString &outputPath = QString());

}  // namespace hydra::domain
