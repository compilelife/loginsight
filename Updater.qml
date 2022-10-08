import QtQuick 2.0
import './constants.js' as C
import './QuickPromise/promise.js' as Q
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

Dialog {
  property string lastestVersion: C.VERSION
  property string changeLog: ''

  anchors.centerIn: parent

  header: RowLayout {
    Label {
      text: `  新版本 ${lastestVersion}`
    }
    Button {
      text: 'x'
      Layout.alignment: Qt.AlignRight
      onClicked: reject()
    }
  }

  ColumnLayout {
    Label {
      text: changeLog
    }
    Button {
      text: '前往下载'
      Layout.alignment: Qt.AlignHCenter
      onClicked: {
        Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
        accept()
      }
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
