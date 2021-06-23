import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Example 2.0
Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Pi Network Stress Test")



    //instance the stellar gateway, it builds the StellarGateway object
    StellarGateway{
        id: testHorizonGateway;
    }

    Wallet{
        id:localWallet;
        gateway : testHorizonGateway;//assign gateway to wallet object, it calls the WRITE method of the property
    }


    ColumnLayout{

        anchors.fill: parent;
        anchors.margins: 5;
        RowLayout{
            visible: localWallet.publicAddress=="";
            TextField{
                id:importText;
                enabled:true;
                visible: localWallet.publicAddress=="";
                Layout.fillWidth: true;
            }
            Button{
                id:importButton;
                text:qsTr("Import");
                //visible: localWallet.publicAddress=="";
                onClicked: {
                    localWallet.import(importText.text);
                }
            }
        }
        RowLayout{
            visible: localWallet.publicAddress!="";
            Label{
                text:qsTr("Account");
            }
            TextField{
                text: localWallet.publicAddress;//it binds publicAddress to text, each time publicAddress signal is emitted, it will be updated
                enabled:true;
                readOnly: true;
                Layout.fillWidth: true;
            }
            Button{
                id:fundButton;
                visible: true;
                enabled:localWallet.publicAddress!="" && !localWallet.funded;
                text:qsTr("Check Balance");
                onClicked: {
                    localWallet.fund();
                }
            }
        }
        RowLayout{
            Label{
                text:qsTr("Balance");
            }
            TextField{
                text:localWallet.balance;
                enabled:false;
            }
        }
        PaymentGUI{
            sourceWallet: localWallet;
        }
    }
}
