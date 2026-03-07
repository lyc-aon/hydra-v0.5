#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QVariant>

#include <exception>

#include "app/desktop_launch_options.hpp"
#include "app/hydra_application.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra V2"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("hydra.local"));

    const hydra::app::DesktopLaunchOptions launchOptions =
        hydra::app::parseDesktopLaunchOptions(app.arguments());

    try {
        hydra::app::HydraApplication hydraApplication;

        QQmlApplicationEngine engine;
        engine.setInitialProperties({
            {QStringLiteral("appState"), QVariant::fromValue(hydraApplication.appState())},
            {QStringLiteral("startupSidebarCollapsed"), launchOptions.startSidebarCollapsed},
            {QStringLiteral("startupSidebarWidth"), launchOptions.startSidebarWidth},
            {QStringLiteral("startupQuickHelpTopic"), launchOptions.startQuickHelpTopic},
            {QStringLiteral("startupDetailHelpTopic"), launchOptions.startDetailHelpTopic},
        });
        engine.loadFromModule(QStringLiteral("Hydra"), QStringLiteral("App"));
        if (engine.rootObjects().isEmpty()) {
            return 1;
        }

        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
        hydra::app::configureDesktopWindow(window, launchOptions);
        hydra::app::scheduleDesktopScreenshotCapture(window, launchOptions, app);

        return app.exec();
    } catch (const std::exception &exception) {
        qCritical("Hydra startup failed: %s", exception.what());
        return 1;
    }
}
