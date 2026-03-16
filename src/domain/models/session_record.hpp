#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>

#include "domain/models/session_category.hpp"
#include "domain/models/session_state.hpp"
#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

struct SessionRecord {
    QString id;
    QString repoId;
    QString name;
    QString providerKey;
    QString providerSessionId;
    QString ownerInstanceId;
    QString launchSafetyKey;
    SessionState state = SessionState::Starting;
    QString tmuxSessionName;
    QString tmuxPaneId;
    QString workingDirectory;
    QString modelOverride;
    QHash<QString, QString> launchEnvironment;
    QString providerProfileMode;
    QString providerProfilePath;
    QDateTime createdAt;
    QDateTime updatedAt;
    QString lastError;
    QString statusDetail;
    StatusProvenance statusProvenance = StatusProvenance::Derived;
    QString statusSourceRef;
    QString transcriptPath;
    QString auditPath;
    QString alias;
    SessionCategory category = SessionCategory::Worker;
};

inline bool operator==(const SessionRecord &lhs, const SessionRecord &rhs)
{
    return lhs.id == rhs.id && lhs.repoId == rhs.repoId && lhs.name == rhs.name
           && lhs.providerKey == rhs.providerKey
           && lhs.providerSessionId == rhs.providerSessionId
           && lhs.ownerInstanceId == rhs.ownerInstanceId
           && lhs.launchSafetyKey == rhs.launchSafetyKey && lhs.state == rhs.state
           && lhs.tmuxSessionName == rhs.tmuxSessionName
           && lhs.tmuxPaneId == rhs.tmuxPaneId
           && lhs.workingDirectory == rhs.workingDirectory
           && lhs.modelOverride == rhs.modelOverride
           && lhs.launchEnvironment == rhs.launchEnvironment
           && lhs.providerProfileMode == rhs.providerProfileMode
           && lhs.providerProfilePath == rhs.providerProfilePath
           && lhs.createdAt == rhs.createdAt && lhs.updatedAt == rhs.updatedAt
           && lhs.lastError == rhs.lastError && lhs.statusDetail == rhs.statusDetail
           && lhs.statusProvenance == rhs.statusProvenance
           && lhs.statusSourceRef == rhs.statusSourceRef
           && lhs.transcriptPath == rhs.transcriptPath
           && lhs.auditPath == rhs.auditPath && lhs.alias == rhs.alias
           && lhs.category == rhs.category;
}

}  // namespace hydra::domain
