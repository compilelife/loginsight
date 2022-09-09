import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import './gencolor.js' as GenColor

Item {
  height: highlights.count > 0 ? 20 : 0

  signal search(string keyword)
  signal filter(string keyword)
  signal changed()

  function getHighlights() {
    const ret = []
    for (let i = 0; i < highlights.count; i++) {
      const {keyword, color} = highlights.get(i)
      ret.push({keyword, color})
    }
    return ret
  }

  Menu {
    id: barMenu
    MenuItem {
      text: '清空'
      onTriggered: clear()
    }
  }

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.RightButton
    onClicked: {
      barMenu.popup()
    }
  }

  ListModel {
    id: highlights
    onCountChanged: changed()
  }

  ListView {
    id: bar
    width: parent.width
    model: highlights
    orientation: ListView.Horizontal
    spacing: 10
    delegate: MouseArea{
      width: childrenRect.width
      height: childrenRect.height
      acceptedButtons: Qt.AllButtons
      Row {
        spacing: 2
        Rectangle{
          width: 20
          height: 20
          color: model.color
        }
        Text {
          text: model.keyword
        }
      }
      onClicked: {
        bar.currentIndex = index
        contextMenu.popup()
      }
    }
  }

  ColorDialog {
    id: selectColorDialog
    onAccepted: {
      currentItem().color = String(color)
      changed()
    }
  }

  Menu {
    id: contextMenu
    MenuItem{
      text: '删除'
      onTriggered: highlights.remove(bar.currentIndex, 1)
    }
    MenuItem{
      text: '颜色'
      onTriggered: selectColorDialog.visible = true
    }
    MenuSeparator{}
    MenuItem{
      text: '查找'
      onTriggered: search(currentItem().keyword)
    }
    MenuItem{
      text: '过滤'
      onTriggered: filter(currentItem().keyword)
    }
  }

  function add(keyword) {
    highlights.append({keyword, color: String(GenColor.next())})
  }

  function currentItem() {
    return highlights.get(bar.currentIndex)
  }

  function clear() {
    highlights.clear()
  }

  function load(arr) {
    arr.forEach(a=>highlights.append(a))
  }
}
