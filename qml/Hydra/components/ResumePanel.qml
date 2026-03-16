pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    property var helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    property var selectedSessionIds: ({})
    property int selectedCount: 0
    property real preservedListContentY: -1
    readonly property real maxListHeight: tightMode ? 176 : (denseMode ? 220 : 268)
    readonly property int listCount: listView.count

    visible: appState.resumableSessionCount > 0
    implicitHeight: visible ? panel.implicitHeight : 0

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    function isSelected(sessionId) {
        return selectedSessionIds[sessionId] === true
    }

    function clearSelection() {
        selectedSessionIds = ({})
        selectedCount = 0
    }

    function toggleSelection(sessionId) {
        const next = Object.assign({}, selectedSessionIds)
        if (next[sessionId] === true) {
            delete next[sessionId]
        } else {
            next[sessionId] = true
        }
        selectedSessionIds = next

        let count = 0
        for (const key in next) {
            if (next[key] === true) {
                count += 1
            }
        }
        selectedCount = count
    }

    function selectedIdsList() {
        const ids = []
        for (const key in selectedSessionIds) {
            if (selectedSessionIds[key] === true) {
                ids.push(key)
            }
        }
        return ids
    }

    function dropMissingSelections() {
        if (selectedCount === 0) {
            return
        }

        const validIds = {}
        for (let row = 0; row < listView.count; ++row) {
            const sessionId = root.appState.resumeSessionIdAt(row)
            if (sessionId.length > 0) {
                validIds[sessionId] = true
            }
        }

        const next = {}
        let count = 0
        for (const key in selectedSessionIds) {
            if (selectedSessionIds[key] === true && validIds[key] === true) {
                next[key] = true
                count += 1
            }
        }

        selectedSessionIds = next
        selectedCount = count
    }

    SurfacePanel {
        id: panel

        anchors.left: parent.left
        anchors.right: parent.right
        panelColor: HydraTheme.railPanelStrong
        panelBorderColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
        contentMargin: root.tightMode ? HydraTheme.space8 : HydraTheme.space10
        contentSpacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space10
        showHexGrid: HydraTheme.currentThemeId === "eva"

        SectionHeader {
            Layout.fillWidth: true
            title: "RESUME"

            InfoDotButton {
                topicId: "resume"
                briefText: "Closed provider sessions that Hydra can reopen with provider-native resume."
                accessibleLabel: "Explain resume list"
                hoverHost: root.helpHost
                onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
            }
        }

        Text {
            Layout.fillWidth: true
            text: "closed resumable sessions // use resume to reopen // use select for bulk delete"
            color: HydraTheme.accentSteelBright
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space8

            Text {
                Layout.fillWidth: true
                text: root.selectedCount > 0
                      ? root.selectedCount + " selected // " + root.listCount + " stored"
                      : root.listCount + " stored"
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                elide: Text.ElideRight
            }

            HydraButton {
                text: "DELETE"
                accessibleLabel: "Delete selected stored sessions"
                toolTipText: "Delete the currently selected stored sessions from Hydra's resume store."
                hoverHost: root.helpHost
                toneColor: HydraTheme.danger
                enabledState: root.selectedCount > 0
                onTriggered: {
                    const deletedCount = root.appState.deleteResumeSessions(root.selectedIdsList())
                    if (deletedCount > 0) {
                        root.clearSelection()
                    }
                }
            }

            HydraButton {
                text: "CLEAR ALL"
                accessibleLabel: "Delete all stored resumable sessions"
                toolTipText: "Delete every closed resumable provider session currently stored in Hydra."
                hoverHost: root.helpHost
                toneColor: HydraTheme.danger
                enabledState: root.listCount > 0
                onTriggered: {
                    const deletedCount = root.appState.clearAllResumableSessions()
                    if (deletedCount > 0) {
                        root.clearSelection()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(root.maxListHeight,
                                             Math.max(listView.contentHeight, root.tightMode ? 96 : 112))
            radius: HydraTheme.radius8
            color: HydraTheme.withAlpha(HydraTheme.railPanelStrong, 0.74)
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.panelLeadSoft, 0.18)
            clip: true

            ListView {
                id: listView

                anchors.fill: parent
                anchors.margins: 1
                model: root.appState.resumeModel
                spacing: HydraTheme.space8
                clip: true
                interactive: contentHeight > height
                boundsBehavior: Flickable.StopAtBounds
                reuseItems: false
                ScrollBar.vertical: ScrollBar {
                    policy: listView.contentHeight > listView.height
                            ? ScrollBar.AlwaysOn
                            : ScrollBar.AlwaysOff
                }
                onCountChanged: root.dropMissingSelections()

                Connections {
                    target: listView.model
                    ignoreUnknownSignals: true

                    function onModelAboutToBeReset() {
                        root.preservedListContentY = listView.contentY
                    }

                    function onModelReset() {
                        root.dropMissingSelections()
                        if (root.preservedListContentY < 0) {
                            return
                        }

                        const targetY = Math.min(root.preservedListContentY,
                                                 Math.max(0, listView.contentHeight - listView.height))
                        Qt.callLater(function() {
                            listView.contentY = targetY
                            root.preservedListContentY = -1
                        })
                    }
                }

                delegate: Rectangle {
                    id: resumeEntry

                    required property string sessionId
                    required property string name
                    required property string repoName
                    required property string providerName
                    required property string detailText
                    required property string statusDetail
                    required property string updatedAtText

                    width: listView.width - ((listView.ScrollBar.vertical && listView.ScrollBar.vertical.visible) ? listView.ScrollBar.vertical.width : 0)
                    implicitHeight: contentColumn.implicitHeight + (root.tightMode ? 14 : 18)
                    radius: HydraTheme.radius8
                    color: HydraTheme.railPanelStrong
                    border.width: 1
                    border.color: hoverArea.hovered
                                  ? HydraTheme.borderFocus
                                  : (root.isSelected(resumeEntry.sessionId)
                                     ? HydraTheme.withAlpha(HydraTheme.accentReady, 0.46)
                                     : HydraTheme.withAlpha(HydraTheme.panelLeadSoft, 0.26))
                    Accessible.role: Accessible.Grouping
                    Accessible.name: "Stored session " + resumeEntry.name

                    HoverHandler {
                        id: hoverArea
                    }

                    ColumnLayout {
                        id: contentColumn

                        anchors.fill: parent
                        anchors.margins: root.tightMode ? HydraTheme.space6 : HydraTheme.space9
                        spacing: HydraTheme.space4

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: HydraTheme.space8

                            Text {
                                Layout.fillWidth: true
                                text: resumeEntry.name
                                color: HydraTheme.textOnDark
                                font.family: HydraTheme.displayFamily
                                font.pixelSize: root.tightMode ? 12 : 13
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: resumeEntry.updatedAtText
                                color: HydraTheme.textOnDarkMuted
                                font.family: HydraTheme.monoFamily
                                font.pixelSize: 10
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: resumeEntry.statusDetail
                            color: HydraTheme.accentBronze
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: HydraTheme.space8

                            Text {
                                Layout.fillWidth: true
                                text: resumeEntry.providerName + "  //  " + resumeEntry.repoName + "  //  "
                                      + resumeEntry.detailText
                                color: HydraTheme.textOnDarkMuted
                                font.family: HydraTheme.monoFamily
                                font.pixelSize: 10
                                elide: Text.ElideMiddle
                            }

                            Rectangle {
                                implicitWidth: 54
                                implicitHeight: 18
                                radius: HydraTheme.radius4
                                color: HydraTheme.withAlpha(HydraTheme.accentReady,
                                                            root.isSelected(resumeEntry.sessionId) ? 0.22 : 0.05)
                                border.width: 1
                                border.color: HydraTheme.withAlpha(root.isSelected(resumeEntry.sessionId)
                                                                       ? HydraTheme.accentReady
                                                                       : HydraTheme.textOnDarkMuted,
                                                                   0.48)
                                Accessible.role: Accessible.CheckBox
                                Accessible.name: "Select stored session " + resumeEntry.name
                                Accessible.checked: root.isSelected(resumeEntry.sessionId)

                                Text {
                                    anchors.centerIn: parent
                                    text: root.isSelected(resumeEntry.sessionId) ? "SELECTED" : "SELECT"
                                    color: root.isSelected(resumeEntry.sessionId)
                                           ? HydraTheme.accentReady
                                           : HydraTheme.textOnDarkMuted
                                    font.family: HydraTheme.monoFamily
                                    font.pixelSize: 10
                                    font.bold: true
                                    font.letterSpacing: 0.4
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { HydraSounds.playClick(); root.toggleSelection(resumeEntry.sessionId) }
                                }
                            }

                            Rectangle {
                                implicitWidth: 54
                                implicitHeight: 18
                                radius: HydraTheme.radius4
                                color: HydraTheme.withAlpha(HydraTheme.danger, 0.06)
                                border.width: 1
                                border.color: HydraTheme.withAlpha(HydraTheme.danger, 0.44)
                                Accessible.role: Accessible.Button
                                Accessible.name: "Delete stored session " + resumeEntry.name

                                Text {
                                    anchors.centerIn: parent
                                    text: "DELETE"
                                    color: HydraTheme.danger
                                    font.family: HydraTheme.monoFamily
                                    font.pixelSize: 10
                                    font.bold: true
                                    font.letterSpacing: 0.4
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { HydraSounds.playClick(); root.appState.deleteResumeSessions([resumeEntry.sessionId]) }
                                }
                            }

                            Rectangle {
                                implicitWidth: 54
                                implicitHeight: 18
                                radius: HydraTheme.radius4
                                color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.08)
                                border.width: 1
                                border.color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.44)
                                Accessible.role: Accessible.Button
                                Accessible.name: "Resume stored session " + resumeEntry.name

                                Text {
                                    anchors.centerIn: parent
                                    text: "RESUME"
                                    color: HydraTheme.accentReady
                                    font.family: HydraTheme.monoFamily
                                    font.pixelSize: 10
                                    font.bold: true
                                    font.letterSpacing: 0.4
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { HydraSounds.playClick(); root.appState.resumeSession(resumeEntry.sessionId) }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
