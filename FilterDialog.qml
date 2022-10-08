import QtQuick 2.0
import QtQuick.Controls 2.15

Dialog {
  id: root
  x: (parent.width - width)/2

  Row{
    spacing: 10
    PatternBox {
      id: patternBox
      onAccept: root.accept()
    }
    CheckBox {
      id: reverseCheckBox
      text: '反转过滤'
    }
    Button {
      text: '过滤'
      onClicked: root.accept()
    }
  }

  onAccepted: {
    patternBox.addCurToCompleter()
  }

  function getFilterArgs() {
    return {
      regex: patternBox.isRegex,
      pattern: patternBox.curText,
      caseSense: patternBox.isCaseSense,
      reverse: reverseCheckBox.checked
    }
  }
}
