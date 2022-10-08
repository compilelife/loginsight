
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15
import './app.js' as App
import './coredef.js' as CoreDef
import './QuickPromise/promise.js' as Q
import './util.js' as Util

ColumnLayout {
  id: root
  spacing: 10
  property alias pattern: patternBox.text
  property var segs: getSegConfig()
  property var previewLines: []
  property var textCodec: null

  RowLayout {
    TextField {
      id: patternBox
      Layout.fillWidth: true
      selectByMouse: true
    }
    Button {
      text: '预览'
      onClicked: previewSyntax()
    }
    Button {
      text: '管理'
      onClicked: App.showToast('功能开发中……')
    }
  }

  RowLayout {
    width: root.width
    height: 400
    Layout.rightMargin: 5
    SyntaxSegments{
      id: syntaxSegs
      Layout.minimumWidth: 200
      Layout.alignment: Qt.AlignTop
    }
    ListView {
      id: viewRoot
      Layout.fillWidth: true
      height: parent.height
      model: previewLines
      clip: true
      Layout.alignment: Qt.AlignTop
      delegate: LogLine {
        width: viewRoot.width
        model: previewLines[index]
        lineNumWidth: 30
        isViewChecked: true
        session: ({syntaxSegConfig: getSegConfig(), highlights:[], textCodec})
        isFocusLine: viewRoot.currentIndex === index
        onFocusLine: viewRoot.currentIndex = lineIndex
      }
      function refresh() {//不知道为啥直接更新previewLines无效
        model = Util.copyArray(previewLines)
      }
    }
  }

  function init(lines) {
    previewLines = []
    let index = 0
    for (const line of lines) {
      previewLines.push({index: index++, content: line.content})
    }
    viewRoot.refresh()
  }

  function previewSyntax() {
    if (syntaxSegs.model.count === 0) {
      App.showToast('请点击“添加字段”添加与正则表达式捕获相同数量的字段', 'error')
      return
    }

    if (pattern.length === 0) {
      App.showToast('请输入格式语法的正则表达式', 'error')
      return
    }

    const lines = []
    for (let i = 0; i < previewLines.length; i++) {
      lines.push(textCodec.toLogByte(previewLines[i].content))
    }

    App.currentSession.core.sendMessage(CoreDef.CmdTestSyntax,
                                        {
                                          pattern: textCodec.toLogByte(pattern),
                                          lines
                                        })
      .then(function(reply){
        for (let i = 0; i < previewLines.length; i++) {
          previewLines[i].segs = reply.segs[i]
        }
        viewRoot.refresh()
      })
  }

  function getSegConfig() {
    const model = syntaxSegs.model
    const ret = []
    for (let i = 0; i < model.count; i++) {
      ret.push(model.get(i))
    }
    return ret
  }

  function setSegConfig(cfg) {
    syntaxSegs.load(cfg)
  }
}
