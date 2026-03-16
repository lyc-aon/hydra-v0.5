#include <QGuiApplication>
#include <QTextStream>
#include <QTimer>

#include "app/hydra_application.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Hydra"));
    QCoreApplication::setApplicationName(QStringLiteral("Hydra V2"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("hydra.local"));
    hydra::app::HydraApplication hydraApplication;
    hydraApplication.startUiLifecycle();
    QTimer timeout;
    timeout.setSingleShot(true);

    bool refreshStarted = hydraApplication.appState()->refreshing();
    QObject::connect(&timeout, &QTimer::timeout, &app, &QCoreApplication::quit);
    QObject::connect(hydraApplication.appState(),
                     &hydra::ui::AppState::refreshingChanged,
                     &app,
                     [&]() {
                         if (hydraApplication.appState()->refreshing()) {
                             refreshStarted = true;
                             return;
                         }
                         if (refreshStarted) {
                             app.quit();
                         }
                     });

    timeout.start(3000);
    app.exec();

    QTextStream(stdout) << "Hydra core smoke initialized. repos="
                        << hydraApplication.appState()->repoCount() << " sessions="
                        << hydraApplication.appState()->sessionCount() << " selected="
                        << hydraApplication.appState()->selectedSessionName() << " tmux="
                        << hydraApplication.appState()->selectedSessionTmuxSessionName()
                        << " available="
                        << (hydraApplication.appState()->selectedSessionAvailable() ? "true"
                                                                                     : "false")
                        << Qt::endl;
    return 0;
}
