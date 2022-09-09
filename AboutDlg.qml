import QtQuick 2.15
import QtQuick.Dialogs 1.3
import './constants.js' as C
import QtQuick.Controls 1.4
import './app.js' as App

Dialog {
  property string newVersion: C.VERSION

  Column {
    TextEdit {
      textFormat: TextEdit.RichText
      text: `<p>LogInsight致力于打造一款日志分析的利器</p><p>联系方式:compilelife@163.com</p>
      官网:<a href="${C.WEB_PAGE}">${C.WEB_PAGE}</a>
      <p>当前版本：${C.VERSION}</p>`
      onLinkActivated: {
        Qt.openUrlExternally(C.WEB_PAGE)
      }
    }

    TextEdit {
      id: downloadHint
      visible: false
      textFormat: TextEdit.RichText
      text: ''
      onLinkActivated: {
        Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
      }
    }

    Button {
      text: '检查更新'
      onClicked: {
        App.main.updater.checkNewVersion().then(function(hasNewVersion){
          if (hasNewVersion) {
            newVersion = App.main.updater.lastestVersion
            downloadHint.text = `最新版本：${newVersion}, <a href="#">点击下载</a>`
          } else {
            downloadHint.text = '已经是最新版本'
          }
          downloadHint.visible = true
        })
      }
    }
  }


}
