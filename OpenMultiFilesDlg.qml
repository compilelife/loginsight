import QtQuick 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4

Dialog {
  property string dir: ''
  property var filenamesByTime: []

  signal openFiles(var filelist)

  width: 500
  height: 500

  standardButtons: StandardButton.Open | StandardButton.Cancel

  onDirChanged: {
    filenamesByTime = NativeHelper.listFiles(dir, '*')
    resort()
  }

  onAccepted: {
    openFiles(previewList.model.map(filename=>dir+'/'+filename))
  }

  ColumnLayout {
    width: parent.width
    height: 400
    RowLayout {
      Text{text: `正在打开目录：${dir}`; Layout.fillWidth: true}
      TextField {
        id: suffixField
        text: ''
        placeholderText: '日志后缀'
        width: 50
        onTextChanged: {
          resort()
        }
      }
    }
    RowLayout {
      width: parent.width
      ComboBox {
        id: typeCombo
        model: ['时间序','智能','数字序','字母序']
        onCurrentIndexChanged: {
          keywordField.enabled = currentIndex >= 2
          resort()
        }
      }
      TextField {
        id: keywordField
        placeholderText: '输入正则表达式'
        Layout.fillWidth: true
        onTextChanged: resort()
      }
    }
    Text {
      property var hints: [
        '按文件的创建日期排序',
        '自动查找文件名中的数字，然后按数字序排列；如果没有数字，就整个文件名按字母序排列',
        '通过正则表达式的分组指出文件名中用于排序的数字，如文件名 xxx_1.log/xxx_2.log，可以用正则表达式\w+_(\d+).log',
        '通过正则表达式的分组指出文件名中用于排序的字母，如文件名 userlog-a.log/userlog-b.log，可以用正则表达式userlog-(\w+).log'
      ]
      Layout.preferredWidth: parent.width
      wrapMode: Text.Wrap
      color: 'gray'
      text: hints[typeCombo.currentIndex]
    }
    ListView {
      id: previewList
      Layout.fillHeight: true
      Layout.preferredWidth: parent.width
      clip: true
      delegate: Text {
        text: modelData
      }
      spacing: 5
    }
  }

  function _getNum(s) {
    const match = /[^0-9]*([0-9]+)[^0-9]*/.exec(s)
    if (!match)
      return -1

    const sub = match[1]
    if (sub)
      return parseInt(sub)
    return -1
  }

  function resort() {
    let filenames = []
    if (suffixField.text.length === 0)
      filenames = [].concat(filenamesByTime)
    else
      filenames = filenamesByTime.filter(name=>name.endsWith(suffixField.text))

    const type = typeCombo.currentIndex
    if (type === 0){}
    else if (type === 1) {
      //尝试从文件名里提取数字，如果提取失败，则按整个文件名的字母序
      //如果提取成功，则每个文件名都提取数字，按数字序比较
      if (filenames.length === 0)
        return
      const cmpNum = _getNum(filenames[0]) >= 0
      filenames.sort(function(f1,f2){
        if (!cmpNum)
          return f1 < f2 ? -1: 1
        return _getNum(f1) - _getNum(f2)
      })
    }
    else if (type === 2) {
      if (keywordField.text.length > 0) {
        const r = new RegExp(keywordField.text)
        filenames.sort(function(f1,f2){
          const s1 = r.exec(f1)[1]
          const s2 = r.exec(f2)[1]
          return parseInt(s1) - parseInt(s2)
        })
      }
    }
    else if (type === 3) {
      if (keywordField.text.length > 0) {
        const r = new RegExp(keywordField.text)
        filenames.sort(function(f1,f2){
          const s1 = r.exec(f1)[1]
          const s2 = r.exec(f2)[1]
          return s1 < s2 ? -1 : 1
        })
      }
    }

    previewList.model = filenames
    previewList.forceLayout()
  }
}
