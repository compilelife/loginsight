import QtQuick 2.0
import QtGraphicalEffects 1.15

Rectangle {
  property string message: ''
  width: content.width + 40
  height: 40
  radius: 20
  color: '#e0000000'
  anchors {
    horizontalCenter: parent.horizontalCenter
    bottom: parent.bottom
    bottomMargin: 20
  }

  Text {
    id: content
    text: message
    font.pixelSize: 14
    color: 'white'
    wrapMode: Text.NoWrap
    anchors.centerIn: parent
  }

  function show(txt, duration=2000) {
    message = txt
    visible = true

    hideTimer.interval = duration
    hideTimer.running = true
  }

  Timer {
    id: hideTimer
    repeat: false
    onTriggered: visible = false
  }
}
