import QtQuick 2.0
import QtQuick.Controls 1.4

Menu {
  property var session: null
  property var logview: null
  property var lineModel: null
  property string selectText: ''

  readonly property bool hasSeletion: selectText.length > 0

  MenuItem {
    text: '添加到时间线'
    onTriggered: session.addToTimeLine(lineModel)
  }
  MenuItem {
    text: '追踪该行'
    onTriggered: session.emphasisLine(lineModel.line)
  }

  MenuItem {
    text: '清除高亮'
    onTriggered: session.highlightBar.clear()
  }
  MenuSeparator{}
  MenuItem {
    visible: hasSeletion
    text: '高亮'
    onTriggered: session.highlightBar.add(selectText)
  }
  MenuItem {
    visible: hasSeletion
    text: '过滤'
    onTriggered: session.filter({pattern: selectText})
  }
  MenuSeparator{}
  MenuItem {
    visible: hasSeletion
    text: '拷贝'
    onTriggered: NativeHelper.clipboardSetText(selectText)
  }
}
