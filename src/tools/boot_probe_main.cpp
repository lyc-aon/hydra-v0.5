#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QVariant>

#include "app/desktop_dialog_bridge.hpp"
#include "app/desktop_launch_options.hpp"
#include "app/qml_backend_registration.hpp"
#include "app/desktop_sound_controller.hpp"
#include "app/hydra_application.hpp"

namespace {

int waitMsFromArguments(const QStringList &arguments)
{
    for (int index = 1; index < arguments.size(); ++index) {
        if (arguments.at(index) == QStringLiteral("--wait-ms") && index + 1 < arguments.size()) {
            return std::max(0, arguments.at(index + 1).toInt());
        }
    }

    return 9000;
}

QJsonValue jsonValueForVariant(const QVariant &value)
{
    return QJsonValue::fromVariant(value);
}

}  // namespace

int main(int argc, char *argv[])
{
    qputenv("HYDRA_DISABLE_SOUND", "1");
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra V2"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("hydra.local"));

    const QString iconPath =
        QDir(QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH)).filePath(QStringLiteral("hydra.png"));
    app.setWindowIcon(QIcon(iconPath));

    const hydra::app::DesktopLaunchOptions launchOptions =
        hydra::app::parseDesktopLaunchOptions(app.arguments());
    const int waitMs = waitMsFromArguments(app.arguments());

    hydra::app::HydraApplication hydraApplication;
    hydra::app::DesktopDialogBridge desktopDialogBridge;
    hydra::app::DesktopSoundController desktopSoundController;
    desktopDialogBridge.setSoundController(&desktopSoundController);
    hydra::app::registerQmlBackendTypes();

    QQmlApplicationEngine engine;
    const QString repoRoot = QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH);
    const QString runtimeImportPath =
        QDir(repoRoot).filePath(QStringLiteral("third_party/qmltermwidget_runtime"));
    engine.addImportPath(runtimeImportPath);
    engine.addImportPath(QDir(QCoreApplication::applicationDirPath())
                             .absoluteFilePath(QStringLiteral("../../third_party/qmltermwidget_runtime")));
    engine.setInitialProperties({
        {QStringLiteral("appState"), QVariant::fromValue(hydraApplication.appState())},
        {QStringLiteral("masterState"), QVariant::fromValue(hydraApplication.appState()->master())},
        {QStringLiteral("routerState"), QVariant::fromValue(hydraApplication.appState()->router())},
        {QStringLiteral("shellState"), QVariant::fromValue(hydraApplication.shellState())},
        {QStringLiteral("terminalController"),
         QVariant::fromValue(hydraApplication.terminalController())},
        {QStringLiteral("masterTerminalController"),
         QVariant::fromValue(hydraApplication.masterTerminalController())},
        {QStringLiteral("routerTerminalController"),
         QVariant::fromValue(hydraApplication.routerTerminalController())},
        {QStringLiteral("themeState"), QVariant::fromValue(hydraApplication.themeState())},
        {QStringLiteral("desktopBridge"), QVariant::fromValue(&desktopDialogBridge)},
        {QStringLiteral("startupSidebarCollapsed"), launchOptions.startSidebarCollapsed},
        {QStringLiteral("startupSidebarCollapsedSet"),
         launchOptions.startSidebarCollapsedProvided},
        {QStringLiteral("startupSidebarWidth"), launchOptions.startSidebarWidth},
        {QStringLiteral("startupQuickHelpTopic"), launchOptions.startQuickHelpTopic},
        {QStringLiteral("startupDetailHelpTopic"), launchOptions.startDetailHelpTopic},
        {QStringLiteral("startupSessionTraceName"), launchOptions.startSessionTraceName},
        {QStringLiteral("startupSkipBoot"), launchOptions.skipBootScreen},
    });

    const QUrl appUrl = QUrl::fromLocalFile(QDir(repoRoot).filePath(QStringLiteral("qml/Hydra/App.qml")));
    engine.load(appUrl);
    if (engine.rootObjects().isEmpty()) {
        qCritical("Hydra boot probe failed: QML root object did not load.");
        return 1;
    }

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    hydra::app::configureDesktopWindow(window, launchOptions);

    QTimer::singleShot(waitMs, &app, [&]() {
        QObject *root = engine.rootObjects().constFirst();
        const auto *appState = hydraApplication.appState();

        QJsonObject result;
        result.insert(QStringLiteral("wait_ms"), waitMs);
        result.insert(QStringLiteral("startupSkipBoot"), jsonValueForVariant(root->property("startupSkipBoot")));
        result.insert(QStringLiteral("startupShellReady"), jsonValueForVariant(root->property("startupShellReady")));
        result.insert(QStringLiteral("startupOverlayActive"), jsonValueForVariant(root->property("startupOverlayActive")));
        result.insert(QStringLiteral("nousSplashActive"), jsonValueForVariant(root->property("nousSplashActive")));
        result.insert(QStringLiteral("bootScreenActive"), jsonValueForVariant(root->property("bootScreenActive")));
        result.insert(QStringLiteral("activeViewMode"), jsonValueForVariant(root->property("activeViewMode")));
        result.insert(QStringLiteral("windowVisible"), window != nullptr ? QJsonValue(window->isVisible())
                                                                        : QJsonValue(false));
        result.insert(QStringLiteral("windowWidth"), window != nullptr ? QJsonValue(window->width())
                                                                      : QJsonValue(0));
        result.insert(QStringLiteral("windowHeight"), window != nullptr ? QJsonValue(window->height())
                                                                       : QJsonValue(0));
        result.insert(QStringLiteral("refreshing"), appState->refreshing());
        result.insert(QStringLiteral("repoCount"), appState->repoCount());
        result.insert(QStringLiteral("sessionCount"), appState->sessionCount());
        result.insert(QStringLiteral("selectedSessionName"), appState->selectedSessionName());
        result.insert(QStringLiteral("selectedSessionAvailable"), appState->selectedSessionAvailable());

        const QByteArray json = QJsonDocument(result).toJson(QJsonDocument::Compact);
        fprintf(stdout, "%s\n", json.constData());
        fflush(stdout);
        app.exit(0);
    });
    return app.exec();
}
