import QtQuick 2.15
import QtQuick.Window 2.15
import QtWebSockets 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import './app.js' as App
import './QuickPromise/promise.js' as Q
import './coredef.js' as CoreDef
import './constants.js' as C
import QtQuick.Controls 1.4 as QC1
import QtQuick.Controls.Universal 2.12
import QtQml 2.15
import QtQuick.Dialogs 1.2

QC1.ApplicationWindow {
  width: 1000
  height: 700
  id: root

  visible: true
  title: qsTr("LogInsight")

  property alias toast: _toast
  property MainMenuBar mainMenu: MainMenuBar{id: mainMenu}

  property Core core: Core {
    onReady: initRegister()
  }

  Actions{
    id: actions
  }

  menuBar: mainMenu

  RowLayout {
    id: head
    width: parent.width
    FlatToolButton {
      visible: Qt.platform.os !== 'osx'
      action1: actions.tinyMenu
    }
    TabBar {
      id: tabBar
      contentHeight: 32
      Layout.fillWidth: true
      currentIndex: -1
      background: Rectangle{color: 'transparent'}
    }
    MainToolbar {
      id: toolBar
    }
  }

  StackLayout {
    id: sessions
    currentIndex: tabBar.currentIndex
    anchors{
      top: head.bottom
      left: contentItem.left
      right: contentItem.right
      bottom: contentItem.bottom
    }

    onCountChanged: {
      handleCurrentItemChanged()
    }
    onCurrentIndexChanged: {
      if (currentSession()) {
        handleCurrentItemChanged()
      }
    }

    function handleCurrentItemChanged() {
      if (currentIndex >= 0) {
        App.setCurrentSession(currentSession())
        currentSession().setAsCurrent()
      } else {
        App.setCurrentSession(null)
        App.setCurrentView(null)
      }
      actions.updateSessionActions(currentIndex >= 0)
    }
  }

  FileDialog {
    id: openDlg
    title: '选择文件/工程路径'
    folder: shortcuts.home
    selectMultiple: false
    property var handler: null
    onAccepted: {
      const url = openDlg.fileUrl.toString().substring(7) //drop file://
      if (handler)
        handler(url)
    }
    function requestOpenFile(hint, func) {
      handler = func
      title = hint
      selectExisting = true
      selectFolder = false
      nameFilters = ['*']
      open()
    }
    function requestOpenDir(hint, func) {
      handler = func
      title = hint
      selectExisting = true
      selectFolder = true
      open()
    }
    function requestSaveFile(hint, filter, func) {
      handler = func
      title = hint
      selectExisting = false
      selectFolder = false
      nameFilters = filter
      open()
    }
  }

  function userSaveFile(hint, filter, func) {
    openDlg.requestSaveFile(hint, filter, func)
  }

  Settings {
    id: settings
  }

  Toast {
    id: _toast
    visible: false
    anchors {
      horizontalCenter: parent.horizontalCenter
      top: parent.top
      topMargin: 35
    }
  }

  Dialog {
    id: pmDlg
    width: 500
    height: 400
    ProcessManager{
      id: pm
    }
    onAccepted: {
      const args = pm.getUserSelect()
      if (args.process.length === 0) {
        _toast.show('未指定要打开的程序')
        return
      }

      _doOpenProcess(args)
    }
  }

  ErrorDlg {
    id: errDlg
    anchors.centerIn: parent
  }

  OpenMultiFilesDlg{
    id: openMultiFilesDlg
    onOpenFiles: {
      const name = dir.substring(dir.lastIndexOf('/')+1)
      _updateRecents({action: 'openMulti', arg: filelist, name})
      _doOpenMulti(filelist, name)
    }
    function openWith(path) {
      dir = path
      open()
    }
  }

  DropArea {
    anchors.fill: parent
    onDropped: {
      if (drop.hasUrls) {
        const urls = drop.urls.map(function(url) {
          return url.toString().substring(7)
        })
        openUrls(urls, 0)
      }
    }

    function openUrls(urls, index){
      if (index >= urls.length)
        return
      _doOpenFileOrPrj(urls[index])
        .then(function(){openUrls(urls, index+1)})
    }
  }

  Updater {id: _updater}

  Keys.onPressed:  {

  }

  property BuyDlg buyDlg: BuyDlg {}
  property AboutDlg aboutDlg: AboutDlg {}
  property Feedback feedBack: Feedback{}
  property alias updater: _updater

  Component.onCompleted: {
    App.setActions(actions)
    App.setMain(this)
    App.setSettings(settings.settings)

    mainMenu.loadSettings()

    showMaximized()
    actions.updateSessionActions(false)

    pm.initRecords()

    if (App.settings.updater.autocheck)
      updater.checkNewVersion().then(function(hasNewVersion){
        if (hasNewVersion) {
          updater.open()
        }
      })
//    _doOpenFileOrPrj('/tmp/test.log')
//    _doOpenProcess('while true;do echo `date`;sleep 1;done')
  }

  function showError(message, title, handler) {
    errDlg.message = message
    errDlg.title = title
    errDlg.oneShotHandler = handler
    errDlg.open()
  }

  function initRegister() {
    const arg = {
      mydir: NativeHelper.encodePath(NativeHelper.myDir()),
      uid: NativeHelper.uniqueId()
    };
    core.sendMessage(CoreDef.CmdInitRegister, arg)
      .then(function(msg) {
        App.setRegisterInfo(msg.rstate, msg.left)
        _updateActionsOnRegisterState(msg.rstate)

        mainMenu.handleRegisterState(msg.rstate)

        if (msg.rstate === CoreDef.RSTry || msg.rstate === CoreDef.RSTryEnd)
          toolBar.tryCountDown.visible = true
        if (msg.rstate === CoreDef.RSTryEnd) {
          App.showError('试用结束，主要功能将受限或异常\n请尽快购买或使用开源版本')
        }
      })
  }

  function popTinyMenuBar() {
    mainMenu.tinyMenuBar.popup()
  }

  function _updateActionsOnRegisterState(state) {
    if (state === CoreDef.RSOpenSource) {
      for (const action of actions.nonOpenSourceAction) {
        action.text += '[专业版]'
        action.enabled = false
      }
    }
  }

  function _doOpenFileOrPrj(url, name) {
    if (!name)
      name = url.substring(url.lastIndexOf('/')+1)
    if (name.endsWith(".liprj")) {
      return _doLoadProject(url)
    }

    const session = addSession(name)
    session.name = name

    return Q.promise(function(resolve,reject){
      session.coreReady.connect(function () {
            session.openFile(url).then(function(){
              resolve()
            }, function () {
              delSession(session)
              reject()
            })
          })
    })
  }

  function _doOpenMulti(filelist, name) {
    const session = addSession(name)
    session.name = name

    return Q.promise(function(resolve,reject){
      session.coreReady.connect(function () {
            session.openMulti(filelist, name).then(function(){
              resolve()
            }, function () {
              delSession(session)
              reject()
            })
          })
    })
  }

  function openMulti(filelist,name) {
    openDlg.requestOpenDir('选择要打开的文件夹', function(url){
      openMultiFilesDlg.openWith(url)
    })
  }

  function storeSettings() {
    NativeHelper.writeToFile(NativeHelper.settingsPath(), JSON.stringify(App.settings))
  }

  function _isSameOpenAction(a1, a2) {
    return a1.action === a2.action && a1.arg === a2.arg && a1.name === a2.name
  }

  function _updateRecents(openAction) {
    const recents = App.settings.recents
    //控制最大长度
    if (recents.length > 10)
      recents.pop()
    //已存在，则移动最前
    const oldIndex = recents.findIndex(it=>_isSameOpenAction(it, openAction))
    if (oldIndex >= 0) {
      recents.splice(oldIndex, 1)
    }
    recents.unshift(openAction)
    storeSettings()
    mainMenu.updateRecents()
  }

  function openFileOrPrj() {
    openDlg.requestOpenFile('选择要打开的日志文件或工程', function(url){
      _updateRecents({action: 'open', arg: url})
      _doOpenFileOrPrj(url)
    })
  }

  function _doOpenProcess({process, cache}) {
    const session = addSession("process")
    session.name = 'process'//TODO: 这里应该给什么TAB名字呢？
    return Q.promise(function(resolve,reject){
      session.coreReady.connect(function () {
        session.openProcess(process, cache).then(function(){
          resolve()
        }, function () {
          delSession(session)
          reject()
        })
      })
    })
  }

  function openProcess() {
    pmDlg.visible = true
  }

  function openClipboard() {
    const path = NativeHelper.writeClipboardToTemp();
    _doOpenFileOrPrj(path, '粘贴板')
  }

  function currentLogView() {
    return currentSession().currentLogView()
  }

  function currentSession() {
    return sessions.itemAt(sessions.currentIndex)
  }

  function addSession(title) {
    const tabBarButton = Qt.createComponent(
                         'qrc:/ClosableTabButton.qml').createObject(tabBar, {
                                                                      "title": title
                                                                    })
    const session = Qt.createComponent(
                    'qrc:/Session.qml').createObject(sessions)

    tabBarButton.closed.connect(function () {
      delSession(session)
    })

    session.meta.tabBtn = tabBarButton
    tabBar.currentIndex = tabBar.count - 1

    return session
  }

  function delSession(session) {
    session.meta.tabBtn.destroy()
    session.destroy()
  }

  function showSettings() {
    settings.visible = true
  }

  function saveProject() {
    const ret = {}
    ret.sessions = []
    for (let i = 0; i < sessions.count; i++) {
      const session = sessions.itemAt(i)
      ret.sessions.push(session.onSave())
    }

    ret.currentIndex = sessions.currentIndex

    openDlg.requestSaveFile('选择要保存的工程文件路径', ['*.liprj'], function(path){
      if (!path.endsWith('.liprj'))
        path += '.liprj'
      if (NativeHelper.writeToFile(path, JSON.stringify(ret))) {
        App.showToast('工程文件保存成功')
      }else {
        App.showToast('工程文件保存失败！')
      }
    })

  }

  function _doLoadProject(path) {
    const prjContent = NativeHelper.readFile(path)
    if (prjContent.length === 0) {
      toast.show('非法的工程文件')
      return Q.rejected()
    }

    const root = JSON.parse(prjContent)

    return _doLoadSession(root, 0).then(function(){
      tabBar.currentIndex = root.currentIndex
    },function(reason){
      toast.show(reason)
    })
  }

  function replayOpenAction({action,arg,name}) {
    let ret = null
    _updateRecents({action,arg,name})
    if (action === 'open')
      ret = _doOpenFileOrPrj(arg, name)
//    else if (action === 'openProcess') //restore open process has no meanings
//      ret = _doOpenProcess(arg)
    else if (action === 'openMulti')
      ret = _doOpenMulti(arg, name)
    return ret
  }

  function userTextOfOpenAction({action,arg,name}) {
    if (action === 'open')
      return arg
    else if (action === 'openMulti')
      return '...'+arg[0]
    else
      return name
  }

  function _doLoadSession(root, index) {
    if (index >= root.sessions.length)
      return Q.resolved()

    const sessionCfg = root.sessions[index]
    //FIXME: 这里main依赖了sessionCfg的具体实现，需要重构
    let ret = replayOpenAction(sessionCfg.openArg)
    if (ret === null)
      toast.show('未知的会话', action, arg)

    if (ret) {
      return ret.then(function(){
        currentSession().onLoad(sessionCfg)
        return _doLoadSession(root, index+1)
      })
    }

    return Q.rejected('一些会话无法还原')
  }

  function doRegister(orderId) {
    core.sendMessage(CoreDef.CmdDoRegister, {orderId})
      .then(function(msg){
        if (msg.ok) {
          App.showToast('注册成功! 请重启程序。', 'info', -1)
        } else {
          App.showError('注册失败：'+msg.info)
        }
      })
  }
}
