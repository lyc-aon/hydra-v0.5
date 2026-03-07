#include "app/desktop_launch_options.hpp"

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPointer>
#include <QQuickWindow>
#include <QTimer>

#include <memory>

namespace hydra::app {

DesktopLaunchOptions parseDesktopLaunchOptions(const QStringList &arguments)
{
    DesktopLaunchOptions options;

    for (int index = 1; index < arguments.size(); ++index) {
        const QString &argument = arguments.at(index);
        if (argument == QStringLiteral("--screenshot") && index + 1 < arguments.size()) {
            options.screenshotPath = arguments.at(++index);
            continue;
        }
        if (argument == QStringLiteral("--screenshot-delay-ms") && index + 1 < arguments.size()) {
            options.screenshotDelayMs = arguments.at(++index).toInt();
            continue;
        }
        if (argument == QStringLiteral("--window-width") && index + 1 < arguments.size()) {
            options.windowWidth = arguments.at(++index).toInt();
            continue;
        }
        if (argument == QStringLiteral("--window-height") && index + 1 < arguments.size()) {
            options.windowHeight = arguments.at(++index).toInt();
            continue;
        }
        if (argument == QStringLiteral("--quit-after-screenshot")) {
            options.quitAfterScreenshot = true;
            continue;
        }
        if (argument == QStringLiteral("--start-sidebar-collapsed")) {
            options.startSidebarCollapsed = true;
            continue;
        }
        if (argument == QStringLiteral("--start-sidebar-width") && index + 1 < arguments.size()) {
            options.startSidebarWidth = arguments.at(++index).toInt();
            continue;
        }
        if (argument == QStringLiteral("--open-quick-help-topic") && index + 1 < arguments.size()) {
            options.startQuickHelpTopic = arguments.at(++index);
            continue;
        }
        if (argument == QStringLiteral("--open-help-topic") && index + 1 < arguments.size()) {
            options.startDetailHelpTopic = arguments.at(++index);
        }
    }

    return options;
}

void configureDesktopWindow(QQuickWindow *window, const DesktopLaunchOptions &options)
{
    if (window == nullptr) {
        return;
    }

    const int width = options.windowWidth > 0 ? options.windowWidth : window->width();
    const int height = options.windowHeight > 0 ? options.windowHeight : window->height();
    window->resize(width, height);
}

void scheduleDesktopScreenshotCapture(QQuickWindow *window,
                                     const DesktopLaunchOptions &options,
                                     QGuiApplication &application)
{
    if (window == nullptr || options.screenshotPath.isEmpty()) {
        return;
    }

    auto saveScreenshot = [window = QPointer<QQuickWindow>(window),
                           options,
                           application = QPointer<QGuiApplication>(&application),
                           completed = std::make_shared<bool>(false)]() {
        if (*completed || window == nullptr || application == nullptr) {
            return;
        }
        *completed = true;

        const QImage screenshot = window->grabWindow();
        if (screenshot.isNull()) {
            qCritical("Hydra screenshot capture failed: rendered image is null.");
            if (options.quitAfterScreenshot) {
                application->exit(2);
            }
            return;
        }

        const QFileInfo outputInfo(options.screenshotPath);
        outputInfo.dir().mkpath(QStringLiteral("."));
        if (!screenshot.save(options.screenshotPath)) {
            qCritical("Hydra screenshot capture failed: could not save %s",
                      qPrintable(options.screenshotPath));
            if (options.quitAfterScreenshot) {
                application->exit(2);
            }
            return;
        }

        qInfo("Hydra screenshot saved to %s", qPrintable(options.screenshotPath));
        if (options.quitAfterScreenshot) {
            application->exit(0);
        }
    };

    QObject::connect(window,
                     &QQuickWindow::frameSwapped,
                     &application,
                     [saveScreenshot]() {
                         QTimer::singleShot(0, saveScreenshot);
                     },
                     Qt::SingleShotConnection);

    QTimer::singleShot(options.screenshotDelayMs, &application, saveScreenshot);
}

}  // namespace hydra::app
