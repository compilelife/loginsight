import QtQuick 2.0
import QtWebSockets 1.15
import QtQuick.Controls 1.4
import "./QuickPromise/promise.js" as Q
import com.cy.CoreBoot 1.0
import "./coredef.js" as CoreDef
import QtQuick.Dialogs 1.3
import './util.js' as Util

Item {
  property var pendings: ({})
  property var serverCmdHandlers: ({})
  property int idGen: 0
  property string tag: ''

  signal ready()

  CoreBoot {
    id: boot
    onStateChanged: function (running) {
      console.log(`[${tag}]`,'state changed to ', running)
      if (running) {
        ready()
      }
      else{
        coreErrDlg.showError('引擎异常中断, 请重启')
      }
    }

    onNewLine: _onTextMsg(line)
  }

  Component.onCompleted: {
    boot.startLocal()
  }

  Component.onDestruction: {
    boot.stop()
  }

  MessageDialog {
    id: coreErrDlg
    visible: false
    standardButtons: StandardButton.Ok
    title: '引擎异常'

    property var then: null
    function showError(content) {
      coreErrDlg.text = content
      coreErrDlg.visible = true
    }

    onVisibleChanged: {
      if (!coreErrDlg.visible && then) {
        then()
      }
    }
  }

  //耗时操作超200ms后显示
  Dialog {
    id: longOpDlg
    property string waitId: ''
    property int waitPromiseId: 0
    property string hint: ''
    property bool enabled: false

    title: '执行耗时操作中'
    standardButtons: StandardButton.Cancel
    Column {
      Text {
        text: longOpDlg.hint
      }
      ProgressBar {
        id: progressBar
        minimumValue: 0
        maximumValue: 100
        value: 0
      }
      Timer {
        id: checkProgressTimer
        interval: 200
        repeat: true
        running: longOpDlg.enabled
        onTriggered: {
//          console.log(`[${tag}]`,'timer sent')
          sendMessage(CoreDef.CmdQueryPromise, {pid: longOpDlg.waitPromiseId})
            .then(function(msg){
//              console.log(`[${tag}]`,'timer', longOpDlg.waitId)
              progressBar.value = msg.progress
              longOpDlg.visible = true
            })
        }
      }
    }

    onRejected: {
      sendMessage(CoreDef.CmdCancelPromise)
      enabled = false
    }

    function startWait(id) {
      waitId = id
      enabled = true
    }

    function finishWait() {
      visible = false
      enabled = false
    }
  }

  function sendModalMessage(cmd, extra) {
    const ret = sendMessage(cmd, extra)
    longOpDlg.startWait(`ui-${idGen}`)
    return ret
  }

  function sendMessage(cmd, extra) {
    const packed = Util.merge({
                 "id": `ui-${++idGen}`,
                 "cmd": cmd
               }, extra)

    const ret = Q.promise(function (resolve, reject) {
      pendings[packed.id] = {
        "resolve": resolve,
        "reject": reject
      }
    })

    const msg = JSON.stringify(packed)
    console.log(`[${tag}]`,'send', msg)
    boot.send(msg)

    return ret
  }

  function replyMessage(msg, extra) {
    const packed = {
      "id": msg.id,
      "cmd": CoreDef.CmdReply,
      "state": CoreDef.StateOk,
      origin: msg.cmd
    }
    if (extra) {
      for (const field in extra)
        packed[field] = extra[field]
    }

    const content = JSON.stringify(packed)
    console.log(`[${tag}]`,'reply', content)
    boot.send(content)
  }

  function _findIndexOf(arr, predict) {
    for (var i = 0; i < arr.length; i++) {
      if (predict(arr[i]))
        return i
    }
    return -1
  }

  function _onTextMsg(msg) {
    let msgObj = null
    try {
      msgObj = JSON.parse(msg)
    } catch (e) {
      console.info('stdout', msg)
      return
    }

    const cmd = msgObj.cmd
    if (!(cmd === CoreDef.CmdReply && msgObj.origin === CoreDef.CmdGetLines))
      console.log(`[${tag}]`,'recv', msg)
    if (cmd === CoreDef.CmdReply) {
      if (msgObj.state === CoreDef.StateOk
          || msgObj.state === CoreDef.StateCancel) {
        const pending = pendings[msgObj.id]
        if (pending) {
          pending.resolve(msgObj)
          delete pendings[msgObj.id]
        }
      } else if (msgObj.state === CoreDef.StateFail) {
        const pending = pendings[msgObj.id]
        if (pending) {
          coreErrDlg.then = ()=>{
            pending.reject(msgObj)
            delete pendings[msgObj.id]
          }
        }
        coreErrDlg.showError(msgObj.why)
      }

      if (msgObj.state === CoreDef.StateFuture) {
        if (longOpDlg.enabled && longOpDlg.waitId === msgObj.id) {
          longOpDlg.waitPromiseId = msgObj["pid"]
        }
      } else {
        if (longOpDlg.enabled && longOpDlg.waitId === msgObj.id) {
          longOpDlg.finishWait()
        }
      }
    } else {
      const handler = serverCmdHandlers[cmd]
      if (handler) {
        const handleRet = handler(msgObj)
        replyMessage(msgObj, handleRet)
      }
    }
  }
}
