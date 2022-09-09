import QtQuick 2.0
import com.cy.LineHighlighter 1.0
import QtQuick 2.15
import './app.js' as App

//when use as Repeater's child, this could be reused
Item {
  property var model: null
  property int lineNumWidth: 0
  property var session: null
  property bool isViewChecked: false
  property bool isFocusLine: false

  property var cols: []

  id: root
  width: parent.width
  height: model === null ? 0 : loader.height

  signal contextMenu(var model, string select)
  signal focusLine(int lineIndex)
  signal emphasisLine(int line)

  property TextEdit _content: null
  property LineHighlighter _highlighter: null

  //act as vue v-if
  Loader {
    id: loader
  }

  onModelChanged: {
    if (model) {
      loader.sourceComponent = lineImpl
    } else {
      loader.sourceComponent = null
    }
  }

  Component {
    //only visible when model not null
    id: lineImpl
    Row {
      spacing: 8
      Rectangle {
        id: indicator
        width: lineNumWidth + 4
        height: content.height + App.settings.logView.lineSpacing
        color: isViewChecked ? '#49b2f6' : 'grey'
        Text {
          width: parent.width
          horizontalAlignment: Text.AlignLeft
          text: String(model.index + 1)
          wrapMode: Text.NoWrap
          color: isFocusLine ? 'white' : 'black'
        }
      }
      TextEdit {
        id: content
        focus:true
        textFormat: TextEdit.PlainText //highlighter set offset on rich text has strange offset
//        readOnly: true //comment this line to show cursor
        selectByMouse: true
        width: root.width - indicator.width - parent.spacing
        text: model.content
        wrapMode: Text.WrapAnywhere
        font {
          family: App.settings.logView.font.family
          pixelSize: App.settings.logView.font.size
          underline: isFocusLine
        }

        MouseArea{
          anchors.fill: parent
          acceptedButtons: Qt.RightButton
          onClicked: {
            if (mouse.button === Qt.RightButton) {
              content.forceActiveFocus()
              contextMenu(model, content.selectedText)
            }
          }
        }
        LineHighlighter {
          id: highlighter
          segColors: getSegColors()
          highlights: session.highlights
          segs: model.segs || []
          searchResult: model.searchResult || {}
          textCodec: session.textCodec

          function getSegColors() {
            const colors = []
            for (const seg of session.syntaxSegConfig) {
              colors.push(seg.color)
            }
            return colors
          }
        }
        Component.onCompleted: {
          highlighter.setup(content.textDocument)
          _content = content
          _highlighter = highlighter
        }
        onActiveFocusChanged: {
          if (activeFocus)
            focusLine(model.index)
        }
        onTextChanged: {//emulate readonly
          undo()
        }
      }
    }
  }

  function invalidate() {
    if (!_content)
        return
    _content.append('1')
    _content.undo()
  }

  function hasActiveFocus() {
    return _content.activeFocus
  }

  function getSearchPos() {
    const fromChar = session.textCodec.toLogOffset(_content.text, _content.cursorPosition)
    return {fromLine: model.index, fromChar}
  }
}
