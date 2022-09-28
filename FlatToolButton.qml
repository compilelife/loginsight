import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Button {
  property var action1: null
  flat: true
  checkable: action1.checkable
  checked: action1.checked
  enabled: action1.enabled
  icon.source: action1.iconSource
  onClicked: action1.trigger(this)
  implicitHeight: 30
  implicitWidth: implicitHeight
  ToolTip {
    text: action1.text
    visible: hovered
    delay: 300
  }
}
