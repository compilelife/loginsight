.pragma library
.import './util.js' as Util
.import './coredef.js' as CoreDef

//qml里的js不能直接设置全局变量，所以下面提供了各种set方法

let settings = null

function setSettings(v) {
  settings = v
}

let registerState = -1
let tryLeftSeconds = 0

function setRegisterInfo(state, left) {
  registerState = state
  tryLeftSeconds = left
}

function showMsgDlg(title, text) {
  main.msgDlg.text = text
  main.msgDlg.title = title;
  main.msgDlg.open()
}

let actions = null
function setActions(o){actions = o}

let main = null
function setMain(o){main = o}

let currentSession = null
function setCurrentSession(o){currentSession = o}
function isCurrentSession(o) {return currentSession === o}

let currentLogView = null
function setCurrentView(o){currentLogView = o}

function showToast(txt, duration=2000) {
  main.toast.show(txt, duration)
}
