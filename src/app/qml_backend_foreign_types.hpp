#pragma once

#include <qqmlintegration.h>

#include "app/desktop_dialog_bridge.hpp"
#include "ui/viewmodels/app_state.hpp"
#include "ui/viewmodels/app_state_facets.hpp"
#include "ui/viewmodels/provider_list_model.hpp"
#include "ui/viewmodels/repo_list_model.hpp"
#include "ui/viewmodels/resume_list_model.hpp"
#include "ui/viewmodels/session_list_model.hpp"
#include "ui/viewmodels/shell_state.hpp"
#include "ui/viewmodels/terminal_surface_controller.hpp"
#include "ui/viewmodels/theme_state.hpp"
#include "ui/viewmodels/worktree_list_model.hpp"

namespace hydra::app::qml {

struct RepoListModelForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::RepoListModel)
    QML_NAMED_ELEMENT(RepoListModel)
    QML_UNCREATABLE("RepoListModel is created by Hydra.")
};

struct ProviderListModelForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::ProviderListModel)
    QML_NAMED_ELEMENT(ProviderListModel)
    QML_UNCREATABLE("ProviderListModel is created by Hydra.")
};

struct ResumeListModelForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::ResumeListModel)
    QML_NAMED_ELEMENT(ResumeListModel)
    QML_UNCREATABLE("ResumeListModel is created by Hydra.")
};

struct SessionListModelForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::SessionListModel)
    QML_NAMED_ELEMENT(SessionListModel)
    QML_UNCREATABLE("SessionListModel is created by Hydra.")
};

struct WorktreeListModelForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::WorktreeListModel)
    QML_NAMED_ELEMENT(WorktreeListModel)
    QML_UNCREATABLE("WorktreeListModel is created by Hydra.")
};

struct MasterStateForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::MasterState)
    QML_NAMED_ELEMENT(MasterState)
    QML_UNCREATABLE("MasterState is created by Hydra.")
};

struct RouterStateForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::RouterState)
    QML_NAMED_ELEMENT(RouterState)
    QML_UNCREATABLE("RouterState is created by Hydra.")
};

struct AppStateForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::AppState)
    QML_NAMED_ELEMENT(AppState)
    QML_UNCREATABLE("AppState is created by Hydra.")
};

struct ShellStateForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::ShellState)
    QML_NAMED_ELEMENT(ShellState)
    QML_UNCREATABLE("ShellState is created by Hydra.")
};

struct TerminalSurfaceControllerForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::TerminalSurfaceController)
    QML_NAMED_ELEMENT(TerminalSurfaceController)
    QML_UNCREATABLE("TerminalSurfaceController is created by Hydra.")
};

struct ThemeStateForeign {
    Q_GADGET
    QML_FOREIGN(hydra::ui::ThemeState)
    QML_NAMED_ELEMENT(ThemeState)
    QML_UNCREATABLE("ThemeState is created by Hydra.")
};

struct DesktopDialogBridgeForeign {
    Q_GADGET
    QML_FOREIGN(hydra::app::DesktopDialogBridge)
    QML_NAMED_ELEMENT(DesktopDialogBridge)
    QML_UNCREATABLE("DesktopDialogBridge is created by Hydra.")
};

}  // namespace hydra::app::qml
