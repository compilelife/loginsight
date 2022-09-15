import QtQuick 2.0
 import QtQuick.Controls 2.15
import QtQuick.Controls 1.4 as QC1

Dialog {
  id: root
  width: 450
  height: 150
  focus: true

  standardButtons: Dialog.Ok | Dialog.Cancel
  title: '过滤'
  contentItem: Row{
    spacing: 10
    PatternBox {
      id: patternBox
    }
    QC1.CheckBox {
      id: reverseCheckBox
      text: '反转过滤'
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
