#pragma once

#include <QObject>
#include <QString>

namespace hydra::app {

class DesktopSoundController;

class DesktopDialogBridge : public QObject {
    Q_OBJECT

public:
    explicit DesktopDialogBridge(QObject *parent = nullptr);

    void setSoundController(DesktopSoundController *soundController);

    Q_INVOKABLE QString browseForFolder(const QString &initialPath = QString()) const;
    Q_INVOKABLE void playBootSound() const;
    Q_INVOKABLE void playClickSound() const;
    Q_INVOKABLE void playHoverSound() const;
    Q_INVOKABLE void playApprovalSound() const;
    Q_INVOKABLE void playCompletionSound() const;
    Q_INVOKABLE void playWarningSound() const;
    Q_INVOKABLE void playSplashSound() const;
    Q_INVOKABLE void playTerminalKeySound() const;

private:
    DesktopSoundController *m_soundController = nullptr;
};

}  // namespace hydra::app
