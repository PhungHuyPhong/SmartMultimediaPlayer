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
    visibility: "FullScreen"
    title: qsTr("Smart Multimedia Player")

    Component.onCompleted: {
            mEngine.videoSink = videoOutputId.videoSink
    }

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
                        id: timeTempLabel
                        text: Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm") + "   " + canManager.temperature.toFixed(1) + "°C"
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
                                   timeTempLabel.text = Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm") + "   " + canManager.temperature.toFixed(1) + "°C"
                               }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Bluetooth"
                        rightPadding: 8
                        onClicked: {
                            bluetoothDialog.open()
                            btManager.clearDevices()
                        }
                    }
                }
    }

        //Title
        Label {
            anchors.top: topBar.bottom
            anchors.topMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter
            id: mediaTitle
            text: mEngine.title
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenterOffset: 0
            font.pointSize: 20
            Layout.fillHeight: false
            Layout.fillWidth: true
            color: "black"
            Layout.alignment: Qt.AlignCenter
        }

        // Video view
        Rectangle{
            id: videoBackground
            anchors.top: mediaTitle.bottom
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            height: 380
            color: "black"
            VideoOutput {
                id: videoOutputId
                anchors.fill: parent
               // visible: mEngine.hasVideo
                fillMode: VideoOutput.PreserveAspectFit
            }
        }

        // Progress bar
        RowLayout {
            id: progressBar
            anchors.top: videoBackground.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Label { text: Qt.formatTime(new Date(mEngine.position), "mm:ss") }
            Slider {
                height: 40
                Layout.fillWidth: true
                from: 0; to: mEngine.duration
                value: mEngine.position
                Layout.preferredWidth: 680
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
                implicitWidth: 50
                implicitHeight: 50
                radius: 25
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
                implicitWidth: 50
                implicitHeight: 50
                radius: 25
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: "qrc:/Icons/icons/previous.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: a2dpManager.isConnected ? a2dpManager.previous() : mEngine.previous()
            }
            RoundButton {
                id: playButton
                implicitWidth: 70
                implicitHeight: 70
                radius: 35
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: mEngine.playing ? "qrc:/Icons/icons/pause.svg" : "qrc:/Icons/icons/play.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: a2dpManager.isConnected ? a2dpManager.playPause() : mEngine.playPause()
            }
            RoundButton {
                id: nextButton
                implicitWidth: 50
                implicitHeight: 50
                radius: 25
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: "qrc:/Icons/icons/next.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: a2dpManager.isConnected ? a2dpManager.next() : mEngine.next()
            }
            RoundButton {
                id: loopButton
                implicitWidth: 50
                implicitHeight: 50
                radius: 25
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


            RoundButton {
                id: muteButton
                implicitWidth: 50
                implicitHeight: 50
                radius: 25
                palette.button: "#7f8fa6"
                contentItem: Image {
                    source: mEngine.muted ? "qrc:/Icons/icons/muted.svg" : "qrc:/Icons/icons/volume.svg"
                    anchors.centerIn: parent
                    width: parent.implicitWidth * 0.5
                    height: parent.implicitHeight * 0.5
                }
                onClicked: mEngine.toggleMute()
            }
            Slider {
                from: 0; to: 100
                value: mEngine.volume
                onMoved: mEngine.setVolume(value)
            }

        }

        Connections {
                target: canManager
                function onGestureChanged(gesture) {
                    /*
                     * Bạn cần gán gestureId cụ thể cho hành động:
                     * 1: Bài tiếp, 2: Bài trước, 3: Play/Pause, 4: Tăng âm lượng, 5: Giảm âm lượng
                     */
                    if (gesture === 1) {
                        // Qua bài mới; ưu tiên A2DP nếu đang kết nối
                        if (a2dpManager.isConnected)
                            a2dpManager.next();
                        else
                            mEngine.next();
                    } else if (gesture === 2) {
                        if (a2dpManager.isConnected)
                            a2dpManager.previous();
                        else
                            mEngine.previous();
                    } else if (gesture === 3) {
                        if (a2dpManager.isConnected)
                            a2dpManager.playPause();
                        else
                            mEngine.playPause();
                    } else if (gesture === 4) {
                        // Tăng âm lượng lên 5 đơn vị
                        let vol = Math.min(mEngine.volume + 5, 100);
                        mEngine.setVolume(vol);
                    } else if (gesture === 5) {
                        // Giảm âm lượng xuống 5 đơn vị
                        let vol = Math.max(mEngine.volume - 5, 0);
                        mEngine.setVolume(vol);
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
