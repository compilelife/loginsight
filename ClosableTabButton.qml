import QtQuick 2.0
import QtQuick.Controls 2.15

TabButton {
  property string title: ''
  signal closed()

  width: implicitWidth

  background: Rectangle {color: checked ? 'white' : 'transparent'}

  contentItem: Row {
    Text {
      text: title+' '
      verticalAlignment: Text.AlignVCenter
      height: parent.height
    }
    Text {
      text: ' X'
      verticalAlignment: Text.AlignVCenter
      height: parent.height
      MouseArea {
        anchors.fill: parent
        onClicked: closed()
      }
    }
  }
}
