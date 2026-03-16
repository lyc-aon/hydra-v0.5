#include "domain/services/status_aggregator.hpp"

#include <QRegularExpression>
#include <QUuid>

namespace hydra::domain {

namespace {

QString detailOrFallback(const QString &detail, const QString &fallback)
{
    return detail.trimmed().isEmpty() ? fallback : detail.trimmed();
}

QString stripLeadingPromptMarker(QString value)
{
    value = value.trimmed();
    static const QRegularExpression expression(
        QStringLiteral(R"(^[\*\>\x{203A}\x{276F}\x{25B8}\x{2022}\-\+]+\s+)"));
    value.remove(expression);
    return value.trimmed();
}

bool matchesAnyPattern(const QString &value, const QStringList &patterns)
{
    for (const QString &pattern : patterns) {
        const QRegularExpression expression(pattern,
                                            QRegularExpression::CaseInsensitiveOption);
        if (!expression.isValid()) {
            continue;
        }
        if (expression.match(value).hasMatch()) {
            return true;
        }
    }

    return false;
}

QString retainedRefreshDetail(const SessionRecord &session,
                              const ports::ProviderObservabilityHints &hints)
{
    const QString detail = session.statusDetail.trimmed();
    if (detail.isEmpty()) {
        return QString();
    }

    if (session.state == SessionState::Idle
        && (session.statusProvenance == StatusProvenance::PromptMarker
            || matchesAnyPattern(detail, hints.promptPatterns)
            || matchesAnyPattern(detail, hints.promptTitlePatterns)))
    {
        return detail;
    }

    if (session.state == SessionState::AwaitingApproval
        && (session.statusProvenance == StatusProvenance::ProviderHint
            || matchesAnyPattern(detail, hints.approvalPatterns)
            || matchesAnyPattern(detail, hints.approvalTitlePatterns)))
    {
        return detail;
    }

    if (session.state == SessionState::WaitingForInput
        && (session.statusProvenance == StatusProvenance::ProviderHint
            || matchesAnyPattern(detail, hints.waitingInputPatterns)
            || matchesAnyPattern(detail, hints.waitingTitlePatterns)))
    {
        return detail;
    }

    return QString();
}

}  // namespace

SessionStatusUpdate StatusAggregator::applySignal(const SessionRecord &session,
                                                  const SessionSignal &signal,
                                                  const ports::ProviderObservabilityHints &hints,
                                                  const QString &paneTail,
                                                  const QString &paneTitle) const
{
    SessionStatusUpdate update;
    update.state = session.state;
    update.provenance = signal.provenance;
    update.sourceRef = signal.sourceRef.trimmed().isEmpty() ? session.tmuxSessionName : signal.sourceRef.trimmed();
    const QDateTime occurredAt =
        signal.occurredAt.isValid() ? signal.occurredAt : QDateTime::currentDateTimeUtc();

    const QString approvalTitle = matchingValue(paneTitle, hints.approvalTitlePatterns);
    const QString promptTitle = matchingValue(paneTitle, hints.promptTitlePatterns);
    const QString activeTitle = matchingValue(paneTitle, hints.activeTitlePatterns);
    const QString waitingTitle = matchingValue(paneTitle, hints.waitingTitlePatterns);
    const QString approvalLine =
        matchingRecentLine(paneTail, hints.approvalPatterns, hints.ignoredTailPatterns, 12);
    const QString activeLine =
        matchingRecentLine(paneTail, hints.activePatterns, hints.ignoredTailPatterns, 12);
    const QString activeDetail = preferredActiveDetail(
        paneTail,
        hints,
        session.statusProvenance == StatusProvenance::UiLocal && session.state == hints.activeState
            ? session.statusDetail
            : QString());
    const QString waitingLine =
        matchingRecentLine(paneTail, hints.waitingInputPatterns, hints.ignoredTailPatterns, 4);
    const QString promptLine =
        matchingRecentLine(paneTail, hints.promptPatterns, hints.ignoredTailPatterns, 3);
    const QString retainedActiveDetail =
        session.state == hints.activeState && !session.statusDetail.trimmed().isEmpty()
            && !matchesAnyPattern(session.statusDetail, hints.ignoredTailPatterns)
            && !matchesAnyPattern(session.statusDetail, hints.approvalPatterns)
            && !matchesAnyPattern(session.statusDetail, hints.promptPatterns)
            && !matchesAnyPattern(session.statusDetail, hints.waitingInputPatterns)
            ? session.statusDetail.trimmed()
            : QString();

    QString summary = summaryForSignal(signal);
    QString detail = signal.payload.trimmed();
    QString timelineDetail = signal.payload.trimmed();
    const qint64 recentInputStateHoldMs =
        hints.recentInputStateHoldMs > 0 ? hints.recentInputStateHoldMs : 2500;
    const qint64 inputStateDeltaMs =
        session.updatedAt.isValid() ? session.updatedAt.msecsTo(occurredAt)
                                    : recentInputStateHoldMs + 1;
    const bool activeHoldEligible =
        session.statusProvenance == StatusProvenance::UiLocal
        || (hints.preferRecentInputHoldBeforeTailSignals
            && session.statusProvenance == StatusProvenance::ProviderHint);
    const bool holdRecentInputState =
        session.state == hints.activeState
        && activeHoldEligible
        && session.updatedAt.isValid()
        && qAbs(inputStateDeltaMs) <= recentInputStateHoldMs;

    switch (signal.kind) {
    case SessionSignalKind::BootstrapExited:
    case SessionSignalKind::SessionExited:
        update.state = SessionState::Exited;
        update.provenance = StatusProvenance::Tmux;
        update.sourceRef = session.tmuxSessionName;
        detail = QStringLiteral("tmux no longer reports the detached session as live.");
        timelineDetail = detail;
        break;
    case SessionSignalKind::MonitorError:
        update.state = SessionState::Error;
        update.provenance = StatusProvenance::MonitorError;
        detail = detailOrFallback(detail, QStringLiteral("tmux control monitor reported an error."));
        timelineDetail = detail;
        break;
    case SessionSignalKind::ApprovalRequested:
        update.state = SessionState::AwaitingApproval;
        update.provenance = StatusProvenance::ProviderHint;
        detail = detailOrFallback(approvalLine, QStringLiteral("Provider output indicates approval is required."));
        summary = QStringLiteral("Approval requested");
        timelineDetail = detail;
        break;
    case SessionSignalKind::WaitingForInputDetected:
        update.state = SessionState::WaitingForInput;
        update.provenance = StatusProvenance::ProviderHint;
        detail = detailOrFallback(waitingLine, QStringLiteral("Provider output indicates input is required."));
        summary = QStringLiteral("Waiting for input");
        timelineDetail = detail;
        break;
    case SessionSignalKind::InputSubmitted:
        update.state = hints.activeState;
        update.provenance = StatusProvenance::UiLocal;
        detail = detailOrFallback(detail, QStringLiteral("Hydra forwarded input to the live provider session."));
        summary = summaryForActiveState(update.state);
        timelineDetail = detail;
        break;
    case SessionSignalKind::PromptDetected:
        update.state = SessionState::Idle;
        update.provenance = StatusProvenance::PromptMarker;
        detail = detailOrFallback(promptLine,
                                  QStringLiteral("Prompt marker detected in pane output."));
        summary = QStringLiteral("Prompt ready");
        timelineDetail = detail;
        break;
    case SessionSignalKind::PaneOutputObserved:
        update.state = hints.activeState;
        update.provenance = StatusProvenance::Tmux;
        detail = QStringLiteral("tmux reported new pane output.");
        summary = summaryForActiveState(update.state);
        timelineDetail = detailOrFallback(activeDetail, detailOrFallback(trimmedExcerpt(paneTail), detail));
        break;
    case SessionSignalKind::BootstrapLive:
    case SessionSignalKind::MonitorAttached:
    case SessionSignalKind::ManualRefresh:
        update.state = SessionState::Idle;
        update.provenance = signal.provenance;
        detail = detailOrFallback(retainedRefreshDetail(session, hints),
                                  QStringLiteral("Hydra refreshed the live session state."));
        timelineDetail = QStringLiteral("Hydra refreshed the live session state.");
        break;
    case SessionSignalKind::LaunchRequested:
        update.state = SessionState::Starting;
        update.provenance = StatusProvenance::UiLocal;
        detail = detailOrFallback(detail, QStringLiteral("Hydra launched the detached tmux session."));
        timelineDetail = detail;
        break;
    case SessionSignalKind::TerminateRequested:
        update.state = SessionState::Exited;
        update.provenance = StatusProvenance::UiLocal;
        detail = detailOrFallback(detail, QStringLiteral("Hydra ended the detached tmux session."));
        timelineDetail = detail;
        break;
    }

    if (!approvalTitle.isEmpty()) {
        update.state = SessionState::AwaitingApproval;
        update.provenance = StatusProvenance::ProviderHint;
        detail = approvalTitle;
        summary = QStringLiteral("Approval requested");
        timelineDetail = detail;
    } else if (!promptTitle.isEmpty()) {
        update.state = SessionState::Idle;
        update.provenance = StatusProvenance::PromptMarker;
        detail = promptTitle;
        summary = QStringLiteral("Prompt ready");
        timelineDetail = detail;
    } else if (!activeTitle.isEmpty()) {
        update.state = hints.activeState;
        update.provenance = StatusProvenance::ProviderHint;
        if (hints.preferActiveTitleDuringRecentInputHold && holdRecentInputState) {
            detail = detailOrFallback(activeTitle, retainedActiveDetail);
        } else {
            detail = detailOrFallback(activeDetail, detailOrFallback(activeTitle, retainedActiveDetail));
        }
        summary = summaryForActiveState(update.state);
        timelineDetail = detail;
    } else if (hints.preferRecentInputHoldBeforeTailSignals && holdRecentInputState) {
        update.state = session.state;
        update.provenance = session.statusProvenance;
        detail = session.statusDetail;
        summary = summaryForActiveState(update.state);
        timelineDetail = detail;
    } else if (!activeLine.isEmpty()) {
        update.state = hints.activeState;
        update.provenance = StatusProvenance::ProviderHint;
        detail = activeLine;
        summary = summaryForActiveState(update.state);
        timelineDetail = detail;
    } else if (holdRecentInputState) {
        update.state = session.state;
        update.provenance = session.statusProvenance;
        detail = session.statusDetail;
        summary = summaryForActiveState(update.state);
        timelineDetail = detail;
    } else if (!approvalLine.isEmpty()) {
        update.state = SessionState::AwaitingApproval;
        update.provenance = StatusProvenance::ProviderHint;
        detail = approvalLine;
        summary = QStringLiteral("Approval requested");
        timelineDetail = detail;
    } else if (!promptLine.isEmpty() && update.state != SessionState::Exited
               && update.state != SessionState::Error)
    {
        update.state = SessionState::Idle;
        update.provenance = StatusProvenance::PromptMarker;
        detail = promptLine;
        summary = QStringLiteral("Prompt ready");
        timelineDetail = detail;
    } else if (!waitingTitle.isEmpty()) {
        update.state = SessionState::WaitingForInput;
        update.provenance = StatusProvenance::ProviderHint;
        detail = waitingTitle;
        summary = QStringLiteral("Waiting for input");
        timelineDetail = detail;
    } else if (!waitingLine.isEmpty()) {
        update.state = SessionState::WaitingForInput;
        update.provenance = StatusProvenance::ProviderHint;
        detail = waitingLine;
        summary = QStringLiteral("Waiting for input");
        timelineDetail = detail;
    } else if (signal.kind == SessionSignalKind::PaneOutputObserved
               && update.state != SessionState::Exited
               && update.state != SessionState::Error)
    {
        const bool retainStableState =
            session.state == SessionState::Idle
            || session.state == SessionState::AwaitingApproval
            || session.state == SessionState::WaitingForInput
            || session.state == SessionState::Starting;
        if (retainStableState && activeDetail.isEmpty()) {
            update.state = session.state;
            update.provenance = session.statusProvenance;
            detail = session.statusDetail;
            switch (session.state) {
            case SessionState::Idle:
                summary = QStringLiteral("Prompt ready");
                break;
            case SessionState::AwaitingApproval:
                summary = QStringLiteral("Approval requested");
                break;
            case SessionState::WaitingForInput:
                summary = QStringLiteral("Waiting for input");
                break;
            case SessionState::Starting:
                summary = QStringLiteral("Launch");
                break;
            default:
                summary = summaryForSignal(signal);
                break;
            }
            timelineDetail = detail;
        } else {
            update.state = hints.activeState;
            update.provenance = StatusProvenance::Tmux;
            detail = QStringLiteral("tmux reported new pane output.");
            summary = summaryForActiveState(update.state);
            timelineDetail =
                detailOrFallback(activeDetail, detailOrFallback(trimmedExcerpt(paneTail), detail));
        }
    }

    update.timelineEvent.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    update.timelineEvent.sessionId = session.id;
    update.timelineEvent.signalKind = signal.kind;
    update.timelineEvent.state = update.state;
    update.timelineEvent.summary = summary;
    update.timelineEvent.detail = timelineDetail;
    update.timelineEvent.provenance = update.provenance;
    update.timelineEvent.sourceRef = update.sourceRef;
    update.timelineEvent.createdAt =
        signal.occurredAt.isValid() ? signal.occurredAt : QDateTime::currentDateTimeUtc();
    update.detail = detail;

    return update;
}

QString StatusAggregator::summaryForSignal(const SessionSignal &signal)
{
    switch (signal.kind) {
    case SessionSignalKind::BootstrapLive:
        return QStringLiteral("Live session recovered");
    case SessionSignalKind::BootstrapExited:
        return QStringLiteral("Session exited");
    case SessionSignalKind::MonitorAttached:
        return QStringLiteral("Monitor attached");
    case SessionSignalKind::PaneOutputObserved:
        return QStringLiteral("Activity observed");
    case SessionSignalKind::PromptDetected:
        return QStringLiteral("Prompt ready");
    case SessionSignalKind::ApprovalRequested:
        return QStringLiteral("Approval requested");
    case SessionSignalKind::WaitingForInputDetected:
        return QStringLiteral("Waiting for input");
    case SessionSignalKind::InputSubmitted:
        return QStringLiteral("Input submitted");
    case SessionSignalKind::LaunchRequested:
        return QStringLiteral("Launch");
    case SessionSignalKind::TerminateRequested:
        return QStringLiteral("Terminate");
    case SessionSignalKind::SessionExited:
        return QStringLiteral("Session exited");
    case SessionSignalKind::MonitorError:
        return QStringLiteral("Monitor error");
    case SessionSignalKind::ManualRefresh:
        return QStringLiteral("Manual refresh");
    }

    return QStringLiteral("Session update");
}

QString StatusAggregator::summaryForActiveState(const SessionState state)
{
    if (state == SessionState::Thinking) {
        return QStringLiteral("Thinking");
    }

    return QStringLiteral("Activity observed");
}

QString StatusAggregator::trimmedExcerpt(const QString &text)
{
    const QString line = lastNonEmptyLine(text);
    if (line.isEmpty()) {
        return QString();
    }

    return line.left(160);
}

QString StatusAggregator::normalizeLine(const QString &text)
{
    QString value = text;
    value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    value = value.simplified();
    return value.trimmed();
}

QString StatusAggregator::lastNonEmptyLine(const QString &text)
{
    const QStringList lines = filteredRecentLines(text, {}, 1);
    if (!lines.isEmpty()) {
        return lines.first();
    }

    return QString();
}

QStringList StatusAggregator::filteredRecentLines(const QString &text,
                                                  const QStringList &ignoredPatterns,
                                                  const int recentLineCount)
{
    if (text.trimmed().isEmpty() || recentLineCount <= 0) {
        return {};
    }

    const QStringList lines = text.split(QLatin1Char('\n'));
    QStringList recentLines;
    recentLines.reserve(recentLineCount);
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        const QString line = normalizeLine(*it);
        if (line.isEmpty()) {
            continue;
        }
        if (!ignoredPatterns.isEmpty() && matchesAnyPattern(line, ignoredPatterns)) {
            continue;
        }

        recentLines.push_back(line);
        if (recentLines.size() >= recentLineCount) {
            break;
        }
    }

    return recentLines;
}

QString StatusAggregator::textAfterLatestPrompt(const QString &text,
                                                const ports::ProviderObservabilityHints &hints)
{
    if (text.trimmed().isEmpty()) {
        return QString();
    }

    const QStringList lines = text.split(QLatin1Char('\n'));
    int latestPromptIndex = -1;
    for (int index = 0; index < lines.size(); ++index) {
        const QString line = normalizeLine(lines.at(index));
        if (line.isEmpty()) {
            continue;
        }
        if (matchesAnyPattern(line, hints.ignoredTailPatterns)) {
            continue;
        }
        if (matchesAnyPattern(line, hints.promptPatterns)) {
            latestPromptIndex = index;
        }
    }

    if (latestPromptIndex < 0 || latestPromptIndex + 1 >= lines.size()) {
        return text;
    }

    return lines.mid(latestPromptIndex + 1).join(QLatin1Char('\n'));
}

QString StatusAggregator::matchingRecentLine(const QString &text,
                                             const QStringList &patterns,
                                             const QStringList &ignoredPatterns,
                                             const int recentLineCount)
{
    if (text.trimmed().isEmpty() || patterns.isEmpty()) {
        return QString();
    }

    const QStringList recentLines = filteredRecentLines(text, ignoredPatterns, recentLineCount);

    for (const QString &line : recentLines) {
        if (matchesAnyPattern(line, patterns)) {
            return line.left(160);
        }
    }

    return QString();
}

QString StatusAggregator::matchingValue(const QString &value, const QStringList &patterns)
{
    const QString normalizedValue = normalizeLine(value);
    if (normalizedValue.isEmpty() || patterns.isEmpty()) {
        return QString();
    }

    for (const QString &pattern : patterns) {
        const QRegularExpression expression(pattern,
                                            QRegularExpression::CaseInsensitiveOption);
        if (!expression.isValid()) {
            continue;
        }
        if (expression.match(normalizedValue).hasMatch()) {
            return normalizedValue.left(160);
        }
    }

    return QString();
}

QString StatusAggregator::preferredActiveDetail(const QString &text,
                                                const ports::ProviderObservabilityHints &hints,
                                                const QString &excludedDetail)
{
    const QString activeText =
        hints.restrictActiveTailToPromptSegment ? textAfterLatestPrompt(text, hints) : text;
    const QString matchedActiveLine =
        matchingRecentLine(activeText, hints.activePatterns, hints.ignoredTailPatterns, 12);
    if (!matchedActiveLine.isEmpty()) {
        return matchedActiveLine;
    }

    if (!hints.allowGenericActiveTailFallback) {
        return QString();
    }

    const QStringList recentLines = filteredRecentLines(activeText, hints.ignoredTailPatterns, 12);
    for (const QString &line : recentLines) {
        if (!excludedDetail.trimmed().isEmpty()) {
            const QString normalizedLine = normalizeLine(line);
            const QString normalizedExcluded = normalizeLine(excludedDetail);
            if (normalizedLine == normalizedExcluded
                || stripLeadingPromptMarker(normalizedLine)
                       == stripLeadingPromptMarker(normalizedExcluded))
            {
                continue;
            }
        }
        if (matchesAnyPattern(line, hints.approvalPatterns)
            || matchesAnyPattern(line, hints.waitingInputPatterns)
            || matchesAnyPattern(line, hints.promptPatterns))
        {
            continue;
        }

        return line.left(160);
    }

    return QString();
}

}  // namespace hydra::domain
