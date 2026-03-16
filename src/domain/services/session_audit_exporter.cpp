#include "domain/services/session_audit_exporter.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include "domain/models/launch_safety.hpp"
#include "domain/models/provider_identity.hpp"
#include "domain/models/session_signal.hpp"
#include "domain/models/session_state.hpp"
#include "domain/models/status_provenance.hpp"
#include "domain/support/slug.hpp"

namespace hydra::domain {

namespace {

QString isoText(const QDateTime &value)
{
    return value.isValid() ? value.toUTC().toString(Qt::ISODateWithMs) : QStringLiteral("");
}

QString cleanPath(const QString &value)
{
    return value.trimmed().isEmpty() ? QStringLiteral("") : QDir::cleanPath(value.trimmed());
}

QString targetKind(const SessionAuditExportPayload &payload)
{
    const QString workingDirectory = cleanPath(payload.session.workingDirectory);
    if (workingDirectory.isEmpty()) {
        return QStringLiteral("unknown");
    }
    if (workingDirectory == QDir::homePath()) {
        return QStringLiteral("home");
    }
    const QString repositoryPath = cleanPath(payload.repositoryPath);
    if (repositoryPath.isEmpty()) {
        return QStringLiteral("directory");
    }
    if (workingDirectory == repositoryPath) {
        return QStringLiteral("repository_root");
    }
    if (workingDirectory.startsWith(repositoryPath + QDir::separator())) {
        return QStringLiteral("repository_subdirectory");
    }
    return QStringLiteral("linked_worktree_or_override");
}

QJsonObject timelineEventObject(const SessionTimelineEvent &event)
{
    return {
        {QStringLiteral("at"), isoText(event.createdAt)},
        {QStringLiteral("signal_key"), sessionSignalKindToStorage(event.signalKind)},
        {QStringLiteral("signal_label"), sessionSignalKindDisplayName(event.signalKind)},
        {QStringLiteral("state_key"), sessionStateToStorage(event.state)},
        {QStringLiteral("state_label"), sessionStateDisplayName(event.state)},
        {QStringLiteral("summary"), event.summary},
        {QStringLiteral("detail"), event.detail},
        {QStringLiteral("provenance_key"), statusProvenanceToStorage(event.provenance)},
        {QStringLiteral("provenance_label"), statusProvenanceDisplayName(event.provenance)},
        {QStringLiteral("source_ref"), event.sourceRef},
    };
}

QJsonDocument buildAuditDocument(const SessionAuditExportPayload &payload)
{
    QJsonArray timeline;
    for (const SessionTimelineEvent &event : payload.timeline) {
        timeline.push_back(timelineEventObject(event));
    }

    const QJsonObject repository = payload.repositoryPath.trimmed().isEmpty()
                                       ? QJsonObject()
                                       : QJsonObject{
                                             {QStringLiteral("id"), payload.repositoryId},
                                             {QStringLiteral("name"), payload.repositoryName},
                                             {QStringLiteral("path"), payload.repositoryPath},
                                         };

    const SessionRecord &session = payload.session;
    const QString workingDirectory = cleanPath(session.workingDirectory);

    return QJsonDocument(QJsonObject{
        {QStringLiteral("schema"), QStringLiteral("hydra.session-audit.v1")},
        {QStringLiteral("exported_at"), isoText(QDateTime::currentDateTimeUtc())},
        {QStringLiteral("session"),
         QJsonObject{{QStringLiteral("id"), session.id},
                     {QStringLiteral("name"), session.name},
                     {QStringLiteral("state_key"), sessionStateToStorage(session.state)},
                     {QStringLiteral("state_label"), sessionStateDisplayName(session.state)},
                     {QStringLiteral("tmux_session_name"), session.tmuxSessionName},
                     {QStringLiteral("tmux_pane_id"), session.tmuxPaneId},
                     {QStringLiteral("created_at"), isoText(session.createdAt)},
                     {QStringLiteral("updated_at"), isoText(session.updatedAt)}}},
        {QStringLiteral("provider"),
         QJsonObject{{QStringLiteral("key"), session.providerKey},
                     {QStringLiteral("label"), providerDisplayNameForKey(session.providerKey)}}},
        {QStringLiteral("launch_safety"),
         QJsonObject{{QStringLiteral("key"), session.launchSafetyKey},
                     {QStringLiteral("label"),
                      launchSafetyDisplayName(launchSafetyFromKey(session.launchSafetyKey))},
                     {QStringLiteral("dangerous"),
                      session.launchSafetyKey == QStringLiteral("bypass")}}},
        {QStringLiteral("target"),
         QJsonObject{{QStringLiteral("kind"), targetKind(payload)},
                     {QStringLiteral("working_directory"), workingDirectory},
                     {QStringLiteral("repository"), repository}}},
        {QStringLiteral("status"),
         QJsonObject{{QStringLiteral("detail"), session.statusDetail},
                     {QStringLiteral("provenance_key"),
                      statusProvenanceToStorage(session.statusProvenance)},
                     {QStringLiteral("provenance_label"),
                      statusProvenanceDisplayName(session.statusProvenance)},
                     {QStringLiteral("source_ref"), session.statusSourceRef},
                     {QStringLiteral("last_error"), session.lastError}}},
        {QStringLiteral("artifacts"),
         QJsonObject{{QStringLiteral("transcript_path"), payload.transcriptPath}}},
        {QStringLiteral("timeline"), timeline},
    });
}

}  // namespace

QString defaultSessionAuditExportPath(const SessionAuditExportPayload &payload)
{
    QString exportRoot = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (exportRoot.trimmed().isEmpty()) {
        exportRoot = QDir::homePath();
    }

    QDir exportDir(QDir(exportRoot).filePath(QStringLiteral("exports")));
    exportDir.mkpath(QStringLiteral("."));

    const QString slug = support::slugifyLowerDash(payload.session.name);
    const QString timestamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString baseName = slug.isEmpty() ? QStringLiteral("session-audit") : slug;
    return exportDir.filePath(QStringLiteral("%1-%2.json").arg(baseName, timestamp));
}

SessionAuditExportResult exportSessionAudit(const SessionAuditExportPayload &payload,
                                            const QString &outputPath)
{
    const QString resolvedPath = outputPath.trimmed().isEmpty()
                                     ? defaultSessionAuditExportPath(payload)
                                     : QFileInfo(outputPath.trimmed()).absoluteFilePath();

    QFileInfo outputInfo(resolvedPath);
    outputInfo.dir().mkpath(QStringLiteral("."));

    QSaveFile file(resolvedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {.ok = false, .path = resolvedPath, .errorMessage = file.errorString()};
    }

    const QByteArray json = buildAuditDocument(payload).toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size()) {
        return {.ok = false, .path = resolvedPath, .errorMessage = file.errorString()};
    }
    if (!file.commit()) {
        return {.ok = false, .path = resolvedPath, .errorMessage = file.errorString()};
    }

    return {.ok = true, .path = resolvedPath, .errorMessage = QStringLiteral("")};
}

}  // namespace hydra::domain
