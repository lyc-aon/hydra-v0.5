cmake_minimum_required(VERSION 3.24)

project(Hydra VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(HydraWarnings)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Sql Concurrent)

qt_standard_project_setup(REQUIRES 6.5)

set(HYDRA_CORE_SOURCES
    src/app/hydra_application.cpp
    src/domain/models/launch_safety.cpp
    src/domain/models/provider_identity.cpp
    src/domain/models/session_category.cpp
    src/domain/models/session_signal.cpp
    src/domain/models/session_state.cpp
    src/domain/models/status_provenance.cpp
    src/domain/services/provider_catalog.cpp
    src/domain/services/repo_registry.cpp
    src/domain/services/session_audit_exporter.cpp
    src/domain/services/session_supervisor.cpp
    src/domain/services/session_supervisor_artifacts.cpp
    src/domain/services/session_supervisor_lifecycle.cpp
    src/domain/services/session_supervisor_observation.cpp
    src/domain/services/session_supervisor_resume.cpp
    src/domain/services/status_aggregator.cpp
    src/domain/services/worktree_manager.cpp
    src/domain/support/slug.cpp
    src/infrastructure/git/git_repo_workspace.cpp
    src/infrastructure/mux/tmux_adapter.cpp
    src/infrastructure/process/process_runner.cpp
    src/infrastructure/providers/claude_code_adapter.cpp
    src/infrastructure/providers/codex_cli_adapter.cpp
    src/infrastructure/providers/codex_session_resolver.cpp
    src/infrastructure/providers/gemini_cli_adapter.cpp
    src/infrastructure/providers/gemini_session_resolver.cpp
    src/infrastructure/providers/hermes_cli_adapter.cpp
    src/infrastructure/providers/hermes_session_resolver.cpp
    src/infrastructure/providers/opencode_cli_adapter.cpp
    src/infrastructure/providers/opencode_session_resolver.cpp
    src/infrastructure/providers/provider_state_paths.cpp
    src/infrastructure/providers/provider_support.cpp
    src/infrastructure/persistence/database_manager.cpp
    src/infrastructure/persistence/sqlite_repo_store.cpp
    src/infrastructure/persistence/sqlite_session_store.cpp
    src/infrastructure/terminal/tmux_terminal_backend.cpp
    src/ui/viewmodels/app_state_actions.cpp
    src/ui/viewmodels/app_state_audit.cpp
    src/ui/viewmodels/app_state.cpp
    src/ui/viewmodels/app_state_facets.cpp
    src/ui/viewmodels/app_state_master.cpp
    src/ui/viewmodels/app_state_persistence.cpp
    src/ui/viewmodels/app_state_provider_runtime.cpp
    src/ui/viewmodels/app_state_router.cpp
    src/ui/viewmodels/app_state_router_presets.cpp
    src/ui/viewmodels/app_state_refresh.cpp
    src/ui/viewmodels/app_state_selection.cpp
    src/ui/viewmodels/app_state_worker_runtime.cpp
    src/ui/viewmodels/provider_list_model.cpp
    src/ui/viewmodels/repo_list_model.cpp
    src/ui/viewmodels/resume_list_model.cpp
    src/ui/viewmodels/router_preset_library.cpp
    src/ui/viewmodels/session_list_model.cpp
    src/ui/viewmodels/shell_state.cpp
    src/ui/viewmodels/terminal_surface_controller.cpp
    src/ui/viewmodels/theme_state.cpp
    src/ui/viewmodels/worktree_list_model.cpp
)

add_library(hydra_core STATIC ${HYDRA_CORE_SOURCES})
target_include_directories(hydra_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(hydra_core PRIVATE HYDRA_BOOTSTRAP_REPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(hydra_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_core)

add_executable(hydra_core_smoke
    src/tools/core_smoke_main.cpp
)
target_link_libraries(hydra_core_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_core_smoke)

add_executable(hydra_shutdown_resume_smoke
    src/tools/shutdown_resume_smoke_main.cpp
)
target_link_libraries(hydra_shutdown_resume_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_shutdown_resume_smoke)

add_executable(hydra_terminal_clipboard_routing_smoke
    src/tools/terminal_clipboard_routing_smoke_main.cpp
)
target_link_libraries(hydra_terminal_clipboard_routing_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_terminal_clipboard_routing_smoke)

add_executable(hydra_terminal_interactive_attach_smoke
    src/tools/terminal_interactive_attach_smoke_main.cpp
)
target_link_libraries(hydra_terminal_interactive_attach_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_terminal_interactive_attach_smoke)

add_executable(hydra_session_order_smoke
    src/tools/session_order_smoke_main.cpp
)
target_link_libraries(hydra_session_order_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_session_order_smoke)

add_executable(hydra_provider_launch_overrides_smoke
    src/tools/provider_launch_overrides_smoke_main.cpp
)
target_link_libraries(hydra_provider_launch_overrides_smoke
    PRIVATE
        hydra_core
        Qt6::Core
        Qt6::Gui
        Qt6::Concurrent
        Qt6::Sql
)
hydra_apply_warnings(hydra_provider_launch_overrides_smoke)

option(HYDRA_BUILD_DESKTOP_APP "Build the Qt Quick desktop shell" ON)
if(HYDRA_BUILD_DESKTOP_APP)
    find_package(Qt6 QUIET COMPONENTS Gui Qml Quick Widgets)

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/QMLTermWidget")
    configure_file(third_party/qmltermwidget_runtime/QMLTermWidget/qmldir
                   "${CMAKE_CURRENT_BINARY_DIR}/QMLTermWidget/qmldir"
                   COPYONLY)
    configure_file(third_party/qmltermwidget_runtime/QMLTermWidget/QMLTermScrollbar.qml
                   "${CMAKE_CURRENT_BINARY_DIR}/QMLTermWidget/QMLTermScrollbar.qml"
                   COPYONLY)
    configure_file(third_party/qmltermwidget_runtime/QMLTermWidget/plugins.qmltypes
                   "${CMAKE_CURRENT_BINARY_DIR}/QMLTermWidget/plugins.qmltypes"
                   COPYONLY)

    set(HYDRA_QMLTERMWIDGET_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/qmltermwidget_runtime")
    set(HYDRA_QMLTERMWIDGET_PLUGIN_DIR "${HYDRA_QMLTERMWIDGET_RUNTIME_DIR}/QMLTermWidget")

    if(TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick AND TARGET Qt6::Widgets
       AND EXISTS "${HYDRA_QMLTERMWIDGET_PLUGIN_DIR}/qmldir")
        qt_policy(SET QTP0004 NEW)

        add_library(hydra_backend_tooling STATIC
            src/app/qml_backend_foreign_types.cpp
        )
        target_include_directories(hydra_backend_tooling
            PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/viewmodels"
        )
        target_link_libraries(hydra_backend_tooling
            PRIVATE
                hydra_core
                Qt6::Core
                Qt6::Gui
                Qt6::Qml
        )
        qt_add_qml_module(hydra_backend_tooling
            URI Hydra.Backend
            VERSION 1.0
            NO_PLUGIN
            OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Hydra/Backend"
            SOURCES
                src/ui/viewmodels/repo_list_model.hpp
                src/ui/viewmodels/provider_list_model.hpp
                src/ui/viewmodels/resume_list_model.hpp
                src/ui/viewmodels/session_list_model.hpp
                src/ui/viewmodels/worktree_list_model.hpp
                src/ui/viewmodels/app_state.hpp
                src/ui/viewmodels/app_state_facets.hpp
                src/ui/viewmodels/shell_state.hpp
                src/ui/viewmodels/terminal_surface_controller.hpp
                src/ui/viewmodels/theme_state.hpp
                src/app/desktop_dialog_bridge.hpp
                src/app/qml_backend_foreign_types.hpp
                src/app/qml_backend_foreign_types.cpp
        )
        hydra_apply_warnings(hydra_backend_tooling)

        set_source_files_properties(
            qml/Hydra/styles/HydraTheme.qml
            qml/Hydra/styles/HydraSounds.qml
            PROPERTIES QT_QML_SINGLETON_TYPE TRUE
        )

        qt_add_executable(hydra
            src/main.cpp
            src/app/desktop_launch_options.cpp
            src/app/desktop_dialog_bridge.cpp
            src/app/qml_backend_registration.cpp
            src/app/desktop_sound_controller.cpp
        )

        target_link_libraries(hydra
            PRIVATE
                hydra_core
                Qt6::Core
                Qt6::Gui
                Qt6::Qml
                Qt6::Quick
                Qt6::Widgets
        )
        target_compile_definitions(hydra PRIVATE HYDRA_BOOTSTRAP_REPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

        qt_add_qml_module(hydra
            URI Hydra
            VERSION 1.0
            NO_IMPORT_SCAN
            QML_FILES
                qml/Hydra/App.qml
                qml/Hydra/components/AppShellSurface.qml
                qml/Hydra/components/AppShortcutHub.qml
                qml/Hydra/components/CloseConfirmDialog.qml
                qml/Hydra/components/ConsoleHeader.qml
                qml/Hydra/components/DetailHelpPanel.qml
                qml/Hydra/components/DividerHandle.qml
                qml/Hydra/components/ExecutePanel.qml
                qml/Hydra/components/FrameCorners.qml
                qml/Hydra/components/FullscreenToggleButton.qml
                qml/Hydra/components/HydraButton.qml
                qml/Hydra/components/HelpSectionCard.qml
                qml/Hydra/components/HoverHintBubble.qml
                qml/Hydra/components/HydraLogo.qml
                qml/Hydra/components/LaunchSidebar.qml
                qml/Hydra/components/RailSidebarShell.qml
                qml/Hydra/components/QuickHelpBubble.qml
                qml/Hydra/components/SectionHeader.qml
                qml/Hydra/components/ResumePanel.qml
                qml/Hydra/components/SessionsSurface.qml
                qml/Hydra/components/ShellBackdrop.qml
                qml/Hydra/components/LaunchDropdownField.qml
                qml/Hydra/components/ActionChip.qml
                qml/Hydra/components/BootScreen.qml
                qml/Hydra/components/PromptEditorSection.qml
                qml/Hydra/components/ProviderRuntimeConfigSection.qml
                qml/Hydra/components/NousSplashScreen.qml
                qml/Hydra/components/StatusChip.qml
                qml/Hydra/components/StartupOverlayStack.qml
                qml/Hydra/components/SurfacePanel.qml
                qml/Hydra/components/TerminalSurfacePanel.qml
                qml/Hydra/components/TargetMapPanel.qml
                qml/Hydra/components/ThemeCycleButton.qml
                qml/Hydra/components/InfoDotButton.qml
                qml/Hydra/components/LaunchActionButton.qml

                qml/Hydra/components/RailAtmosphere.qml
                qml/Hydra/components/NativeTerminalViewport.qml

                qml/Hydra/components/MasterConfigDialog.qml
                qml/Hydra/components/MasterTerminalPane.qml
                qml/Hydra/components/MasterTerminalSurface.qml
                qml/Hydra/components/RouterConfigDialog.qml
                qml/Hydra/components/SessionOrbitStrip.qml
                qml/Hydra/components/SessionSyncNode.qml
                qml/Hydra/components/AggregatePulseRing.qml
                qml/Hydra/components/HeartbeatTrace.qml
                qml/Hydra/components/RouterPanel.qml
                qml/Hydra/components/RepoCard.qml
                qml/Hydra/components/SessionBoard.qml
                qml/Hydra/components/SessionCard.qml
                qml/Hydra/components/SessionTimelinePanel.qml
                qml/Hydra/components/WorkbenchPane.qml
                qml/Hydra/components/WorktreeCard.qml
                qml/Hydra/styles/HydraTheme.qml
                qml/Hydra/styles/HydraSounds.qml
            IMPORTS
                Hydra.Backend
            RESOURCES
                qml/Hydra/HelpCatalog.js
                qml/Hydra/assets/hydra_logo_mono.png
                qml/Hydra/assets/nous_logo.png
                qml/Hydra/assets/sounds/ui_boot.wav
                qml/Hydra/assets/sounds/ui_dialog_warning.wav
                qml/Hydra/assets/sounds/ui_incoming_im.wav
                qml/Hydra/assets/sounds/ui_incoming_mail.wav
                qml/Hydra/assets/sounds/ui_keyboard_clicking.wav
                qml/Hydra/assets/sounds/ui_nous_splash.wav
                qml/Hydra/assets/sounds/ui_rollover.wav
                qml/Hydra/assets/sounds/ui_select_popup.wav
                qml/Hydra/components/qmldir
                qml/Hydra/styles/qmldir
        )

        set_target_properties(hydra PROPERTIES
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME hydra
        )

        hydra_apply_warnings(hydra)

        add_executable(hydra_boot_probe
            src/tools/boot_probe_main.cpp
            src/app/desktop_launch_options.cpp
            src/app/desktop_dialog_bridge.cpp
            src/app/qml_backend_registration.cpp
            src/app/desktop_sound_controller.cpp
        )
        target_link_libraries(hydra_boot_probe
            PRIVATE
                hydra_core
                Qt6::Core
                Qt6::Gui
                Qt6::Qml
                Qt6::Quick
                Qt6::Widgets
        )
        target_compile_definitions(hydra_boot_probe PRIVATE HYDRA_BOOTSTRAP_REPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
        hydra_apply_warnings(hydra_boot_probe)

        install(TARGETS hydra
            BUNDLE DESTINATION .
            RUNTIME DESTINATION bin
        )

    elseif(TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick AND TARGET Qt6::Widgets)
        message(WARNING
            "Hydra desktop shell target disabled because qmltermwidget runtime files were not found at "
            "${HYDRA_QMLTERMWIDGET_PLUGIN_DIR}. Run tools/prepare_qmltermwidget.sh to generate the runtime import."
        )
    else()
        message(WARNING
            "Qt6 Gui/Qml/Quick/Widgets components were not found. The Hydra desktop shell target "
            "is disabled for this environment. Install the Qt declarative development "
            "packages to build the QML application target."
        )
    endif()
endif()

option(HYDRA_BUILD_TESTS "Build Hydra tests" OFF)
if(HYDRA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
