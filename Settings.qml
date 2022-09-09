import QtQuick 2.0
import QtQuick.Dialogs 1.3 as Dialogs
import QtQuick.Controls 2.15
import './app.js' as App
import QtQuick.Layouts 1.15
import './util.js' as Util

Dialogs.Dialog {
  id:root
  title: '设置'
  width: 300
  standardButtons: Dialogs.StandardButton.Apply | Dialogs.StandardButton.Cancel
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
                            }
                          })

  Column {
    GroupBox {//logview
      title: '日志文本框'
      width: root.width
      ColumnLayout {
        SettingItem {
          hint: '字体:'
          Button {
            id: fontBtn
            Layout.alignment: Qt.AlignRight
            text: `${settings.logView.font.family}  ${settings.logView.font.size}`
            onClicked: fontDlg.visible = true
          }
        }

        SettingItem {
          hint: '行间距:'
          SpinBox {
            id: spacingSpin
            from: 0
            to: 20
            value: settings.logView.lineSpacing
            onValueChanged: {
              settings.logView.lineSpacing = value
            }
          }
        }
      }
    }

    GroupBox {
      title: '更新'
      width: root.width
      CheckBox {
        text: '启动时检查'
        checked: settings.updater.autocheck
        onCheckedChanged: {
          settings.updater.autocheck = checked
        }
      }
    }
  }



  Dialogs.FontDialog {
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

  Dialogs.MessageDialog {
    id: confirmRelauch
    title: '保存重构'
    text: '设置已保存，重启后生效；点击重启（目前有bug，需要用户手动重启）'
    onAccepted: {
      NativeHelper.relaunch()
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
    NativeHelper.writeToFile(NativeHelper.settingsPath(), JSON.stringify(settings))
    confirmRelauch.open()
  }
}
