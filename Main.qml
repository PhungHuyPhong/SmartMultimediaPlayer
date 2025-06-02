import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Dialogs

ApplicationWindow {
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
        spacing: 0

        // Top bar
        RowLayout {
            width: parent.width
            height: 40
            Rectangle {
                color: "#333"
                anchors.fill: parent
                RowLayout {
                    anchors.fill: parent
                    spacing: 20
                    Button { text: "Open"; onClicked: fileDialog.open() }
                    Label {
                        text: Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm")
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Button { text: "Bluetooth"; onClicked: bluetoothDialog.open() }
                }
            }
        }

        // Video view
        VideoOutput {
            id: videoOutputId
            width: parent.width
            height: 300
            fillMode: VideoOutput.PreserveAspectFit
            visible: mEngine.hasVideo
        }

        // Playback controls
        RowLayout {
            spacing: 10
          //  padding: 10
            Button { text: "Add Files"; onClicked: fileDialog.open() }
            RoundButton {
                id: prevButton
                width: 100
                height: 100
                radius: width/2
                padding: 0
                palette.button: "#7f8fa6"
                icon.source: "qrc:/Icons/icons/previous.svg"
                // contentItem: Image {
                //     source: "qrc:/Icons/icons/previous.svg"
                //     anchors.centerIn: parent
                //     // width: parent.width * 0.6
                //     // height: parent.height * 0.6
                //    // fillMode: Image.PreserveAspectFit
                // }
                onClicked: mEngine.previous()

            }
            RoundButton {
                id: playButton
                palette.button: "#7f8fa6"
                icon.source: mEngine.playing ? "qrc:/Icons/icons/pause.svg" : "qrc:/Icons/icons/play.svg"
                onClicked: mEngine.playPause()
            }
            RoundButton {
                id: nextButton
                palette.button: "#7f8fa6"
                icon.source: "qrc:/Icons/icons/next.svg"
                onClicked: mEngine.next()
            }
        }

        // Playlist
        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            model: mEngine.playlist
            delegate: Text {
                text: modelData
                font.pixelSize: 14
                color: "black"
            }
        }

        // Progress bar
        RowLayout {
            spacing: 10
          //  padding: 10
            Label { text: Qt.formatTime(new Date(mEngine.position), "mm:ss") }
            Slider {
                Layout.fillWidth: true
                from: 0; to: mEngine.duration
                value: mEngine.position
                onMoved: mEngine.setPosition(value)
            }
            Label { text: Qt.formatTime(new Date(mEngine.duration), "mm:ss") }
        }

        // Volume
        RowLayout {
            spacing: 10
          //  padding: 10
            Button {
                id: muteButton
                icon.source: mEngine.muted ? "qrc:/Icons/icons/muted.svg" : "qrc:/Icons/icons/volume.svg"
                onClicked: mEngine.toggleMute()
            }
            Slider {
                from: 0; to: 100
                value: mEngine.volume
                onMoved: mEngine.setVolume(value)
            }
        }
    }

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
