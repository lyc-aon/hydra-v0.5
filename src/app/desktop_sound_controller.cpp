#include "app/desktop_sound_controller.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

namespace hydra::app {

namespace {

QString firstExistingDirectory(const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isDir()) {
            return info.absoluteFilePath();
        }
    }
    return {};
}

}  // namespace

DesktopSoundController::DesktopSoundController(QObject *parent)
    : QObject(parent)
{
    m_uptime.start();
    m_traceEnabled = traceEnabled();
    refreshConfiguration();
    traceMessage(QStringLiteral("Initialized. enabled=%1 player='%2' soundRoot='%3'")
                     .arg(m_enabled ? QStringLiteral("true") : QStringLiteral("false"),
                          m_playerCommand,
                          m_soundRoot));
}

bool DesktopSoundController::enabled() const
{
    return m_enabled;
}

void DesktopSoundController::playBoot()
{
    playCue(QStringLiteral("boot"), QStringLiteral("ui_boot.wav"), 800);
}

void DesktopSoundController::playClick()
{
    playCue(QStringLiteral("click"), QStringLiteral("ui_select_popup.wav"), 70);
}

void DesktopSoundController::playHover()
{
    playCue(QStringLiteral("hover"), QStringLiteral("ui_rollover.wav"), 120);
}

void DesktopSoundController::playApproval()
{
    playCue(QStringLiteral("approval"), QStringLiteral("ui_incoming_im.wav"), 220);
}

void DesktopSoundController::playCompletion()
{
    playCue(QStringLiteral("completion"), QStringLiteral("ui_incoming_mail.wav"), 220);
}

void DesktopSoundController::playWarning()
{
    playCue(QStringLiteral("warning"), QStringLiteral("ui_dialog_warning.wav"), 250);
}

void DesktopSoundController::playSplash()
{
    playCue(QStringLiteral("splash"), QStringLiteral("ui_nous_splash.wav"), 800);
}

void DesktopSoundController::playTerminalKey()
{
    playCue(QStringLiteral("terminal_key"), QStringLiteral("ui_keyboard_clicking.wav"), 25);
}

void DesktopSoundController::playCue(const QString &cueId,
                                     const QString &fileName,
                                     const qint64 minimumIntervalMs)
{
    refreshConfiguration();
    if (!m_enabled || shouldThrottle(cueId, minimumIntervalMs)) {
        if (!m_enabled) {
            traceMessage(QStringLiteral("Skipping cue '%1' because sound is disabled.").arg(cueId));
        }
        return;
    }

    const QString soundPath = QDir(m_soundRoot).filePath(fileName);
    if (!QFileInfo::exists(soundPath)) {
        qWarning().noquote()
            << QStringLiteral("Hydra sound asset missing for cue '%1': %2").arg(cueId, soundPath);
        return;
    }

    traceMessage(QStringLiteral("Playing cue '%1' with %2 -> %3")
                     .arg(cueId, m_playerCommand, soundPath));
    if (!QProcess::startDetached(m_playerCommand,
                                 playerArguments(soundPath),
                                 QString(),
                                 nullptr))
    {
        qWarning().noquote()
            << QStringLiteral("Hydra sound playback failed for cue '%1' using %2")
                   .arg(cueId, m_playerCommand);
    }
}

void DesktopSoundController::refreshConfiguration()
{
    const QString disableSound =
        qEnvironmentVariable("HYDRA_DISABLE_SOUND").trimmed().toLower();
    if (disableSound == QStringLiteral("1") || disableSound == QStringLiteral("true")) {
        m_enabled = false;
        traceMessage(QStringLiteral("Sound disabled by HYDRA_DISABLE_SOUND."));
        return;
    }

    if (m_playerCommand.isEmpty()) {
        m_playerCommand = resolvePlayerCommand();
    }
    if (m_soundRoot.isEmpty()) {
        m_soundRoot = resolveSoundRoot();
    }

    m_enabled = !m_playerCommand.isEmpty() && !m_soundRoot.isEmpty();
    if (!m_enabled) {
        traceMessage(QStringLiteral("Sound unavailable. player='%1' soundRoot='%2'")
                         .arg(m_playerCommand, m_soundRoot));
    }
}

bool DesktopSoundController::shouldThrottle(const QString &cueId, const qint64 minimumIntervalMs)
{
    if (minimumIntervalMs <= 0) {
        return false;
    }

    const qint64 nowMs = m_uptime.elapsed();
    const auto lastPlayback = m_lastPlaybackMs.constFind(cueId);
    if (lastPlayback != m_lastPlaybackMs.cend()) {
        if (nowMs - lastPlayback.value() < minimumIntervalMs) {
            return true;
        }
    }

    m_lastPlaybackMs.insert(cueId, nowMs);
    return false;
}

QString DesktopSoundController::resolvePlayerCommand() const
{
    const QString overrideCommand =
        qEnvironmentVariable("HYDRA_SOUND_PLAYER").trimmed();
    if (!overrideCommand.isEmpty()) {
        const QString resolvedOverride = QStandardPaths::findExecutable(overrideCommand);
        if (!resolvedOverride.isEmpty()) {
            return resolvedOverride;
        }
        if (QFileInfo::exists(overrideCommand)) {
            return QFileInfo(overrideCommand).absoluteFilePath();
        }
    }

    const QStringList candidates{
        QStringLiteral("pw-play"),
        QStringLiteral("aplay"),
    };
    for (const QString &candidate : candidates) {
        const QString resolved = QStandardPaths::findExecutable(candidate);
        if (!resolved.isEmpty()) {
            return resolved;
        }
    }
    return {};
}

QString DesktopSoundController::resolveSoundRoot() const
{
    const QString repoRoot = QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH);
    return firstExistingDirectory({
        QDir(repoRoot).filePath(QStringLiteral("qml/Hydra/assets/sounds")),
        QDir(QCoreApplication::applicationDirPath())
            .absoluteFilePath(QStringLiteral("Hydra/qml/Hydra/assets/sounds")),
    });
}

QStringList DesktopSoundController::playerArguments(const QString &soundPath) const
{
    if (QFileInfo(m_playerCommand).fileName() == QStringLiteral("aplay")) {
        return {QStringLiteral("-q"), soundPath};
    }
    return {soundPath};
}

bool DesktopSoundController::traceEnabled() const
{
    const QString traceValue =
        qEnvironmentVariable("HYDRA_SOUND_TRACE").trimmed().toLower();
    return traceValue == QStringLiteral("1") || traceValue == QStringLiteral("true");
}

void DesktopSoundController::traceMessage(const QString &message) const
{
    if (!m_traceEnabled) {
        return;
    }
    qWarning().noquote() << QStringLiteral("[hydra-sound] %1").arg(message);
}

}  // namespace hydra::app
