import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Dialog {
  property var range: ({begin: 0, end: 0})
  property int index: valueBox.value - 1

  width: 400
  height: 100

  standardButtons: StandardButton.Ok | StandardButton.Cancel
  title: '跳转到……'

  Column {
      spacing: 10
      width: parent.width
      Text {
        text: `请输入行号： ${range.begin+1} - ${range.end+1}`
      }
      SpinBox {
        id: valueBox
        width: parent.width
        property int realValue: value - 1
        minimumValue: range.begin + 1
        maximumValue: range.end + 1
        onEditingFinished: accepted()
      }
    }

  function setIndex(index) {
    valueBox.value = index + 1
  }
}
