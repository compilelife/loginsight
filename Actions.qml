import QtQuick 2.0
import QtQuick.Controls 1.4
import './app.js' as App
import './coredef.js' as CoreDef

Item {
  property var sessionActions: [
    close,
    filter,
    search,
    goTo,
    clearTimeLine,
    shotTimeLine,
    setSyntax,
    followLog,
    saveProject,
    copyLines,
    exportLog
  ]

  property var nonOpenSourceAction: [
    openProcess,
    openMulti,
    setSyntax,
    copyLines,
    exportLog,
  ]

  function updateSessionActions(hasSession) {
    const enable = hasSession
    for (const action of sessionActions) {
      if (App.registerState === CoreDef.RSOpenSource
          && nonOpenSourceAction.indexOf(action) >= 0) {
        continue
      }

      action.enabled = enable
    }
  }

  property Action open:   Action {
    text: '打开文件/项目'
    shortcut: 'ctrl+o'
    onTriggered: App.main.openFileOrPrj()
  }

  property Action openMulti: Action {
    text: '打开文件夹'
    shortcut: 'ctrl+shift+o'
    onTriggered: App.main.openMulti()
  }

  property Action openProcess: Action {
    text: '打开外部程序'
    onTriggered: App.main.openProcess()
  }

  property Action close:   Action {
    text: '关闭当前标签页'
    shortcut: 'ctrl+w'
    onTriggered: App.main.delSession(App.currentSession)
  }

  property Action filter:   Action {
    text: '过滤'
    shortcut: 'ctrl+d'
    iconSource: 'qrc:/images/filter.png'
    onTriggered: App.currentLogView.filterAction()
  }

  property Action search:   Action {
    text: '查找'
    shortcut: 'ctrl+f'
    iconSource: 'qrc:/images/search.png'
    onTriggered: App.currentLogView.searchAction()
  }

  property Action goTo:  Action {
    text: '跳转'
    iconSource: 'qrc:/images/locate.png'
    shortcut: 'ctrl+g'
    onTriggered: App.currentLogView.gotoAction()
  }

  property Action addToTimeLine: Action {
    text: '添加到时间线'
    shortcut: 'ctrl+t'
    iconSource: 'qrc:/images/addto.png'
    onTriggered: App.currentLogView.addCurLineToTimeLine()
  }

  property Action clearTimeLine:  Action {
    text: '清空时间线'
    iconSource: 'qrc:/images/clear.png'
    onTriggered: App.currentSession.timeline.clear()
  }

  property Action shotTimeLine: Action {
    text: '截图时间线到剪贴板'
    iconSource: 'qrc:/images/screenshot.png'
    onTriggered: App.currentSession.timeline.screenShot()
  }

  property Action goBack: Action {
    text: '后退'
    enabled: false
    iconSource: 'qrc:/images/left.png'
    shortcut: 'ctrl+['
    onTriggered: App.currentLogView.goBack()
  }

  property Action goForward: Action {
    text: '前进'
    enabled: false
    iconSource: 'qrc:/images/right.png'
    shortcut: 'ctrl+]'
    onTriggered: App.currentLogView.goForward()
  }

  property Action setSyntax: Action {
    text: '格式语法'
     onTriggered: App.currentSession.showSyntaxDlg()
  }

  property Action feedback: Action {
    text: '联系作者'
    onTriggered: App.main.feedBack.open()
  }

  property Action settings: Action {
    text: '设置'
    onTriggered: App.main.showSettings()
  }

  property Action followLog: Action {
    text: '跟随模式'
    checkable: true
    checked: true
    shortcut: 'ctrl+m'
    iconSource: 'qrc:/images/pause.png'
    onCheckedChanged: {
      if (checked)
        iconSource = 'qrc:/images/pause.png'
      else
        iconSource = 'qrc:/images/play.png'
      App.currentSession.setFollowLog(checked)
    }
  }

  property Action copyLines: Action {
    text: '复制多行'
    checkable: true
    shortcut: 'ctrl+shift+c'
    iconSource: 'qrc:/images/copy-lines.png'
    property int fromLineIndex: -1
    function chooseLine(line) {
      if (!checked) return
      if (fromLineIndex < 0) {
        fromLineIndex = line
        App.showToast('请选择结束行')
      } else {
        App.currentSession.copyLines(fromLineIndex, line)
        checked = false
      }
    }
    onCheckedChanged: {
      if (checked) {
        fromLineIndex = -1
        App.showToast('将复制多行，请选择开始行')
      }
    }
  }

  property Action openClipboard: Action {
    text: '打开剪贴板文本'
    onTriggered: App.main.openClipboard()
  }

  property Action saveProject: Action {
    text: '保存工程'
    onTriggered: App.main.saveProject()
  }

  property Action about: Action {
    text: '关于'
    onTriggered: App.main.aboutDlg.open()
  }

  property Action register: Action {
    text: '注册'
    onTriggered: App.main.buyDlg.open()
  }

  property Action exportLog: Action {
    text: '导出日志'
    onTriggered: App.currentSession.exportLog()
  }
}
