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
    bool startSidebarCollapsedProvided = false;
    int startSidebarWidth = -1;
    QString startQuickHelpTopic;
    QString startDetailHelpTopic;
    QString startThemeId;
    QString startSessionTraceName;
    bool skipBootScreen = false;
    int startScreen = -1;
};

[[nodiscard]] DesktopLaunchOptions parseDesktopLaunchOptions(const QStringList &arguments);
void configureDesktopWindow(QQuickWindow *window, const DesktopLaunchOptions &options);
void scheduleDesktopScreenshotCapture(QQuickWindow *window,
                                     const DesktopLaunchOptions &options,
                                     QGuiApplication &application);

}  // namespace hydra::app
