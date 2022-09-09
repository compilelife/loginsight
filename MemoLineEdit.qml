import QtQuick 2.0
import QtQuick.Controls 1.2

ComboBox {
  property alias curText: editText
  signal memoRecall(var data)

  editable: true
  currentIndex: -1
  model: ListModel{}
  textRole: 'text'
  onActivated: {
    const data = model.get(currentIndex)
    if (data) {
      memoRecall(data)
    }
  }

  function addMemo(data) {
    model.qppend(data)
  }
}
