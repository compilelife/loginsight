import QtQuick 2.0
 import QtQuick.Controls 2.15

Dialog {
  id: root
  width: 350
  height: 150
  focus: true

  standardButtons: Dialog.Ok | Dialog.Cancel
  title: '过滤'
  contentItem: PatternBox {
    id: patternBox
  }

  onAccepted: {
    patternBox.addCurToCompleter()
  }

  function getFilterArgs() {
    return {
      regex: patternBox.isRegex,
      pattern: patternBox.curText,
      caseSense: patternBox.isCaseSense
    }
  }
}
