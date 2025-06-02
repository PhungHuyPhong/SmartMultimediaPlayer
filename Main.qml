import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs
import QtMultimedia

Window {
    width: 800
    height: 600
    visible: true
    title: qsTr("Smart Multimedia")

    FileDialog {
            id: fileDialog
            title: "Select Media Files"
            nameFilters: [ "Audio or Video Files (*.mp3 *.wav *.mp4 *.mkv)", "All Files (*)" ]
            onAccepted: {
                    for (var i = 0; i < fileDialog.selectedFiles.length; ++i) {
                        var url = Qt.resolvedUrl(fileDialog.selectedFiles[i]);
                        mEngine.addToPlaylist(url);
                    }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            VideoOutput {
                id: videoOutputId
                Layout.fillWidth: true
                Layout.preferredHeight: 300
            }


            RowLayout {
                spacing: 10
                Button {
                    text: "Add Files"
                    onClicked: fileDialog.open()
                }
                Button { text: "Prev"; onClicked: mEngine.previous() }
                Button { text: mEngine.playing ? "Pause" : "Play"; onClicked: mEngine.playPause() }
                Button { text: "Next"; onClicked: mEngine.next() }
                Button { text: mEngine.muted ? "Unmute" : "Mute"; onClicked: mEngine.toggleMute() }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: mEngine.playlist
                delegate: Text { text: modelData }
            }

            RowLayout {
                spacing: 10
                Label { text: Qt.formatTime(new Date(mEngine.position), "mm:ss") }
                Slider {
                    Layout.fillWidth: true
                    from: 0; to: mEngine.duration
                    value: mEngine.position
                    onMoved: mEngine.setPosition(value)
                }
                Label { text: Qt.formatTime(new Date(mEngine.duration), "mm:ss") }
            }

            RowLayout {
                spacing: 10
                Label { text: "Volume" }
                Slider {
                    from: 0; to: 100
                    value: mEngine.volume
                    onValueChanged: {
                            if (pressed) {
                                mEngine.setVolume(value)
                            }
                    }
                }
            }
        }






    // ColumnLayout {
    //         anchors.fill: parent
    //         spacing: 10
    //         anchors.margins: 20

    //         // Scan button
    //         Button {
    //             id: scanButton
    //             text: btManager.scanning ? "Stop Scan" : " Start Scan"
    //             onClicked: {
    //                 if (btManager.scanning)
    //                     btManager.stopScan()
    //                 else
    //                     btManager.startScan()
    //             }
    //         }

    //         // Status text
    //         Text {
    //             id: statusText
    //             text: btManager.scanning ? " Scanning for devices..." : " Ready"
    //             color: "blue"
    //         }

    //         // List of discovered devices
    //         ListView {
    //             id: deviceListView
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //             model: btManager.deviceNames
    //             clip: true

    //             delegate: Rectangle {
    //                 width: parent.width
    //                 height: 50
    //                 color: "lightgray"
    //                 border.color: "gray"
    //                 radius: 5
    //                 Row {
    //                     anchors.fill: parent
    //                     anchors.margins: 10
    //                     spacing: 10

    //                     Text {
    //                         text: modelData
    //                         font.bold: true
    //                         verticalAlignment: Text.AlignVCenter
    //                         elide: Text.ElideRight
    //                         width: parent.width - 100
    //                     }

    //                     Button {
    //                         text: "Connect"
    //                         onClicked: btManager.connectToDevice(index)
    //                     }
    //                 }
    //             }
    //         }

    //         // Connected device info
    //         Text {
    //             id: connectedText
    //             text: ""
    //             color: "green"
    //             font.pixelSize: 16
    //             wrapMode: Text.WordWrap
    //         }

    //         // Error messages
    //         Text {
    //             id: errorText
    //             text: ""
    //             color: "red"
    //             font.pixelSize: 14
    //             wrapMode: Text.WordWrap
    //         }


    //         Connections {
    //             target: btManager

    //             function onConnectedToDevice(name) {
    //                 // Khi signal connectedToDevice(name) được emit
    //                 connectedText.text = " Connected to: " + name
    //                 errorText.text = ""
    //             }

    //             function onBluetoothError(error) {
    //                 // Khi signal bluetoothError(error) được emit
    //                 errorText.text = error
    //                 connectedText.text = ""
    //             }

    //             function onDeviceListChanged() {
    //                 // Khi signal deviceListChanged() được emit
    //                 console.log(" Device list updated")
    //             }
    //         }

    // }
}
