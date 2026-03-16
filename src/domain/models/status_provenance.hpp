#pragma once

#include <QString>

namespace hydra::domain {

enum class StatusProvenance {
    Derived,
    Tmux,
    PromptMarker,
    ProviderHint,
    UiLocal,
    MonitorError,
};

QString statusProvenanceToStorage(StatusProvenance provenance);
StatusProvenance statusProvenanceFromStorage(const QString &value);
QString statusProvenanceDisplayName(StatusProvenance provenance);
QString statusProvenanceToneKey(StatusProvenance provenance);

}  // namespace hydra::domain
