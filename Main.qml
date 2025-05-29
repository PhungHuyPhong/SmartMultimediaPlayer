import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtMultimedia

Window {
    width: 800
    height: 600
    visible: true
    title: qsTr("Smart Multimedia")








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
