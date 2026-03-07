pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

SurfacePanel {
    id: root

    required property QtObject appState
    property QtObject helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    property bool canCreateWorktree: false
    contentMargin: denseMode ? HydraTheme.space8 : HydraTheme.space10
    contentSpacing: denseMode ? HydraTheme.space6 : HydraTheme.space8

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    SectionHeader {
        Layout.fillWidth: true
        title: "TARGET MAP"

        InfoDotButton {
            topicId: "target-map"
            briefText: "Choose the repository or linked worktree that new shells should use."
            accessibleLabel: "Explain target map"
            hoverHost: root.helpHost
            onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
        }

        StatusChip {
            toneColor: root.appState.selectedRepoId.length > 0 ? HydraTheme.accentReady : HydraTheme.accentMuted
            textColor: root.appState.selectedRepoId.length > 0 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
            fillOpacity: 0.1
            borderOpacity: 0.3
            minWidth: root.appState.selectedRepoId.length > 0 ? 64 : 78
            text: root.appState.selectedRepoId.length > 0 ? "ONLINE" : "NO TARGET"
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: HydraTheme.space8

        Text {
            text: "Repositories"
            color: HydraTheme.textOnDark
            font.family: HydraTheme.displayFamily
            font.pixelSize: 12
            font.bold: true
            Layout.fillWidth: true
        }

        StatusChip {
            minWidth: 42
            text: root.appState.repoCount
            toneColor: HydraTheme.accentSteel
            textColor: HydraTheme.accentSteelBright
            fillOpacity: 0.08
            borderOpacity: 0.22
            textPixelSize: 10
            verticalPadding: HydraTheme.space4
        }
    }

    Column {
        Layout.fillWidth: true
        spacing: HydraTheme.space6

        Repeater {
            model: root.appState.repoModel

            delegate: Item {
                required property string repoId
                required property string name
                required property string path
                required property string description
                required property color accentColor
                required property bool selected

                width: parent.width
                implicitHeight: repoCard.implicitHeight

                RepoCard {
                    id: repoCard

                    anchors.left: parent.left
                    anchors.right: parent.right
                    hoverHost: root.helpHost
                    compactMode: root.denseMode
                    repoId: parent.repoId
                    repoName: parent.name
                    repoPath: parent.path
                    repoDescription: parent.description
                    accentColor: parent.accentColor
                    selected: parent.selected
                    onActivated: root.appState.selectedRepoId = parent.repoId
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.9)
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: HydraTheme.space8

        Text {
            text: "Worktrees"
            color: HydraTheme.textOnDark
            font.family: HydraTheme.displayFamily
            font.pixelSize: 12
            font.bold: true
            Layout.fillWidth: true
        }

        StatusChip {
            minWidth: 42
            text: root.appState.worktreeCount
            toneColor: HydraTheme.accentSteel
            textColor: HydraTheme.accentSteelBright
            fillOpacity: 0.08
            borderOpacity: 0.22
            textPixelSize: 10
            verticalPadding: HydraTheme.space4
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: HydraTheme.space8

        TextField {
            id: branchField

            Layout.fillWidth: true
            implicitHeight: 34
            enabled: root.canCreateWorktree
            hoverEnabled: true
            selectByMouse: true
            color: HydraTheme.textOnDark
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            placeholderText: "feature/name"
            placeholderTextColor: HydraTheme.textOnLightSoft
            padding: HydraTheme.space10
            Accessible.name: "Worktree branch name"
            onAccepted: {
                if (root.appState.createWorktree(text)) {
                    text = ""
                }
            }

            background: Rectangle {
                radius: HydraTheme.radius6
                color: HydraTheme.railPanelStrong
                border.width: 1
                border.color: branchField.activeFocus ? HydraTheme.borderFocus : HydraTheme.borderDark

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: HydraTheme.withAlpha(branchField.activeFocus
                                                ? HydraTheme.accentSteelBright
                                                : HydraTheme.accentSteel,
                                                0.26)
                }
            }

            HoverHandler {
                enabled: branchField.enabled && !!root.helpHost
                onHoveredChanged: {
                    if (!root.helpHost) {
                        return
                    }
                    if (hovered && root.helpHost.queueHoverHint) {
                        root.helpHost.queueHoverHint("Type a branch name to create a linked Git worktree for the selected repository.",
                                                     branchField)
                    } else if (!hovered && root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(branchField)
                    }
                }
            }
        }

        Rectangle {
            id: createButton

            property bool hovered: false

            implicitWidth: root.tightMode ? 56 : 66
            implicitHeight: 34
            radius: HydraTheme.radius6
            color: root.canCreateWorktree
                   ? (hovered ? Qt.lighter(HydraTheme.accentSteel, 1.06) : HydraTheme.accentSteel)
                   : HydraTheme.accentMuted
            border.width: 1
            border.color: root.canCreateWorktree
                          ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, hovered ? 0.48 : 0.3)
                          : HydraTheme.borderDark
            scale: createArea.pressed && root.canCreateWorktree ? 0.97 : 1.0
            transformOrigin: Item.Center
            Accessible.role: Accessible.Button
            Accessible.name: "Create worktree"
            Accessible.onPressAction: {
                if (root.canCreateWorktree && root.appState.createWorktree(branchField.text)) {
                    branchField.text = ""
                }
            }

            Behavior on scale {
                NumberAnimation {
                    duration: HydraTheme.motionFast
                    easing.type: Easing.OutCubic
                }
            }

            Behavior on color {
                ColorAnimation { duration: HydraTheme.motionFast }
            }

            MouseArea {
                id: createArea

                anchors.fill: parent
                enabled: root.canCreateWorktree
                hoverEnabled: true
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onEntered: {
                    createButton.hovered = true
                    if (root.helpHost && root.helpHost.queueHoverHint) {
                        root.helpHost.queueHoverHint(root.canCreateWorktree
                                                     ? "Create a linked worktree from the typed branch name."
                                                     : "Select a Git repository before creating a worktree.",
                                                     createButton)
                    }
                }
                onExited: {
                    createButton.hovered = false
                    if (root.helpHost && root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(createButton)
                    }
                }
                onClicked: {
                    if (root.appState.createWorktree(branchField.text)) {
                        branchField.text = ""
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: root.tightMode ? "NEW" : "CREATE"
                color: root.canCreateWorktree ? HydraTheme.shellBg : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.displayFamily
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 0.8
            }
        }
    }

    Text {
        visible: !root.denseMode
        text: root.appState.repositoryIsGit
              ? "select a repository or linked worktree"
              : "worktree creation requires a git repository"
        color: root.appState.repositoryIsGit ? HydraTheme.textOnDarkMuted : HydraTheme.warning
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

    Column {
        Layout.fillWidth: true
        spacing: HydraTheme.space6
        visible: root.appState.worktreeCount > 0

        Repeater {
            model: root.appState.worktreeModel

            delegate: Item {
                required property string branchName
                required property string path
                required property bool isMain
                required property bool selected

                width: parent.width
                implicitHeight: worktreeCard.implicitHeight

                WorktreeCard {
                    id: worktreeCard

                    anchors.left: parent.left
                    anchors.right: parent.right
                    hoverHost: root.helpHost
                    compactMode: root.denseMode
                    branchName: parent.branchName
                    path: parent.path
                    isMain: parent.isMain
                    selected: parent.selected
                    onActivated: selectedPath => root.appState.selectedWorktreePath = selectedPath
                }
            }
        }
    }

    Text {
        visible: root.appState.worktreeCount === 0
        text: root.appState.selectedRepoId.length === 0
              ? "select a repository to inspect worktrees"
              : "no linked worktrees yet; the repo root stays selected"
        wrapMode: Text.WordWrap
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: root.denseMode ? 9 : 10
        Layout.fillWidth: true
    }
}
