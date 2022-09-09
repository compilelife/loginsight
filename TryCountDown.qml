import QtQuick 2.0
import './app.js' as App

Text {
  onVisibleChanged: {
    if (visible) {
      const hours = Math.round(App.tryLeftSeconds / 3600)
      text = `试用剩余时间：\n${hours}小时`
    }
  }
  MouseArea {
    anchors.fill: parent
    onClicked: {
      App.main.buyDlg.open()
    }
  }
}
