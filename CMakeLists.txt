cmake_minimum_required(VERSION 3.24)

project(Hydra VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(HydraWarnings)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Sql)

qt_standard_project_setup(REQUIRES 6.5)

set(HYDRA_CORE_SOURCES
    src/app/hydra_application.cpp
    src/domain/models/session_state.cpp
    src/domain/services/repo_registry.cpp
    src/domain/services/session_supervisor.cpp
    src/domain/services/worktree_manager.cpp
    src/domain/support/slug.cpp
    src/infrastructure/git/git_repo_workspace.cpp
    src/infrastructure/mux/tmux_adapter.cpp
    src/infrastructure/process/process_runner.cpp
    src/infrastructure/persistence/database_manager.cpp
    src/infrastructure/persistence/sqlite_repo_store.cpp
    src/infrastructure/persistence/sqlite_session_store.cpp
    src/ui/viewmodels/app_state.cpp
    src/ui/viewmodels/repo_list_model.cpp
    src/ui/viewmodels/session_list_model.cpp
    src/ui/viewmodels/worktree_list_model.cpp
)

add_library(hydra_core STATIC ${HYDRA_CORE_SOURCES})
target_include_directories(hydra_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(hydra_core PRIVATE HYDRA_BOOTSTRAP_REPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(hydra_core
    PUBLIC
        Qt6::Core
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
        Qt6::Sql
)
hydra_apply_warnings(hydra_core_smoke)

option(HYDRA_BUILD_DESKTOP_APP "Build the Qt Quick desktop shell" ON)
if(HYDRA_BUILD_DESKTOP_APP)
    find_package(Qt6 QUIET COMPONENTS Gui Qml Quick)

    if(TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick)
        qt_policy(SET QTP0004 NEW)

        set_source_files_properties(
            qml/Hydra/styles/HydraTheme.qml
            PROPERTIES QT_QML_SINGLETON_TYPE TRUE
        )

        qt_add_executable(hydra
            src/main.cpp
            src/app/desktop_launch_options.cpp
        )

        target_link_libraries(hydra
            PRIVATE
                hydra_core
                Qt6::Core
                Qt6::Gui
                Qt6::Qml
                Qt6::Quick
        )

        qt_add_qml_module(hydra
            URI Hydra
            VERSION 1.0
            QML_FILES
                qml/Hydra/App.qml
                qml/Hydra/components/ConsoleHeader.qml
                qml/Hydra/components/DetailHelpPanel.qml
                qml/Hydra/components/DividerHandle.qml
                qml/Hydra/components/ExecutePanel.qml
                qml/Hydra/components/FrameCorners.qml
                qml/Hydra/components/HelpSectionCard.qml
                qml/Hydra/components/HoverHintBubble.qml
                qml/Hydra/components/LaunchSidebar.qml
                qml/Hydra/components/LaunchBusPanel.qml
                qml/Hydra/components/QuickHelpBubble.qml
                qml/Hydra/components/SectionHeader.qml
                qml/Hydra/components/SessionsSurface.qml
                qml/Hydra/components/ShellBackdrop.qml
                qml/Hydra/components/StatusChip.qml
                qml/Hydra/components/SurfacePanel.qml
                qml/Hydra/components/TargetMapPanel.qml
                qml/Hydra/components/InfoDotButton.qml
                qml/Hydra/components/MetricTile.qml
                qml/Hydra/components/RepoCard.qml
                qml/Hydra/components/SessionBoard.qml
                qml/Hydra/components/SessionCard.qml
                qml/Hydra/components/WorktreeCard.qml
                qml/Hydra/styles/HydraTheme.qml
            RESOURCES
                qml/Hydra/HelpCatalog.js
                qml/Hydra/components/qmldir
                qml/Hydra/styles/qmldir
        )

        set_target_properties(hydra PROPERTIES
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME hydra
        )

        hydra_apply_warnings(hydra)

        install(TARGETS hydra
            BUNDLE DESTINATION .
            RUNTIME DESTINATION bin
        )

    else()
        message(WARNING
            "Qt6 Gui/Qml/Quick components were not found. The Hydra desktop shell target "
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
