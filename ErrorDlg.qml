import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

Dialog {
  property alias message: messageLabel.text
  property var oneShotHandler: null

  standardButtons: Dialog.Ok

  RowLayout {
    spacing: 20
    Image {
      source: "qrc:/images/error.png"
    }
    Text {
      id: messageLabel
      font.pointSize: 11
      Layout.alignment: Qt.AlignTop
    }
  }
  onAccepted: {
    if (oneShotHandler)
      oneShotHandler()
    oneShotHandler = null
  }
}
