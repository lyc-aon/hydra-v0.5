#include "app/qml_backend_registration.hpp"

#include <qqml.h>

#include "app/desktop_dialog_bridge.hpp"
#include "ui/viewmodels/app_state.hpp"
#include "ui/viewmodels/app_state_facets.hpp"
#include "ui/viewmodels/shell_state.hpp"
#include "ui/viewmodels/terminal_surface_controller.hpp"
#include "ui/viewmodels/theme_state.hpp"

namespace hydra::app {

void registerQmlBackendTypes()
{
    static bool registered = false;

    if (!registered) {
        qmlRegisterUncreatableType<hydra::ui::AppState>(
            "Hydra.Backend", 1, 0, "AppState", "AppState is created by Hydra.");
        qmlRegisterUncreatableType<hydra::ui::MasterState>(
            "Hydra.Backend", 1, 0, "MasterState", "MasterState is created by Hydra.");
        qmlRegisterUncreatableType<hydra::ui::RouterState>(
            "Hydra.Backend", 1, 0, "RouterState", "RouterState is created by Hydra.");
        qmlRegisterUncreatableType<hydra::ui::ShellState>(
            "Hydra.Backend", 1, 0, "ShellState", "ShellState is created by Hydra.");
        qmlRegisterUncreatableType<hydra::ui::TerminalSurfaceController>(
            "Hydra.Backend",
            1,
            0,
            "TerminalSurfaceController",
            "TerminalSurfaceController is created by Hydra.");
        qmlRegisterUncreatableType<hydra::ui::ThemeState>(
            "Hydra.Backend", 1, 0, "ThemeState", "ThemeState is created by Hydra.");
        qmlRegisterUncreatableType<hydra::app::DesktopDialogBridge>(
            "Hydra.Backend",
            1,
            0,
            "DesktopDialogBridge",
            "DesktopDialogBridge is created by Hydra.");
        registered = true;
    }
}

}  // namespace hydra::app
