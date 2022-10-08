import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

Rectangle {
  property string message: ''
  id: root
  width: content.width + 10
  height: 40
  color: '#e0ff0000'

  RowLayout {
    id: content
    height: parent.height
    spacing: 20

    Text {
      id: txt
      Layout.leftMargin: 10
      text: message
      color: 'white'
      wrapMode: Text.NoWrap
    }

    Button {
      id: btn
      text: '知道了'
      contentItem: Text {
        text: parent.text
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
        color: 'white'
      }
      onClicked: root.visible = false
    }
  }

  function show(txt, type='info', duration=2000) {
    message = txt
    visible = true

    color = type === 'info' ? '#e04CAF50' : '#e0ff0000'

    if (duration > 0) {
      btn.visible = false
      hideTimer.running = false
      hideTimer.interval = duration
      hideTimer.running = true
    } else {
      btn.visible = true
    }
  }

  Timer {
    id: hideTimer
    repeat: false
    onTriggered: visible = false
  }
}
