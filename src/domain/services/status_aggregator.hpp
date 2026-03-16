#pragma once

#include <QString>
#include <QStringList>

#include "domain/models/session_record.hpp"
#include "domain/models/session_signal.hpp"
#include "domain/models/session_timeline_event.hpp"
#include "domain/ports/provider_adapter.hpp"

namespace hydra::domain {

struct SessionStatusUpdate {
    SessionState state = SessionState::Idle;
    QString detail;
    StatusProvenance provenance = StatusProvenance::Derived;
    QString sourceRef;
    SessionTimelineEvent timelineEvent;
};

class StatusAggregator final {
public:
    SessionStatusUpdate applySignal(const SessionRecord &session,
                                    const SessionSignal &signal,
                                    const ports::ProviderObservabilityHints &hints,
                                    const QString &paneTail,
                                    const QString &paneTitle) const;

private:
    static QString summaryForSignal(const SessionSignal &signal);
    static QString summaryForActiveState(SessionState state);
    static QString trimmedExcerpt(const QString &text);
    static QString normalizeLine(const QString &text);
    static QString lastNonEmptyLine(const QString &text);
    static QStringList filteredRecentLines(const QString &text,
                                           const QStringList &ignoredPatterns,
                                           int recentLineCount);
    static QString textAfterLatestPrompt(const QString &text,
                                         const ports::ProviderObservabilityHints &hints);
    static QString matchingRecentLine(const QString &text,
                                      const QStringList &patterns,
                                      const QStringList &ignoredPatterns = {},
                                      int recentLineCount = 14);
    static QString matchingValue(const QString &value, const QStringList &patterns);
    static QString preferredActiveDetail(const QString &text,
                                         const ports::ProviderObservabilityHints &hints,
                                         const QString &excludedDetail = {});
};

}  // namespace hydra::domain
