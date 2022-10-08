import QtQuick 2.0
import QtQuick.Dialogs 1.2 as Dlg
import QtQuick.Controls 2.15
import './app.js' as App
import QtQuick.Layouts 1.12

Dlg.Dialog {
  id: root
  standardButtons: Dlg.StandardButton.Ok | Dlg.StandardButton.Cancel
  Column {
    spacing: 5
    RowLayout {
      width: 600
      Text {
        text: '订单号：'
      }
      TextField {
        id: orderIdText
        selectByMouse: true
        Layout.fillWidth: true
      }
    }
    Text {
      text: '设备ID：' + NativeHelper.uniqueId()
      color: 'gray'
    }
    Text {
      text: '按所需运行的平台点击下方按钮进入购买页面：'
      onLinkActivated: Qt.openUrlExternally(link)
    }
    RowLayout {
      spacing: 10
      Button{
        icon{
          source: 'qrc:/images/windows.png'
          color: 'transparent'
          width: 48
          height: 48
        }
        flat: true
        onClicked: Qt.openUrlExternally('https://mianbaoduo.com/o/bread/YZibl55q')
      }
      Button{
        icon{
          source: 'qrc:/images/macos.png'
          color: 'transparent'
          width: 48
          height: 48
        }
        flat: true
        onClicked: Qt.openUrlExternally('https://mianbaoduo.com/o/bread/YZibmZpx')
      }
      Button{
        icon{
          source: 'qrc:/images/linux.png'
          color: 'transparent'
          width: 48
          height: 48
        }
        flat: true
        onClicked: Qt.openUrlExternally('https://mianbaoduo.com/o/bread/YZibmZtp')
      }
      Button{
        icon{
          source: 'qrc:/images/all.png'
          color: 'transparent'
          width: 48
          height: 48
        }
        flat: true
        onClicked: Qt.openUrlExternally('https://mianbaoduo.com/o/bread/YZibmZtw')
      }
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
