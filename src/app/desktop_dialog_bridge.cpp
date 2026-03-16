#include "app/desktop_dialog_bridge.hpp"

#include "app/desktop_sound_controller.hpp"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

namespace hydra::app {

DesktopDialogBridge::DesktopDialogBridge(QObject *parent)
    : QObject(parent)
{
}

void DesktopDialogBridge::setSoundController(DesktopSoundController *soundController)
{
    m_soundController = soundController;
}

QString DesktopDialogBridge::browseForFolder(const QString &initialPath) const
{
    const QFileInfo initialInfo(initialPath.trimmed());
    const QString startDirectory = initialInfo.exists()
                                       ? (initialInfo.isDir() ? initialInfo.absoluteFilePath()
                                                              : initialInfo.absolutePath())
                                       : QDir::homePath();

    return QFileDialog::getExistingDirectory(nullptr,
                                             QStringLiteral("Select Repository Folder"),
                                             startDirectory,
                                             QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
}

void DesktopDialogBridge::playBootSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playBoot();
    }
}

void DesktopDialogBridge::playClickSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playClick();
    }
}

void DesktopDialogBridge::playHoverSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playHover();
    }
}

void DesktopDialogBridge::playApprovalSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playApproval();
    }
}

void DesktopDialogBridge::playCompletionSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playCompletion();
    }
}

void DesktopDialogBridge::playWarningSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playWarning();
    }
}

void DesktopDialogBridge::playSplashSound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playSplash();
    }
}

void DesktopDialogBridge::playTerminalKeySound() const
{
    if (m_soundController != nullptr) {
        m_soundController->playTerminalKey();
    }
}

}  // namespace hydra::app
