#include "ui/viewmodels/app_state.hpp"

#include <QDir>
#include <QStandardPaths>

#include "domain/models/launch_safety.hpp"

namespace hydra::ui {

namespace {

QString bootstrapRepoPath()
{
    return QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH);
}

QString bootstrapRepoFilePath(const QString &relativePath)
{
    return QDir(bootstrapRepoPath()).filePath(relativePath);
}

}  // namespace

QString AppState::masterProviderKey() const { return m_masterProviderKey; }

void AppState::setMasterProviderKey(const QString &providerKey)
{
    if (m_masterProviderKey == providerKey) return;
    m_masterProviderKey = providerKey;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterLaunchSafetyKey() const
{
    return domain::launchSafetyKey(m_masterLaunchSafety);
}

void AppState::setMasterLaunchSafetyKey(const QString &safetyKey)
{
    const auto safety = domain::launchSafetyFromKey(safetyKey);
    if (m_masterLaunchSafety == safety) return;
    m_masterLaunchSafety = safety;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterModelId() const { return m_masterModelId; }

void AppState::setMasterModelId(const QString &modelId)
{
    const QString trimmed = modelId.trimmed();
    if (m_masterModelId == trimmed) return;
    m_masterModelId = trimmed;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterApiKeyEnvVarName() const
{
    return normalizedProviderApiKeyEnvVarName(m_masterProviderKey, m_masterApiKeyEnvVarName);
}

void AppState::setMasterApiKeyEnvVarName(const QString &envVarName)
{
    const QString trimmed = envVarName.trimmed();
    if (m_masterApiKeyEnvVarName == trimmed) return;
    m_masterApiKeyEnvVarName = trimmed;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterApiKeyValue() const { return m_masterApiKeyValue; }

void AppState::setMasterApiKeyValue(const QString &apiKeyValue)
{
    if (m_masterApiKeyValue == apiKeyValue) return;
    m_masterApiKeyValue = apiKeyValue;
    emit masterConfigChanged();
}

QString AppState::masterSysprompt() const { return m_masterSysprompt; }

void AppState::setMasterSysprompt(const QString &sysprompt)
{
    if (m_masterSysprompt == sysprompt) return;
    m_masterSysprompt = sysprompt;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterHermesProfileMode() const
{
    return normalizedHermesProfileMode(m_masterHermesProfileMode);
}

void AppState::setMasterHermesProfileMode(const QString &profileMode)
{
    const QString normalizedMode = normalizedHermesProfileMode(profileMode);
    if (m_masterHermesProfileMode == normalizedMode) return;
    m_masterHermesProfileMode = normalizedMode;
    persistSelectionState();
    emit masterConfigChanged();
}

QString AppState::masterHermesProfilePath() const
{
    return m_masterHermesProfilePath.trimmed().isEmpty()
               ? defaultHermesProfileTemplatePath()
               : m_masterHermesProfilePath;
}

void AppState::setMasterHermesProfilePath(const QString &profilePath)
{
    const QString trimmed = profilePath.trimmed();
    const QString normalizedPath =
        trimmed.isEmpty() ? defaultHermesProfileTemplatePath() : trimmed;
    if (m_masterHermesProfilePath == normalizedPath) return;
    m_masterHermesProfilePath = normalizedPath;
    persistSelectionState();
    emit masterConfigChanged();
}

void AppState::applyMasterConfig(const QString &providerKey,
                                 const QString &safetyKey,
                                 const QString &sysprompt,
                                 const QString &modelId,
                                 const QString &apiKeyEnvVarName,
                                 const QString &apiKeyValue,
                                 const QString &hermesProfileMode,
                                 const QString &hermesProfilePath)
{
    const bool masterWasRunning = findMasterSession() != nullptr;

    m_masterProviderKey = providerKey.trimmed();
    m_masterLaunchSafety = domain::launchSafetyFromKey(safetyKey);
    m_masterModelId = modelId.trimmed();
    m_masterApiKeyEnvVarName = apiKeyEnvVarName.trimmed();
    m_masterApiKeyValue = apiKeyValue;
    m_masterSysprompt = sysprompt;
    m_masterHermesProfileMode = normalizedHermesProfileMode(hermesProfileMode);
    m_masterHermesProfilePath = hermesProfilePath.trimmed().isEmpty()
                                    ? defaultHermesProfileTemplatePath()
                                    : hermesProfilePath.trimmed();
    persistSelectionState();
    emit masterConfigChanged();
    if (masterWasRunning) {
        relaunchMasterSession();
    } else {
        showInfoStatus(QStringLiteral("Saved master config. Launch the master terminal when needed."));
    }
}

void AppState::relaunchMasterSession()
{
    const QString masterSessionId = [this]() -> QString {
        const auto *master = findMasterSession();
        return master != nullptr ? master->id : QString();
    }();
    if (!masterSessionId.isEmpty()) {
        m_sessionSupervisor.terminateSession(masterSessionId);
    }
    m_masterSessionId.clear();
    refreshSessionsSnapshot();
    resolveMasterSession();
    emit masterSessionChanged();
    ensureMasterSession();
}

QString AppState::masterWorkspacePath()
{
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataDir).filePath(QStringLiteral("workspaces/master"));
}

QString AppState::defaultMasterSysprompt() const
{
    const QString readmePath = bootstrapRepoFilePath(QStringLiteral("README.md"));
    const QString handoffPath = bootstrapRepoFilePath(QStringLiteral("HANDOFF.md"));
    const QString architecturePath = bootstrapRepoFilePath(
        QStringLiteral("docs/architecture/system-architecture.md"));

    return QStringLiteral(
        "# Hydra Master Terminal\n"
        "\n"
        "You are the Master Terminal in Hydra, a multi-session AI orchestration "
        "environment. Multiple AI agent sessions run in parallel tmux panes. "
        "You coordinate through the Hydra Router.\n"
        "\n"
        "## Control Boundary\n"
        "\n"
        "Your only outbound control path is the Router session.\n"
        "Do not send text directly to worker panes.\n"
        "Do not use raw `tmux send-keys`, `tmux paste-buffer`, `tmux capture-pane`, or similar pane-targeting commands against worker sessions from this master terminal.\n"
        "If the Router is unhealthy or unavailable, report that problem instead of bypassing it.\n"
        "The master workspace is control-only. Do not create or edit product files here to satisfy user requests.\n"
        "If a user request would change app code, notes, or artifacts in a repo, that work belongs to a worker session.\n"
        "\n"
        "## Local Control Kit\n"
        "\n"
        "Everything you need is already inside this workspace:\n"
        "\n"
        "    ls\n"
        "    cat HYDRA_CONTROL.md\n"
        "    cat manifest.json\n"
        "\n"
        "Use the local `hydra-router` wrapper instead of absolute Hydra control paths whenever possible.\n"
        "\n"
        "## Sending Commands To The Router\n"
        "\n"
        "Default to sending the human goal to the Router in plain language. The Router is "
        "responsible for choosing a target session, scraping recent context, enriching the "
        "worker prompt, and routing it.\n"
        "\n"
        "Send the raw human goal to the Router like this:\n"
        "\n"
        "    ./hydra-router ask-router \"<plain-language request>\"\n"
        "\n"
        "`./hydra-router ask-router` already wraps the request in the Router contract. Do not "
        "turn a casual human request into Hydra jargon unless it is necessary.\n"
        "\n"
        "For normal user requests, your first real action should be `./hydra-router ask-router`.\n"
        "Do not inspect the repo, read `manifest.json`, browse files, or build a plan before that unless the "
        "request is specifically about Hydra state.\n"
        "Do not use Read/Edit/Write/Glob/Grep tools in the master workspace for normal worker tasks.\n"
        "If more context is needed, let the Router gather it from the target worker.\n"
        "If the human explicitly names a live alias such as Alpha, Bravo, Charlie, or Delta, "
        "preserve that alias in the request you send to the Router.\n"
        "Do not paraphrase away an explicit worker name like `ask Delta`, `see what Charlie thinks`, "
        "or `send this to Bravo`.\n"
        "Explicit alias mentions are binding intent signals, not optional flavor text.\n"
        "\n"
        "Do not use `./hydra-router route` as your normal control path from the master terminal.\n"
        "Use the explicit route helper only for control-plane debugging or an operator instruction that is explicitly about testing the router contract itself:\n"
        "\n"
        "    ./hydra-router route <alias> \"<worker prompt>\"\n"
        "\n"
        "Both helper paths preserve the Router control boundary. They return quickly, the Router stays available for more routing work, and worker completion comes back later as a routed report after Router review.\n"
        "\n"
        "You may receive lines beginning with `ROUTER REPORT @...:`.\n"
        "\n"
        "Short one-line forms are infrastructure status updates.\n"
        "\n"
        "Multi-line `ROUTER REPORT` blocks that include `router_summary:` and `worker_report:` "
        "are source material coming back from a worker through the Router. Treat that block as "
        "real content, not just metadata.\n"
        "\n"
        "When you receive a multi-line `ROUTER REPORT` block:\n"
        "- summarize the substance directly for the human\n"
        "- paraphrase the findings clearly instead of repeating the raw block verbatim\n"
        "- do not only say that a report arrived or is coming through\n"
        "- only mention incompleteness if the worker report is obviously truncated or ambiguous\n"
        "- if the router summary and worker report disagree, say that briefly and ask the Router for one clarifying follow-up\n"
        "\n"
        "## Human-Facing Behavior\n"
        "\n"
        "User prompts may be casual, vague, or underspecified. Do not demand protocol words "
        "or technical precision from the human.\n"
        "\n"
        "The human should not need to name an alias, file, implementation plan, or routing "
        "syntax for normal requests.\n"
        "\n"
        "For creative build requests, give at most one short sentence of commentary, then send "
        "the request almost verbatim through `./hydra-router ask-router`.\n"
        "For requests that explicitly name a worker alias, keep that alias in the text almost verbatim "
        "when you call `./hydra-router ask-router`.\n"
        "For note-writing, code changes, parser updates, and page changes, do not do the work yourself in this workspace.\n"
        "\n"
        "If the request is as vague as `make something cool`, `make it look better`, or "
        "`one-shot vibe code a page`, still move forward. Trust the Router to choose a worker, "
        "scrape recent context, infer a sensible artifact, and sharpen the worker prompt.\n"
        "\n"
        "Only ask the human a follow-up when the request is truly blocked by a missing decision.\n"
        "\n"
        "## Secondary References\n"
        "\n"
        "If you need deeper Hydra validation context, these local references are available:\n"
        "\n"
        "    cat %1\n"
        "    cat %2\n"
        "    cat %3\n"
        "\n"
        "## Workflow\n"
        "\n"
        "1. Keep the human request high-level and natural\n"
        "2. Use `./hydra-router ask-router` first for normal requests\n"
        "3. Read `manifest.json` only when session awareness is actually needed\n"
        "4. Do not bypass the Router to reach workers directly\n"
        "5. Reserve `./hydra-router route <alias>` for explicit router-control debugging only\n"
        "6. Wait for `ROUTER REPORT` and summarize important completions briefly\n"
        "7. For multi-line `ROUTER REPORT` blocks, summarize the actual findings instead of announcing completion\n"
        "\n"
        "You have full shell access for orchestration, file operations, and tmux "
        "interaction, but the Router remains the primary path to workers.\n"
    ).arg(readmePath,
          handoffPath,
          architecturePath);
}

}  // namespace hydra::ui
