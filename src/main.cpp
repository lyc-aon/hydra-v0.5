#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QIcon>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QSocketNotifier>
#include <QUrl>
#include <QVariant>
#include <qqml.h>

#include <exception>

#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "app/desktop_launch_options.hpp"
#include "app/desktop_dialog_bridge.hpp"
#include "app/hydra_application.hpp"
#include "app/desktop_sound_controller.hpp"
#include "app/qml_backend_registration.hpp"

namespace {

sigset_t blockedSignalMask()
{
    sigset_t signalMask;
    sigemptyset(&signalMask);
    sigaddset(&signalMask, SIGINT);
    sigaddset(&signalMask, SIGTERM);
    sigaddset(&signalMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &signalMask, nullptr);
    return signalMask;
}

class PosixSignalRelay final : public QObject {
public:
    explicit PosixSignalRelay(const sigset_t &signalMask, QObject *parent = nullptr)
        : QObject(parent)
    {
        m_signalMask = signalMask;
        m_signalFd = signalfd(-1, &m_signalMask, SFD_NONBLOCK | SFD_CLOEXEC);
        if (m_signalFd < 0) {
            return;
        }

        m_notifier = std::make_unique<QSocketNotifier>(m_signalFd, QSocketNotifier::Read, this);
        QObject::connect(m_notifier.get(), &QSocketNotifier::activated, this, [this]() {
            if (m_signalFd < 0) {
                return;
            }

            signalfd_siginfo signalInfo;
            const ssize_t readSize = ::read(m_signalFd, &signalInfo, sizeof(signalInfo));
            if (readSize != static_cast<ssize_t>(sizeof(signalInfo))) {
                return;
            }

            if (QCoreApplication::instance() != nullptr) {
                QCoreApplication::quit();
            }
        });
    }

    ~PosixSignalRelay() override
    {
        m_notifier.reset();
        if (m_signalFd >= 0) {
            ::close(m_signalFd);
        }
    }

private:
    sigset_t m_signalMask {};
    int m_signalFd = -1;
    std::unique_ptr<QSocketNotifier> m_notifier;
};

}  // namespace

int main(int argc, char *argv[])
{
    const sigset_t signalMask = blockedSignalMask();
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra V2"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("hydra.local"));

    const QString iconPath =
        QDir(QStringLiteral(HYDRA_BOOTSTRAP_REPO_PATH)).filePath(QStringLiteral("hydra.png"));
    app.setWindowIcon(QIcon(iconPath));

    const hydra::app::DesktopLaunchOptions launchOptions =
        hydra::app::parseDesktopLaunchOptions(app.arguments());

    try {
        hydra::app::HydraApplication hydraApplication;
        hydra::app::DesktopDialogBridge desktopDialogBridge;
        hydra::app::DesktopSoundController desktopSoundController;
        desktopDialogBridge.setSoundController(&desktopSoundController);
        PosixSignalRelay signalRelay(signalMask);
        QObject::connect(&app, &QGuiApplication::lastWindowClosed, &hydraApplication, [ &hydraApplication ]() {
            hydraApplication.shutdownOwnedSessions();
        });
        if (!launchOptions.startThemeId.isEmpty()) {
            hydraApplication.themeState()->setCurrentThemeId(launchOptions.startThemeId);
        }
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

        const QUrl appUrl =
            QUrl::fromLocalFile(QDir(repoRoot).filePath(QStringLiteral("qml/Hydra/App.qml")));
        engine.load(appUrl);
        if (engine.rootObjects().isEmpty()) {
            qWarning("Hydra startup failed: QML root object did not load.");
            return 1;
        }

        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
        hydra::app::configureDesktopWindow(window, launchOptions);
        hydra::app::scheduleDesktopScreenshotCapture(window, launchOptions, app);
        const int exitCode = app.exec();
        hydraApplication.shutdownOwnedSessions();
        return exitCode;
    } catch (const std::exception &exception) {
        qCritical("Hydra startup failed: %s", exception.what());
        return 1;
    }
}
