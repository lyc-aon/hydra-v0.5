#include <QCoreApplication>
#include <QTextStream>

#include "app/hydra_application.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    hydra::app::HydraApplication hydraApplication;

    QTextStream(stdout) << "Hydra core smoke initialized. repos="
                        << hydraApplication.appState()->repoCount() << " sessions="
                        << hydraApplication.appState()->sessionCount() << Qt::endl;
    return 0;
}
