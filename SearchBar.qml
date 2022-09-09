import QtQuick 2.0
import QtQuick.Controls 1.2
import QtGraphicalEffects 1.15

Item {
  id: root
  width: content.width + 20
  height: content.height + 8

  property alias isRegex: patternBox.isRegex
  property alias isCaseSense: patternBox.isCaseSense

  signal search(string keyword, bool reverse, bool isContinue)

  property string _lastKeyword: ''
  property bool _lastReverse: false

  onVisibleChanged: {
    _lastKeyword = ''
    _lastReverse = false
  }

  Keys.onEscapePressed: {
    visible = false
  }

  Rectangle {
    id: background
    anchors.fill: root
    color: 'white'
  }

  DropShadow {
    source: background
    anchors.fill: background
    verticalOffset: 2
    horizontalOffset: 2
    radius: 3
    color: '#80000000'
  }

  Row {
    id: content
    spacing: 10
    width: childrenRect.width
    height: childrenRect.height
    anchors.centerIn: parent
    PatternBox {
      id: patternBox
      onAccept: requestSearch()
    }

    IconButton {
      size: parent.height
      source: "qrc:/images/left.png"
      onClicked: requestSearch(true)
    }

    IconButton {
      size: parent.height
      source: "qrc:/images/right.png"
      onClicked: requestSearch()
    }

    IconButton {
      id: closeBtn
      size: parent.height
      source: "qrc:/images/close.png"
      onClicked: root.visible = false
    }
  }

  function _updateIsContinue(keyword, reverse) {
    const isContinue = (_lastKeyword === keyword) && (_lastReverse === reverse)
    _lastKeyword = keyword
    _lastReverse = reverse
    return isContinue
  }

  function requestSearch(reverse = false) {
    const keyword = patternBox.curText
    search(keyword, reverse, _updateIsContinue(keyword, reverse))
    patternBox.addCurToCompleter()
  }
}
