#pragma once

#include <QString>
#include <QStringList>

class QGuiApplication;
class QQuickWindow;

namespace hydra::app {

struct DesktopLaunchOptions {
    QString screenshotPath;
    int screenshotDelayMs = 900;
    int windowWidth = -1;
    int windowHeight = -1;
    bool quitAfterScreenshot = false;
    bool startSidebarCollapsed = false;
    int startSidebarWidth = -1;
    QString startQuickHelpTopic;
    QString startDetailHelpTopic;
};

[[nodiscard]] DesktopLaunchOptions parseDesktopLaunchOptions(const QStringList &arguments);
void configureDesktopWindow(QQuickWindow *window, const DesktopLaunchOptions &options);
void scheduleDesktopScreenshotCapture(QQuickWindow *window,
                                     const DesktopLaunchOptions &options,
                                     QGuiApplication &application);

}  // namespace hydra::app
