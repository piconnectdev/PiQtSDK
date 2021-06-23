import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Example 2.0

ColumnLayout{
    id: paymentGui;
    property Wallet sourceWallet;


    readonly property bool fundedTarget : targetWallet.funded;
    readonly property bool validTarget : targetWallet.publicAddress!="";
    Wallet{
        id:targetWallet;
        gateway : testHorizonGateway;//assign gateway to wallet object, it calls the WRITE method of the property
        publicAddress: destinationField.text;
    }
    RowLayout{
        Label{
            text:qsTr("Payment");

        }
        Button{
            text:qsTr("start");
            enabled: fundedTarget;
            onClicked: {
                //aymentGui.enabled=false;// pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
                //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
                sourceWallet.startPay(destinationField.text,amountField.text,memoField.text);
            }
        }
        Button{
            text:qsTr("stop");
            //visible: fundedTarget;
            onClicked: {
                //paymentGui.enabled=false;// pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
                //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
                sourceWallet.stopPay();
            }
        }
    }
    RowLayout{
        Label{
            text:qsTr("Destination");
        }
        TextField{
            id:destinationField;
            Layout.fillWidth: true;
            onFocusChanged:{
                if(focus)
                    selectAll()
            }
        }
        Label{
            color:Qt.rgba(1,0,0,1);
            visible: !validTarget;
            text:qsTr("Invalid destination wallet");
        }
    }

    RowLayout{
        Label{
            text:qsTr("Amount");
        }
        TextField{
            id:amountField;
            onFocusChanged:{
                if(focus)
                    selectAll()
            }
        }
    }
    RowLayout{
        Label{
            text:qsTr("Memo");
        }
        TextField{
            id:memoField;
            onFocusChanged:{
                if(focus)
                    selectAll()
            }
        }
    }
    Button{
        text:qsTr("Pay");
        visible: fundedTarget;
        onClicked: {
            paymentGui.enabled=false;// pay and create if fails, it emit error and invokes onError synchronized, so we have to disable it before.
            //signals and slots are invoked in the same moment if the objects live in the same thread and you don't defer the signal by using a timer.
            sourceWallet.pay(destinationField.text,amountField.text,memoField.text);            
        }
    }
    Button{
        text:qsTr("Create");
        visible: validTarget && !fundedTarget;
        onClicked: {
            paymentGui.enabled=false;
            sourceWallet.create(destinationField.text,amountField.text,memoField.text);            
        }
    }
    //this is a way to connect signals to functions on the GUI
    Connections{
        target:sourceWallet;
        onSuccess:{
            targetWallet.update();
            paymentGui.enabled=true;
            errorText.text = qsTr("Success!");
        }
        onError:{
            paymentGui.enabled=true;
            errorText.text = lastError;
        }
    }
    Text{
        id:errorText;
    }
}
