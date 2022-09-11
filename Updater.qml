import QtQuick 2.0
import './constants.js' as C
import './QuickPromise/promise.js' as Q
import QtQuick.Dialogs 1.3

Item {
  property string lastestVersion: C.VERSION
  property string changeLog: ''

  property Dialog goDownloadDlg : Dialog{
    standardButtons: StandardButton.Ok |StandardButton.Cancel
    Text {
      text: `# 新版本 ${lastestVersion}\n${changeLog}\n[点击下载](${C.WEB_DOWNLOAD_URL})`
      textFormat: Text.MarkdownText
      onLinkActivated: {
        Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
      }
    }
    onAccepted: {
      Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
    }
  }

  function checkNewVersion() {
    return Q.promise(function(resolve){
      const xhr = new XMLHttpRequest
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          const info = JSON.parse(xhr.responseText)
          lastestVersion = info.version
          changeLog = info.change

          console.info(`current version: ${C.VERSION}, remote version: ${info.version}`)
          if (C.isVersionBigger(info.version, C.VERSION)) {
            resolve(true)
          }else {
            resolve(false)
          }
        }
      }
      xhr.open('GET', C.WEB_GET_VERSION_URL)
      xhr.send(null)
    })
  }
}
