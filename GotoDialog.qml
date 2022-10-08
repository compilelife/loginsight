import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

Dialog {
  property var range: ({begin: 0, end: 0})
  property int index: 0

  x: (parent.width - width)/2

   ColumnLayout {
     spacing: 10
      TextField {
        id: lineField
        implicitWidth: 400
        selectByMouse: true
        onAccepted: commit()
      }
      Label {
        text: `请输入要跳转到的行号，介于${range.begin+1}到${range.end+1}之间`
      }
    }

   onVisibleChanged: {
     if (visible)
       lineField.forceActiveFocus()
   }

   function commit() {
     index = parseInt(lineField.text) - 1
     if (index < range.begin || index > range.end)
        return
     accept()
   }

  function setIndex(v) {
    lineField.text = `${v + 1}`
    lineField.selectAll()
    index = v
  }
}
