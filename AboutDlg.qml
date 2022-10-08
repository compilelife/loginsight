import QtQuick 2.15
import './constants.js' as C
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import './app.js' as App
import QtQuick.Dialogs 1.2 as Dlg

Dlg.Dialog {
  property string newVersion: C.VERSION

  ColumnLayout {
    spacing: 5

    TextEdit {
      textFormat: TextEdit.RichText
      readOnly: true
      selectByMouse: true
      text: `<p>Loginsight致力于打造一款日志分析的利器</p><p>联系方式:compilelife@163.com
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      官网:<a href="${C.WEB_PAGE}">${C.WEB_PAGE}</a></p>
      <p>当前版本：${C.VERSION}</p>`
      onLinkActivated: {
        Qt.openUrlExternally(C.WEB_PAGE)
      }
    }

    Rectangle {
      height: 1
      Layout.alignment: Qt.AlignHCenter
      width: 300
      color: 'grey'
    }

    Label {
      id: downloadHint
      visible: false
      textFormat: TextEdit.RichText
      text: ''
      Layout.alignment: Qt.AlignHCenter
      onLinkActivated: {
        if (link === 'download') {
          Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
        }
        else {
          App.main.updater.open()
        }
        accept()
      }
    }

    Button {
      Layout.alignment: Qt.AlignHCenter
      text: '检查更新'
      onClicked: {
        App.main.updater.checkNewVersion().then(function(hasNewVersion){
          if (hasNewVersion) {
            newVersion = App.main.updater.lastestVersion
            downloadHint.text = `最新版本：${newVersion}, <a href="download">点击下载</a>, <a href="detail"> 了解详情 </a>`
          } else {
            downloadHint.text = '已经是最新版本'
          }
          downloadHint.visible = true
        })
      }
    }
  }


}
