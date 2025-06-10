import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Dialogs

Window {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: qsTr("Smart Multimedia Player")

    FileDialog {
        id: fileDialog
        title: "Select Media Files"
        nameFilters: ["Audio/Video Files (*.mp3 *.wav *.mp4 *.mkv)", "All Files (*)"]
        onAccepted: {
            for (var i = 0; i < fileDialog.selectedFiles.length; ++i) {
                var url = fileDialog.selectedFiles[i];
                mEngine.addToPlaylist(url);
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Select Playlist Folder"
        onAccepted: {
            console.log("Selected folder:", folderDialog.selectedFolder.toString())
            if (folderDialog.selectedFolder) {
                var url = Qt.resolvedUrl(folderDialog.selectedFolder)
                mEngine.addToPlaylist(url)
            }
        }
    }

    Dialog {
        id: bluetoothDialog
        title: "Bluetooth Devices"
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 400
        height: 400

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            anchors.margins: 10

            Button {
                text: btManager.scanning ? "Stop Scan" : "Start Scan"
                onClicked: {
                    if (btManager.scanning)
                        btManager.stopScan()
                    else
                        btManager.startScan()
                }
            }

            Text {
                text: btManager.scanning ? "Scanning..." : "Ready"
                color: "blue"
            }

            ListView {
                id: deviceListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: btManager.deviceNames
                clip: true
                delegate: Rectangle {
                    width: parent.width
                    height: 50
                    color: "lightgray"
                    border.color: "gray"
                    radius: 5

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10
                        anchors.margins: 10

                        Text {
                            text: modelData
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "Connect"
                            onClicked: btManager.connectToDevice(index)
                        }
                    }
                }
            }

            Text {
                id: connectedText
                text: ""
                color: "green"
            }

            Text {
                id: errorText
                text: ""
                color: "red"
            }

            Connections {
                target: btManager
                function onConnectedToDevice(name) {
                    connectedText.text = "Connected to: " + name
                    errorText.text = ""
                }
                function onBluetoothError(error) {
                    errorText.text = error
                    connectedText.text = ""
                }
                function onDeviceListChanged() {
                    console.log("Device list updated")
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        spacing: 0

        // Top bar
        Rectangle {
            id: topBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: "#333"
            RowLayout {
                anchors.fill: parent
                spacing: 20
                Menu {
                    id: openMenu
                    MenuItem {
                        text: "Open Files"
                        onTriggered: fileDialog.open()
                    }
                    MenuItem {
                        text: "Open Folder"
                        onTriggered: folderDialog.open()
                    }
                }
                Button {
                    text: "Open"
                    onClicked: openMenu.open()
                }
                Label {
                    id: timeLabel
                    text: Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm")
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    color: "white"
                    Layout.alignment: Qt.AlignCenter

                    Timer {
                           interval: 1000
                           running: true
                           repeat: true
                           onTriggered: {
                               timeLabel.text = Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm")
                           }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Bluetooth"
                    rightPadding: 8
                    onClicked: bluetoothDialog.open()
                }
            }
        }
        //Title
        Rectangle{
            id: mediaTitle
            anchors.top: topBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#4cd137"
            height: 30
            Label {
                text: mEngine.title
                horizontalAlignment: Text.AlignHCenter
                Layout.fillHeight: false
                Layout.fillWidth: true
                color: "white"
                Layout.alignment: Qt.AlignCenter
            }
        }

        // Video view
        VideoOutput {
            id: videoOutputId
            anchors.top: mediaTitle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 390
            fillMode: VideoOutput.PreserveAspectFit
            // visible: mEngine.hasVideo
        }

        // Progress bar
        RowLayout {
            id: progressBar
            anchors.top: videoOutputId.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Label { text: Qt.formatTime(new Date(mEngine.position), "mm:ss") }
            Slider {
                Layout.fillWidth: true
                from: 0; to: mEngine.duration
                value: mEngine.position
                onMoved: mEngine.setPosition(value)
            }
            Label {
                text: Qt.formatTime(new Date(mEngine.duration), "mm:ss")
            }
        }
        // Playback controls
        RowLayout {
            id: playback
            anchors.top: progressBar.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            RoundButton {
                id: shuffleButton
                implicitWidth: 60
                implicitHeight: 60
                radius: 30
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: mEngine.shuffle ? "qrc:/Icons/icons/shuffle.svg" : "qrc:/Icons/icons/noshuffle.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: mEngine.setShuffle(!mEngine.shuffle)
            }

            RoundButton {
                id: prevButton
                implicitWidth: 60
                implicitHeight: 60
                radius: 30
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: "qrc:/Icons/icons/previous.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: mEngine.previous()
            }
            RoundButton {
                    id: playButton
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 80
                    radius: 40
                    palette.button: "#7f8fa6"
                    contentItem: Image {
                        source: mEngine.playing ? "qrc:/Icons/icons/pause.svg" : "qrc:/Icons/icons/play.svg"
                        anchors.centerIn: parent
                        width: parent.implicitWidth * 0.5
                        height: parent.implicitHeight * 0.5
                    }
                    onClicked: mEngine.playPause()
            }
            RoundButton {
                id: nextButton
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                radius: 30
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: "qrc:/Icons/icons/next.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: mEngine.next()
            }
            RoundButton {
                id: loopButton
                implicitWidth: 60
                implicitHeight: 60
                radius: 30
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: mEngine.loopOne ? "qrc:/Icons/icons/loop1.svg" :
                                              (mEngine.loopAll ? "qrc:/Icons/icons/loop.svg" : "qrc:/Icons/icons/noloop.svg")
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: {
                    if (!mEngine.loopAll && !mEngine.loopOne) {
                        mEngine.setLoopAll(true)
                        mEngine.setLoopOne(false)
                    } else if (mEngine.loopAll && !mEngine.loopOne) {
                        mEngine.setLoopAll(false)
                        mEngine.setLoopOne(true)
                    } else {
                        mEngine.setLoopAll(false)
                        mEngine.setLoopOne(false)
                    }
                }
            }
        }
                // RoundButton {
                //     id: muteButton
                //     Layout.preferredWidth: 60
                //     Layout.preferredHeight: 60
                //     radius: 30
                //     icon.source: mEngine.muted ? "qrc:/Icons/icons/muted.svg" : "qrc:/Icons/icons/volume.svg"
                //     onClicked: mEngine.toggleMute()
                // }
                // Slider {
                //     from: 0; to: 100
                //     value: mEngine.volume
                //     onMoved: mEngine.setVolume(value)
                // }
        //}
    }

    // Playlist
        // ListView {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 100
        //     model: mEngine.playlist
        //     delegate: Text {
        //         text: modelData
        //         font.pixelSize: 14
        //         color: "black"
        //     }
        // }




    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            // Sync progress slider
            if (!mainWindow.visible)
                stop();
        }
    }
}
