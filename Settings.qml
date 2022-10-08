import QtQuick 2.0
import QtQuick.Dialogs 1.3 as Dlg
import QtQuick.Controls 2.15
import './app.js' as App
import QtQuick.Layouts 1.15
import './util.js' as Util

Dlg.Dialog {
  id:root
  title: '设置'
  width: 300
  standardButtons: Dlg.StandardButton.Apply | Dlg.StandardButton.Cancel
  property var settings: ({//FIXME: 为什么这样的写法没有响应式的效果？
                            logView: {
                              font: {
                                size: 14,
                                family: 'monospace'
                              },
                              lineSpacing: 5
                            },
                            processManager: {
                              records: []
                            },
                            updater: {
                              autocheck: true
                            },
                            recents: [],
                            showMenuBar: true
                          })

  ColumnLayout {
    width: parent.width

    Label {text: '日志文本框'; font.bold: true; font.pixelSize: 16}
    ColumnLayout {
      width: parent.width
      SettingItem {
        Layout.preferredWidth: parent.width
        hint: '字体:'
        Button {
          id: fontBtn
          Layout.alignment: Qt.AlignRight
          text: `${settings.logView.font.family}  ${settings.logView.font.size}`
          onClicked: fontDlg.visible = true
        }
      }

      SettingItem {
        Layout.preferredWidth: parent.width
        hint: '行间距:'
        SpinBox {
          id: spacingSpin
          from: 0
          to: 20
          value: settings.logView.lineSpacing
          Layout.alignment: Qt.AlignRight
          onValueChanged: {
            settings.logView.lineSpacing = value
          }
        }
      }
    }

    Label {text: '更新'; font.bold: true; font.pixelSize: 16}
    CheckBox {
      text: '启动时检查'
      checked: settings.updater.autocheck
      onCheckedChanged: {
        settings.updater.autocheck = checked
      }
    }
  }


  Dlg.FontDialog {
    id: fontDlg
    onVisibleChanged: {
      if (visible) {
        currentFont.family = settings.logView.font.family
        currentFont.pixelSize = settings.logView.font.size
      }
    }

    onAccepted: {
      settings.logView.font.family = font.family
      settings.logView.font.size = font.pixelSize
      fontBtn.text = `${settings.logView.font.family}  ${settings.logView.font.size}`
    }
  }

  Component.onCompleted: {
    const s = NativeHelper.readFile(NativeHelper.settingsPath());
    if (s.length > 0)
      Util.merge(settings, JSON.parse(s))

    fontBtn.text = `${settings.logView.font.family}  ${settings.logView.font.size}`
    spacingSpin.value = settings.logView.lineSpacing
  }

  onApply: {
    App.main.storeSettings()
    App.showToast('设置已保存，重启后生效', 'info', -1)
  }
}
