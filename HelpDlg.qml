import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import './constants.js' as C
import './app.js' as App

Dialog {
  ColumnLayout {
    Label {
      text: `帮助文档: <a href="${C.WEB_MANUAL}">${C.WEB_MANUAL}</a>`
      onLinkActivated: Qt.openUrlExternally(link)
    }

    Label { text: '快捷键：' }

    ListView {
      id: shortcutList
      height: Math.min(model.count * 30, 500)
      width: 300
      model: _getShortcuts()
      clip: true
      delegate: Rectangle {
        color: 'white'
        width: parent.width
        height: 30

        RowLayout {
          width: parent.width
          Label {text: name; Layout.alignment: Qt.AlignLeft}
          Label {text: key; Layout.alignment: Qt.AlignRight}
        }
      }
    }
  }

  function _getShortcuts() {
    const actions = App.actions
    const shortcuts = Qt.createQmlObject("import QtQml.Models 2.15; ListModel{}", shortcutList, 'shortcuts')

    for (var key in actions) {
      if (actions.hasOwnProperty(key)) {
        const action = actions[key]
        if (action && action['shortcut']) {
          shortcuts.append({name: action['text'], key: action['shortcut']})
        }
      }
    }

    //TODO: 保持与LogView _handleLineKeyDown一致
    shortcuts.append({name: '添加当前行到时间线', key: 'A'})
    shortcuts.append({name: '搜索', key: 'S'})
    shortcuts.append({name: '跳转到行', key: 'D'})
    shortcuts.append({name: '过滤', key: 'F'})
    shortcuts.append({name: '追踪该行', key: 'T'})
    shortcuts.append({name: '高亮被选词', key: 'E'})

    return shortcuts
  }
}
