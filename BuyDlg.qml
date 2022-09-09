import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import './app.js' as App
import QtQuick.Layouts 1.12

Dialog {
  id: root
  standardButtons: StandardButton.Ok | StandardButton.Cancel
  Column {
    spacing: 5
    RowLayout {
      width: 600
      Text {
        text: '订单号：'
      }
      TextField {
        id: orderIdText
        Layout.fillWidth: true
      }
    }
    Text {
      text: '设备ID：' + NativeHelper.uniqueId()
    }
    Text {
      text: '<a href="https://mianbaoduo.com/o/bread/YZibl55q">windows购买地址</a> <a href="https://mianbaoduo.com/o/bread/YZibmZpx">mac购买地址</a> <a href="https://mianbaoduo.com/o/bread/YZibmZtp">linux购买地址</a> <a href="https://mianbaoduo.com/o/bread/YZibmZtw">全平台购买地址</a>'
      onLinkActivated: Qt.openUrlExternally(link)
    }
  }
  onAccepted: {
    if (orderIdText.length === 0) {
      App.showToast('订单号不能为空')
    } else {
      App.main.doRegister(orderIdText.text)
    }
  }
}
