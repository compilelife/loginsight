
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtQuick.Controls 2.15 as QC2
import QtQml.Models 2.15
import './app.js' as App
import './coredef.js' as CoreDef
import './QuickPromise/promise.js' as Q

ColumnLayout {
  id: root
  spacing: 10
  property alias pattern: patternBox.text
  property var segs: getSegConfig()
  property alias lines: previewLines
  property var textCodec: null

  RowLayout {
    TextField {
      id: patternBox
      Layout.fillWidth: true
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
      model: previewLines
      Layout.fillWidth: true
      height: parent.height
      clip: true
      Layout.alignment: Qt.AlignTop
      delegate: LogLine {
        width: viewRoot.width
        model: _prepareLineModel(previewLines.get(index))
        lineNumWidth: 30
        isViewChecked: true
        session: ({syntaxSegConfig: getSegConfig(), highlights:[], textCodec})
        isFocusLine: viewRoot.currentIndex === index
        onFocusLine: viewRoot.currentIndex = lineIndex
        function _prepareLineModel(o) {//FIXME: segs直接赋给logline报错：Unable to assign QQmlListModel to QVector<QVariantMap>
          const {content, index} = o
          const segs = []
          for(let i = 0; i < o.segs.count; i++) {
            const {length, offset} = o.segs.get(i)
            segs.push({length, offset})
          }
          return {content, index, segs}
        }
      }
    }
  }

  ListModel {
    id: previewLines
    function init(lines) {
      clear()
      let index = 0
      for (const line of lines) {
        append({index: index++, content: line.content, segs: line.segs})
      }
    }
  }

  function previewSyntax() {
    if (syntaxSegs.model.count === 0) {
      App.showToast('请点击“添加字段”添加与正则表达式捕获相同数量的字段')
      return
    }

    if (pattern.length === 0) {
      App.showToast('请输入格式语法的正则表达式')
      return
    }

    const lines = []
    for (let i = 0; i < previewLines.count; i++) {
      lines.push(textCodec.toLogByte(previewLines.get(i).content))
    }

    App.currentSession.core.sendMessage(CoreDef.CmdTestSyntax,
                                        {
                                          pattern: textCodec.toLogByte(pattern),
                                          lines
                                        })
      .then(function(reply){
        const patched = []
        for (let i = 0; i < previewLines.count; i++) {
          //FIXME: previewLines.setProperty(i, "segs", reply.segs[i])不生效
          const {content, index} = previewLines.get(i)
          const l = {content, index, segs: reply.segs[i]}
          previewLines.set(i, l)
        }
        //FIXME: 界面不刷新，只能用这几行“强制刷新”
        //linux/macos有效，Linux不生效
        viewRoot.model = []
        viewRoot.model = previewLines
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
