import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import StressTest 2.0

ColumnLayout {
    id: paymentGui
    property Wallet sourceWallet

    readonly property bool fundedTarget: targetWallet.funded
    readonly property bool validTarget: targetWallet.publicAddress != ""
    Wallet {
        id: targetWallet
        gateway: testHorizonGateway //assign gateway to wallet object, it calls the WRITE method of the property
        //publicAddress: destinationField.text
    }
    RowLayout {
//        Label {
//            text: qsTr("Payment")
//        }
        Button {
            text: qsTr("start")
            enabled: true
            hoverEnabled: true
            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Bắt đầu gửi tự động.")
            onClicked: {
                //aymentGui.enabled=false;// pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
                //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
                sourceWallet.startPay(intervalField.text,
                                      destinationField.text, amountField.text,
                                      memoField.text)
            }
        }
        Button {
            text: qsTr("stop")
            enabled: true
            visible: true
            hoverEnabled: true
            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Chấm dứt gửi tự động.")
            onClicked: {
                //paymentGui.enabled=false;// pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
                //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
                sourceWallet.stopPay()
            }
        }
        Button {
            text: qsTr("Pay")
            enabled: destinationField.text !== ""
            visible: true
            hoverEnabled: true
            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Chỉ gửi một lần.")
            onClicked: {
                paymentGui.enabled = false // pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
                //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
                sourceWallet.pay(destinationField.text, amountField.text,
                                 memoField.text)
            }
        }
    }

    RowLayout {
        Label {
            text: qsTr("Intverval")
        }
        TextField {
            id: intervalField
            placeholderText: "30 seconds"
            onFocusChanged: {
                if (focus)
                    selectAll()
            }
        }
    }
    RowLayout {
        Label {
            text: qsTr("Amount")
        }
        TextField {
            id: amountField
            placeholderText: "0.0001"
            onFocusChanged: {
                if (focus)
                    selectAll()
            }
        }
        Label {
            text: qsTr("Memo")
        }
        TextField {
            id: memoField
            placeholderText: "STRESS TEST"
            onFocusChanged: {
                if (focus)
                    selectAll()
            }
        }
    }
    RowLayout {}
    RowLayout {
        Label {
            text: qsTr("Destination")
        }
        Label {
            text: qsTr("      ")
        }
        Label {
            text: qsTr("Total:") + sourceWallet.totalPay
        }
    }

    RowLayout {
        width: window.width
        height: window.height / 4


        /*
        TextArea {
            id: destinationField
            anchors.fill: parent
            //width: parent.width
            //height: window.height / 8
            //Layout.fillWidth: true
            //Layout.fillHeight: true
            //Layout.alignment: Qt.AlignTop
            wrapMode: TextEdit.WordWrap
            readOnly: false
            textMargin: 0.0
            selectByKeyboard: true
            selectByMouse: true
            //background: null
            padding: 0

            Layout.fillWidth: true
            onFocusChanged: {
                if (focus)
                    selectAll()
            }
        }*/
        Flickable {
            id: flickable
            width: parent.width
            height: Math.min(contentHeight, 100)
            contentWidth: width
            contentHeight: textArea.implicitHeight

            TextArea.flickable: TextArea {
                id: destinationField
                width: parent.width
                //text: qsTr("Hello, world! Hello, world! Hello, world! Hello, world! ")
                wrapMode: Text.WordWrap
                onFocusChanged: {
                    if (focus)
                        selectAll()
                }
            }
            ScrollBar.vertical: ScrollBar {}
        }
    }
    Text {
        id: errorText
    }
    RowLayout {}
    Label {
        id: statusLabel
        //color: Qt.rgba(1, 0, 0, 1)
        visible: true
        wrapMode: Label.WordWrap
        width: window.width

        //anchors.margins: 20
        //y: destinationField.bottom
        //anchors.left: parent.left
        //anchors.right: parent.right
        //anchors.bottom: parent.bottom
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
        text: sourceWallet.lastDestination
    }

    //this is a way to connect signals to functions on the GUI
    Connections {
        target: sourceWallet
        onSuccess: {
            targetWallet.update()
            paymentGui.enabled = true
            statusLabel.text = sourceWallet.totalPay + ": Success =>" + sourceWallet.lastDestination
        }
        onError: {
            paymentGui.enabled = true
            statusLabel.text = sourceWallet.totalPay + ": " + lastError + " =>"
                    + sourceWallet.lastDestination
        }
    }
}
