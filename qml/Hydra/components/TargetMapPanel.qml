pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

SurfacePanel {
    id: root

    required property AppState appState
    property DesktopDialogBridge desktopBridge: null
    property var helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    property bool canCreateWorktree: false
    property bool worktreesExpanded: false
    readonly property bool hasRepository: root.appState.selectedRepoId.length > 0
    readonly property string worktreeHeaderTitle: root.hasRepository
                                                  ? "WORKTREES // " + root.activeWorktreeLabel
                                                  : "WORKTREES"
    readonly property string activeWorktreeLabel: {
        if (!root.hasRepository) {
            return "LOCKED"
        }
        const branch = root.appState.selectedWorktreeBranch.trim()
        return branch.length > 0 ? branch.toUpperCase() : "ROOT"
    }
    showHexGrid: true
    panelColor: HydraTheme.railPanelStrong
    panelBorderColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
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
            briefText: "Choose the repository root or open the worktree drawer when you need a branch-isolated launch target."
            accessibleLabel: "Explain target map"
            hoverHost: root.helpHost
            onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
        }

        StatusChip {
            toneColor: root.hasRepository ? HydraTheme.accentReady : HydraTheme.accentMuted
            textColor: root.hasRepository ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
            fillOpacity: 0.1
            borderOpacity: 0.3
            minWidth: root.hasRepository ? 96 : 78
            text: root.hasRepository ? "TARGET READY" : (root.appState.launchInHomeDirectory ? "HOME" : "NO TARGET")
        }

        Rectangle {
            id: unbindTargetButton

            property bool hovered: false

            visible: root.hasRepository
            implicitWidth: 62
            implicitHeight: 24
            activeFocusOnTab: true
            radius: HydraTheme.radius4
            color: unbindTargetButton.hovered
                   ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.2)
                   : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.1)
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.accentBronze,
                                               unbindTargetButton.activeFocus
                                               ? 0.78
                                               : (unbindTargetButton.hovered ? 0.64 : 0.38))
            Accessible.role: Accessible.Button
            Accessible.name: "Unbind current target"
            Accessible.onPressAction: root.appState.clearSelectedTarget()

            Behavior on color {
                ColorAnimation { duration: HydraTheme.motionFast }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                cursorShape: Qt.PointingHandCursor
                onEntered: {
                    unbindTargetButton.hovered = true
                    if (root.helpHost && root.helpHost.queueHoverHint) {
                        root.helpHost.queueHoverHint("Unbind the current repository/worktree target and switch launch to HOME mode.",
                                                     unbindTargetButton)
                    }
                }
                onExited: {
                    unbindTargetButton.hovered = false
                    if (root.helpHost && root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(unbindTargetButton)
                    }
                }
                onClicked: { HydraSounds.playClick(); root.appState.clearSelectedTarget() }
            }

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                    root.appState.clearSelectedTarget()
                    event.accepted = true
                }
            }

            Text {
                anchors.centerIn: parent
                text: "UNBIND"
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
            }
        }
    }

    Text {
        visible: !root.denseMode
        text: root.appState.launchInHomeDirectory
              ? "Launch is currently unbound and will start in your home directory."
              : (root.hasRepository
                 ? "Repo root launches by default. Unbind the target to switch launch back to HOME."
              : (root.appState.repoCount > 0
                 ? "Select a repository or add another one to unlock launch targets."
                 : "Add a repository to unlock launch targets."))
        color: (root.hasRepository || root.appState.launchInHomeDirectory)
               ? HydraTheme.textOnDarkMuted
               : HydraTheme.warning
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.topMargin: HydraTheme.space2
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.topMargin: root.denseMode ? HydraTheme.space4 : HydraTheme.space8
        spacing: HydraTheme.space8

        Text {
            text: "REPOSITORIES"
            color: HydraTheme.textOnDark
            font.family: HydraTheme.displayFamily
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 0.8
            Layout.fillWidth: true
        }

        StatusChip {
            minWidth: 40
            text: root.appState.repoCount
            toneColor: HydraTheme.accentSteel
            textColor: HydraTheme.accentSteelBright
            fillOpacity: 0.08
            borderOpacity: 0.22
            textPixelSize: 10
            verticalPadding: HydraTheme.space3
            horizontalPadding: HydraTheme.space5
        }

    }

    RowLayout {
        Layout.fillWidth: true
        Layout.topMargin: HydraTheme.space2
        spacing: HydraTheme.space8

        TextField {
            id: repoPathField

            Accessible.name: "Repository folder path"
            Layout.fillWidth: true
            placeholderText: "/abs/path/to/repo"
            color: HydraTheme.textOnDark
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            selectByMouse: true

            background: Rectangle {
                radius: HydraTheme.radius6
                color: HydraTheme.withAlpha(HydraTheme.railCardSelected, 0.98)
                border.width: 1
                border.color: repoPathField.activeFocus
                              ? HydraTheme.borderFocus
                              : HydraTheme.borderDark
            }

            onAccepted: {
                if (root.appState.addRepositoryPath(text)) {
                    text = ""
                }
            }
        }

        Rectangle {
            id: browseRepoButton

            property bool hovered: false

            implicitWidth: 70
            implicitHeight: 32
            activeFocusOnTab: true
            radius: HydraTheme.radius4
            color: browseRepoButton.hovered
                   ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.18)
                   : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.1)
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.accentBronze,
                                               browseRepoButton.activeFocus
                                               ? 0.76
                                               : (browseRepoButton.hovered ? 0.62 : 0.34))
            Accessible.role: Accessible.Button
            Accessible.name: "Browse for repository folder"

            Behavior on color {
                ColorAnimation { duration: HydraTheme.motionFast }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                cursorShape: Qt.PointingHandCursor
                onEntered: {
                    browseRepoButton.hovered = true
                    if (root.helpHost && root.helpHost.queueHoverHint) {
                        root.helpHost.queueHoverHint("Open a folder picker and register the selected repository path.",
                                                     browseRepoButton)
                    }
                }
                onExited: {
                    browseRepoButton.hovered = false
                    if (root.helpHost && root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(browseRepoButton)
                    }
                }
                onClicked: {
                    HydraSounds.playClick()
                    if (!root.desktopBridge || !root.desktopBridge.browseForFolder) {
                        return
                    }

                    const pickedPath = root.desktopBridge.browseForFolder(repoPathField.text)
                    if (!pickedPath || pickedPath.length === 0) {
                        return
                    }

                    repoPathField.text = pickedPath
                    if (root.appState.addRepositoryPath(pickedPath)) {
                        repoPathField.text = ""
                    }
                }
            }

            Keys.onPressed: event => {
                if (event.key !== Qt.Key_Return && event.key !== Qt.Key_Enter && event.key !== Qt.Key_Space) {
                    return
                }
                if (!root.desktopBridge || !root.desktopBridge.browseForFolder) {
                    event.accepted = true
                    return
                }
                const pickedPath = root.desktopBridge.browseForFolder(repoPathField.text)
                if (pickedPath && pickedPath.length > 0) {
                    repoPathField.text = pickedPath
                    if (root.appState.addRepositoryPath(pickedPath)) {
                        repoPathField.text = ""
                    }
                }
                event.accepted = true
            }

            Text {
                anchors.centerIn: parent
                text: "BROWSE"
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
            }
        }

        Rectangle {
            id: addRepoButton

            property bool hovered: false

            implicitWidth: 52
            implicitHeight: 32
            activeFocusOnTab: true
            radius: HydraTheme.radius4
            color: addRepoButton.hovered
                   ? HydraTheme.withAlpha(HydraTheme.accentSteel, 0.18)
                   : HydraTheme.withAlpha(HydraTheme.accentSteel, 0.1)
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.accentSteelBright,
                                               addRepoButton.activeFocus
                                               ? 0.76
                                               : (addRepoButton.hovered ? 0.62 : 0.34))
            Accessible.role: Accessible.Button
            Accessible.name: "Add repository"
            Accessible.onPressAction: {
                if (root.appState.addRepositoryPath(repoPathField.text)) {
                    repoPathField.text = ""
                }
            }

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                    if (root.appState.addRepositoryPath(repoPathField.text)) {
                        repoPathField.text = ""
                    }
                    event.accepted = true
                }
            }

            Behavior on color {
                ColorAnimation { duration: HydraTheme.motionFast }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                cursorShape: Qt.PointingHandCursor
                onEntered: {
                    addRepoButton.hovered = true
                    if (root.helpHost && root.helpHost.queueHoverHint) {
                        root.helpHost.queueHoverHint("Register the typed folder path as another Hydra repository.",
                                                     addRepoButton)
                    }
                }
                onExited: {
                    addRepoButton.hovered = false
                    if (root.helpHost && root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(addRepoButton)
                    }
                }
                onClicked: {
                    HydraSounds.playClick()
                    if (root.appState.addRepositoryPath(repoPathField.text)) {
                        repoPathField.text = ""
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: "ADD"
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
            }
        }
    }

    Column {
        Layout.fillWidth: true
        Layout.topMargin: HydraTheme.space2
        spacing: HydraTheme.space4

        Repeater {
            model: root.appState.repoModel

            delegate: Item {
                required property string repoId
                required property string name
                required property string path
                required property color accentColor
                required property bool selected

                width: parent.width
                implicitHeight: repoCard.implicitHeight

                RepoCard {
                    id: repoCard

                    anchors.left: parent.left
                    anchors.right: parent.right
                    hoverHost: root.helpHost
                    compactMode: root.tightMode
                    repoId: parent.repoId
                    repoName: parent.name
                    repoPath: parent.path
                    accentColor: parent.accentColor
                    selected: parent.selected
                    onActivated: root.appState.selectedRepoId = parent.repoId
                    onRemoveRequested: targetRepoId => root.appState.deleteRepository(targetRepoId)
                }
            }
        }

        Text {
            visible: root.appState.repoCount === 0
            text: "No repositories tracked yet. Use ADD to register a folder."
            color: HydraTheme.textOnDarkMuted
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            wrapMode: Text.WordWrap
            width: parent.width
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.topMargin: HydraTheme.space2
        implicitHeight: 1
        color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.9)
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.topMargin: HydraTheme.space2
        implicitHeight: 36
        radius: HydraTheme.radius6
        color: HydraTheme.withAlpha(HydraTheme.textOnDark, 0.03)
        border.width: 1
        border.color: HydraTheme.borderDark

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: HydraTheme.space12
            anchors.rightMargin: HydraTheme.space10
            spacing: HydraTheme.space10

            Text {
                text: root.worktreeHeaderTitle
                color: HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 0.8
                Layout.fillWidth: true
            }

            StatusChip {
                minWidth: 40
                text: root.appState.worktreeCount
                toneColor: HydraTheme.accentSteel
                textColor: HydraTheme.accentSteelBright
                fillOpacity: 0.08
                borderOpacity: 0.22
                textPixelSize: 10
                verticalPadding: HydraTheme.space3
                horizontalPadding: HydraTheme.space5
            }

            Rectangle {
                id: worktreeToggle

                property bool hovered: false

                implicitWidth: 54
                implicitHeight: 24
                activeFocusOnTab: true
                radius: HydraTheme.radius4
                color: worktreeToggle.hovered
                       ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.18)
                       : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.1)
                border.width: 1
                border.color: HydraTheme.withAlpha(HydraTheme.accentBronze,
                                                   worktreeToggle.activeFocus
                                                   ? 0.76
                                                   : (worktreeToggle.hovered ? 0.62 : 0.38))
                Accessible.role: Accessible.Button
                Accessible.name: "Toggle worktrees"
                Accessible.onPressAction: root.worktreesExpanded = !root.worktreesExpanded

                Behavior on color {
                    ColorAnimation { duration: HydraTheme.motionFast }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        worktreeToggle.hovered = true
                        if (root.helpHost && root.helpHost.queueHoverHint) {
                            root.helpHost.queueHoverHint(root.worktreesExpanded
                                                         ? "Hide the worktree drawer."
                                                         : "Open the worktree drawer for advanced repo targets.",
                                                         worktreeToggle)
                        }
                    }
                    onExited: {
                        worktreeToggle.hovered = false
                        if (root.helpHost && root.helpHost.clearHoverHint) {
                            root.helpHost.clearHoverHint(worktreeToggle)
                        }
                    }
                    onClicked: { HydraSounds.playClick(); root.worktreesExpanded = !root.worktreesExpanded }
                }

                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                        root.worktreesExpanded = !root.worktreesExpanded
                        event.accepted = true
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: root.worktreesExpanded ? "HIDE" : "SHOW"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }
    }

    Item {
        id: worktreeDrawer

        property real reveal: root.worktreesExpanded ? 1.0 : 0.0

        Layout.fillWidth: true
        implicitHeight: Math.round(worktreeBody.implicitHeight * reveal)
        opacity: reveal
        clip: true

        Behavior on reveal {
            NumberAnimation {
                duration: HydraTheme.motionNormal
                easing.type: Easing.InOutCubic
            }
        }

        Column {
            id: worktreeBody

            width: parent.width
            y: Math.round((1.0 - worktreeDrawer.reveal) * -8)
            spacing: HydraTheme.space8

            Item {
                width: parent.width
                height: HydraTheme.space2
            }

            RowLayout {
                width: parent.width
                spacing: HydraTheme.space10

                TextField {
                    id: branchField

                    Layout.fillWidth: true
                    implicitHeight: 32
                    enabled: root.canCreateWorktree
                    hoverEnabled: true
                    selectByMouse: true
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 11
                    placeholderText: "feature/name"
                    placeholderTextColor: HydraTheme.textOnLightSoft
                    padding: HydraTheme.space9
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
                                                        0.24)
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

                    implicitWidth: root.tightMode ? 54 : 66
                    implicitHeight: 32
                    activeFocusOnTab: true
                    radius: HydraTheme.radius6
                    color: root.canCreateWorktree
                           ? (createButton.hovered ? Qt.lighter(HydraTheme.accentSteel, 1.06) : HydraTheme.accentSteel)
                           : HydraTheme.accentMuted
                    border.width: 1
                    border.color: root.canCreateWorktree
                                  ? HydraTheme.withAlpha(HydraTheme.accentSteelBright,
                                                         createButton.activeFocus
                                                         ? 0.62
                                                         : (createButton.hovered ? 0.48 : 0.3))
                                  : (createButton.activeFocus ? HydraTheme.borderFocus : HydraTheme.borderDark)
                    scale: createArea.pressed && root.canCreateWorktree ? 0.97 : 1.0
                    transformOrigin: Item.Center
                    Accessible.role: Accessible.Button
                    Accessible.name: "Create worktree"
                    Accessible.onPressAction: {
                        if (root.canCreateWorktree && root.appState.createWorktree(branchField.text)) {
                            branchField.text = ""
                        }
                    }

                    Keys.onPressed: event => {
                        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space)
                                && root.canCreateWorktree) {
                            if (root.appState.createWorktree(branchField.text)) {
                                branchField.text = ""
                            }
                            event.accepted = true
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
                        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
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
                            HydraSounds.playClick()
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

            Item {
                width: parent.width
                height: HydraTheme.space2
            }

            Text {
                text: root.hasRepository
                      ? "Select a linked worktree only when the next shell should start away from the repo root."
                      : "Select a repository to inspect or create worktrees."
                color: root.hasRepository ? HydraTheme.textOnDarkMuted : HydraTheme.warning
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Item {
                width: parent.width
                height: HydraTheme.space2
            }

            Column {
                width: parent.width
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
                            compactMode: root.tightMode
                            branchName: parent.branchName
                            path: parent.path
                            isMain: parent.isMain
                            selected: parent.selected
                            onActivated: selectedPath => root.appState.selectedWorktreePath = selectedPath
                            onRemoveRequested: targetPath => root.appState.deleteWorktree(targetPath)
                        }
                    }
                }
            }

            Text {
                visible: root.appState.worktreeCount === 0
                text: root.hasRepository
                      ? "No linked worktrees yet. The repo root stays active."
                      : "Select a repository to inspect worktrees."
                wrapMode: Text.WordWrap
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                width: parent.width
            }
        }
    }
}
