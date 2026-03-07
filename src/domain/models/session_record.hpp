#pragma once

#include <QDateTime>
#include <QString>

#include "domain/models/session_state.hpp"

namespace hydra::domain {

struct SessionRecord {
    QString id;
    QString repoId;
    QString name;
    QString providerKey;
    SessionState state = SessionState::Starting;
    QString tmuxSessionName;
    QString tmuxPaneId;
    QString workingDirectory;
    QDateTime createdAt;
    QDateTime updatedAt;
    QString lastError;
};

}  // namespace hydra::domain
