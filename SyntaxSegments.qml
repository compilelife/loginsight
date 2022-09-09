import QtQuick 2.0
import './coredef.js' as CoreDef
import './gencolor.js' as GenColor
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4 as QC1
import QtQml.Models 2.15
import QtQuick.Dialogs 1.3
import './QuickPromise/promise.js' as Q
import QtQuick.Layouts 1.15

ListView {
  id: root
  spacing: 10
  height: childrenRect.height
  width: 260
  //{color:string, type:int, name: string}
  model: ListModel{}
  delegate: Row {
    spacing: 5
    IconButton {
      size: 32
      source: 'qrc:/images/delete.png'
      onClicked: root.model.remove(index)
    }
    TextField {
      width: 90
      Layout.alignment: Qt.AlignVCenter
      text: name
    }
    Rectangle {
      width: height
      height: typeText.height
      color: model.color
      MouseArea {
        anchors.fill: parent
        onClicked: {
          colorDlg.requestColor()
            .then(function(color){
              model.color = String(color)
            })
        }
      }
    }
    ComboBox {
      id: typeText
      width: 90
      model: CoreDef.SegTypeNames
      currentIndex: type
    }
  }

  footer: Component {
    Item {
      height: addBtn.height + 8
      width: root.width
      Button {
        id: addBtn
        anchors.centerIn: parent
        width: root.width
        text: '添加字段'
        onClicked: {
          model.append({color: String(GenColor.next()), type: CoreDef.SegTypeStr, name: 'field'})
        }
      }
    }
  }

  ColorDialog {
    id: colorDlg
    property var curAction: null
    function requestColor() {
      curAction = Q.promise(function(resolve, reject){
        visible = true
      })

      return curAction
    }
    onAccepted: {
      if (curAction)
        curAction.resolve(color)
    }
    onRejected: {
      if (curAction)
        curAction.reject()
    }
  }

  function load(segs) {
    model.clear()
    for (const seg of segs) {
      model.append(seg)
    }
  }
}
