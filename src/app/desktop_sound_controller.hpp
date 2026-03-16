#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

namespace hydra::app {

class DesktopSoundController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled CONSTANT)

public:
    explicit DesktopSoundController(QObject *parent = nullptr);

    [[nodiscard]] bool enabled() const;

    Q_INVOKABLE void playBoot();
    Q_INVOKABLE void playClick();
    Q_INVOKABLE void playHover();
    Q_INVOKABLE void playApproval();
    Q_INVOKABLE void playCompletion();
    Q_INVOKABLE void playWarning();
    Q_INVOKABLE void playSplash();
    Q_INVOKABLE void playTerminalKey();

private:
    void playCue(const QString &cueId, const QString &fileName, qint64 minimumIntervalMs);
    void refreshConfiguration();
    [[nodiscard]] bool shouldThrottle(const QString &cueId, qint64 minimumIntervalMs);
    [[nodiscard]] QString resolvePlayerCommand() const;
    [[nodiscard]] QString resolveSoundRoot() const;
    [[nodiscard]] QStringList playerArguments(const QString &soundPath) const;
    [[nodiscard]] bool traceEnabled() const;
    void traceMessage(const QString &message) const;

    QString m_playerCommand;
    QString m_soundRoot;
    bool m_enabled = false;
    bool m_traceEnabled = false;
    QElapsedTimer m_uptime;
    QHash<QString, qint64> m_lastPlaybackMs;
};

}  // namespace hydra::app
