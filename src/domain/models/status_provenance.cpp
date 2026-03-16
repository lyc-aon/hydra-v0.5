#include "domain/models/status_provenance.hpp"

namespace hydra::domain {

QString statusProvenanceToStorage(const StatusProvenance provenance)
{
    switch (provenance) {
    case StatusProvenance::Derived:
        return QStringLiteral("derived");
    case StatusProvenance::Tmux:
        return QStringLiteral("tmux");
    case StatusProvenance::PromptMarker:
        return QStringLiteral("prompt-marker");
    case StatusProvenance::ProviderHint:
        return QStringLiteral("provider-hint");
    case StatusProvenance::UiLocal:
        return QStringLiteral("ui-local");
    case StatusProvenance::MonitorError:
        return QStringLiteral("monitor-error");
    }

    return QStringLiteral("derived");
}

StatusProvenance statusProvenanceFromStorage(const QString &value)
{
    if (value == QStringLiteral("tmux")) {
        return StatusProvenance::Tmux;
    }
    if (value == QStringLiteral("prompt-marker")) {
        return StatusProvenance::PromptMarker;
    }
    if (value == QStringLiteral("provider-hint")) {
        return StatusProvenance::ProviderHint;
    }
    if (value == QStringLiteral("ui-local")) {
        return StatusProvenance::UiLocal;
    }
    if (value == QStringLiteral("monitor-error")) {
        return StatusProvenance::MonitorError;
    }

    return StatusProvenance::Derived;
}

QString statusProvenanceDisplayName(const StatusProvenance provenance)
{
    switch (provenance) {
    case StatusProvenance::Derived:
        return QStringLiteral("Derived");
    case StatusProvenance::Tmux:
        return QStringLiteral("tmux");
    case StatusProvenance::PromptMarker:
        return QStringLiteral("Prompt");
    case StatusProvenance::ProviderHint:
        return QStringLiteral("Provider");
    case StatusProvenance::UiLocal:
        return QStringLiteral("Hydra");
    case StatusProvenance::MonitorError:
        return QStringLiteral("Monitor");
    }

    return QStringLiteral("Derived");
}

QString statusProvenanceToneKey(const StatusProvenance provenance)
{
    switch (provenance) {
    case StatusProvenance::Derived:
        return QStringLiteral("steel");
    case StatusProvenance::Tmux:
        return QStringLiteral("ready");
    case StatusProvenance::PromptMarker:
        return QStringLiteral("bronze");
    case StatusProvenance::ProviderHint:
        return QStringLiteral("phosphor");
    case StatusProvenance::UiLocal:
        return QStringLiteral("steel");
    case StatusProvenance::MonitorError:
        return QStringLiteral("danger");
    }

    return QStringLiteral("steel");
}

}  // namespace hydra::domain
