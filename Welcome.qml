import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import './app.js' as App

//Loginsight
//洞察日志
//启动         帮助
//...
//最近
//...
Rectangle {
  color: 'white'

  ColumnLayout {
    id: title
    anchors {
      left: parent.left
      leftMargin: 200
      top: parent.top
      topMargin: 100
    }

    Label {
      text: 'Loginsight'
      color: '#1a1a1a'
      font.pixelSize: 40
    }
    Label {
      text: '洞察日志'
      color: '#303030'
      font.pixelSize: 25
    }
  }

  GridLayout {
    columns: 2
    rows: 2

    columnSpacing: parent.width/4
    rowSpacing: parent.height/15

    anchors {
      top: title.bottom
      left: title.left
      topMargin: 30
    }

    ColumnLayout {
      Layout.alignment: Qt.AlignTop
      Label {
        id: startText
        text: '启动'
        color: '#808080'
        font.pixelSize: 20
      }
      WelcomeStartButton{action: App.actions.open}
      WelcomeStartButton{action: App.actions.openMulti}
      WelcomeStartButton{action: App.actions.openProcess}
      WelcomeStartButton{action: App.actions.openClipboard}
    }

    ColumnLayout {
      Layout.alignment: Qt.AlignTop
      Layout.rowSpan: 2
      Label {
        text: '最近'
        color: '#808080'
        font.pixelSize: 20
      }
      Repeater {
        model: App.main.mainMenu.getRecentActions()
        WelcomeStartButton {action: modelData}
      }
    }

    ColumnLayout {
      Layout.alignment: Qt.AlignTop
      Label {
        text: '帮助'
        color: '#808080'
        font.pixelSize: 20
      }
      Label {
        text: '<a href="http://www.loginsight.top/manual/">使用文档</a>'
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: 'blue'
      }
      WelcomeStartButton {action: App.actions.help; prompt: '快捷键'}
      WelcomeStartButton {action: App.actions.about}
    }
  }
}
