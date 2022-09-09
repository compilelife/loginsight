import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import './coredef.js' as CoreDef
import QtQml.Models 2.15
import QtQml 2.15
import QtQuick.Dialogs 1.3
import './util.js' as Util
import './app.js' as App
import com.cy.TextCodec 1.0

Item {
  id: root
  property string name: ''
  signal coreReady()
  property Core core: Core {
        onReady: {
          core.sendMessage(CoreDef.CmdInitRegister, {mydir: NativeHelper.encodePath(NativeHelper.myDir()), uid: NativeHelper.uniqueId()})
            .then(function() {
              core.serverCmdHandlers[CoreDef.ServerCmdRangeChanged] = handleLogRangeChanged
              coreReady()
            })
        }
    }

    property var meta: ({})

    property var logMap: ({})

    property var timeline: timelineImpl
    property var highlightBar: highlightBarImpl

    property var highlights: []
    property alias syntaxSegConfig: setSyntax.segs

    property var logExclusive : null

  readonly property TextCodec textCodec: TextCodec{}

  property bool followLog: true

  property var openArg: ({})

  onNameChanged: {
    core.tag = name
  }

    Component.onCompleted: {
      logExclusive = Qt.createQmlObject('import QtQuick.Controls 1.4; ExclusiveGroup{}', root, 'logExclusive')
    }

    HighlightBar {
      id: highlightBarImpl
      width: parent.width
      onChanged: {
        highlights = getHighlights()
        invalidate()
      }
      onFilter: {
        root.filter({pattern: keyword})
      }
      onSearch: {
        root.search({pattern: keyword})
      }
    }

    SplitView {
      anchors.top: highlightBarImpl.bottom
      height: parent.height - highlightBarImpl.height
        width: parent.width
        orientation: Qt.Horizontal
        SplitView {
            height: parent.height
            orientation: Qt.Vertical
            SplitView.fillWidth: true

            LogView {
                id: rootLogView
                core:root.core
                width: parent.width
                SplitView.fillHeight: true
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 400
            }
            ColumnLayout {
                id: subLogs
                width: parent.width
                SplitView.preferredHeight: 300
                visible: tabBar.count > 0
                TabBar {
                    id: tabBar
                    contentHeight: 26
                }
                StackLayout {
                    id: holder
                    Layout.fillHeight: true
                    currentIndex: tabBar.currentIndex
                    onCountChanged: {
                      if (count <= 0) {//所有sub log都关闭了的话，就让root log view获得焦点
                        rootLogView.checked = true
                      }
                    }
                }

                function append(id, range, name) {
                    const tabBarButton = Qt.createComponent('qrc:/ClosableTabButton.qml')
                                            .createObject(tabBar, {title: name})
                    const subLog = Qt.createComponent('qrc:/LogView.qml')
                                      .createObject(holder, {core})

                    subLog.initLogModel(id, range)
                    _onLogAdded(id, subLog)

                    tabBarButton.closed.connect(function(){
                        tabBarButton.destroy()
                        _onLogRemoved(subLog.logId)
                        subLog.destroy()
                    })

                  tabBar.currentIndex = tabBar.count - 1
                }
            }
        }
        TimeLine {
          id: timelineImpl
            height: parent.height
            SplitView.preferredWidth: 400
            visible: !empty

           onDoubleClickNode: {
             emphasisLine(line)
           }
        }
    }

  MessageDialog {
    id: errTip
    standardButtons: MessageDialog.Ok
    function display(title, detail) {
      errTip.title = title
      errTip.text = detail
      errTip.visible = true
    }
  }

  Dialog {
    id: setSyntaxDlg
    title: '格式语法'
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    width: 800
    height: 500
    SetSyntax {
      id: setSyntax
      width: parent.width
      textCodec: root.textCodec
    }

    function show() {
      setSyntax.lines.init(rootLogView.getTopLines(30))
      visible = true
    }

    onAccepted: {
      if (setSyntax.pattern.length === 0)
        return
      core.sendMessage(CoreDef.CmdSetLineSegment, {
                                                pattern: textCodec.toLogByte(setSyntax.pattern),
                                                segs: setSyntax.segs,
                                                caseSense: true
                                              })
            .then(function() {
              onSyntaxChanged()
            })
       }
   }

  function showSyntaxDlg() {
    setSyntaxDlg.show()
  }

    function _onLogAdded(logId, logView) {
        logMap[logId]=logView
      logView.session = root
      logView.exclusiveGroup = logExclusive
      logView.checked = true
    }
    function _getLogView(logId) {
        return logMap[logId]
    }
    function _onLogRemoved(logId) {
      core.sendMessage(CoreDef.CmdCloseLog, {logId})
        delete logMap[logId]
    }

    function openFile(path) {
      openArg = {action: 'open', arg: path}
        return core.sendModalMessage(CoreDef.CmdOpenFile,
                                     {path: NativeHelper.encodePath(path)})
            .then(msg=>{
                rootLogView.initLogModel(msg.logId, msg.range)
                _onLogAdded(msg.logId, rootLogView)
            })
    }

    function openProcess(process, cache) {
      openArg = {action: 'openProcess', arg: {process, cache}}
      return core.sendMessage(CoreDef.CmdOpenProcess,
                              {process: NativeHelper.encodePath(process),
                                cache})
        .then(function(msg){
          rootLogView.initLogModel(msg.logId, msg.range)
          _onLogAdded(msg.logId, rootLogView)
        })
    }

    //param: {pattern, caseSense, regex}
    function filter(param) {
      const curLog = currentLogView()
      const tabName = param.pattern || '过滤结果'
      if (param.pattern) {
        param.pattern = textCodec.toLogByte(param.pattern)
      }
      const filterArg = Util.merge({
                                     logId: curLog.logId,
                                     caseSense: true,
                                     regex: false,
                                   }, param)
        core.sendModalMessage(CoreDef.CmdFilter, filterArg)
            .then(msg=>{
                subLogs.append(msg.logId, msg.range, tabName)
            })
    }

    //param: {pattern, caseSense, reverse, regex}
    function search(param, searchPos = null) {
      const curLog = currentLogView()
      const {fromLine,fromChar} = searchPos ? searchPos : curLog.getSearchPos()
      const patternToHint = param.pattern
      if (param.pattern) {
        param.pattern = textCodec.toLogByte(param.pattern)
      }
      const searchArg = Util.merge({
        logId: curLog.logId,
        fromLine,
        fromChar,
        pattern: '',
        caseSense: true,
        reverse: false,
        regex: false
      }, param)

      core.sendModalMessage(CoreDef.CmdSearch, searchArg)
        .then(function(msg){
          if (msg.found) {
            curLog.showSearchResult(msg)
          } else {
            if (searchArg.reverse) {
              App.showToast('查找到顶部，未找到'+patternToHint)
            } else {
              App.showToast('查找到底部，未找到'+patternToHint)
            }
          }
        })
    }

    function handleLogRangeChanged(msg) {
      if (!followLog)
        return

        core.sendMessage(CoreDef.CmdSyncLogs)
            .then(function(msg){
                for (const r of msg.ranges) {
                    const logView = _getLogView(r.logId)
                    if (logView)
                        logView.updateRange(r.range)
                }
            })
    }

    function addToTimeLine(lineModel) {
      timeline.addNode(lineModel.line, lineModel.content)
    }

    function currentLogView() {
      for (const key in logMap) {
        if (logMap[key].checked)
           return logMap[key]
      }
      return rootLogView
    }

    function emphasisLine(line) {
      core.sendMessage(CoreDef.CmdMapLine, {logId: rootLogView.logId, index: line})
        .then(function(msg){
          for (const {logId,index} of msg.lines) {
            logMap[logId].showIntoView(index, {remember: true})
          }
          timeline.highlightNode(line)
        })
    }

    function onSyntaxChanged() {
      for (const key in logMap) {
        logMap[key].onSyntaxChanged()
      }
    }

    function invalidate(reload) {
      for (const key in logMap) {
        logMap[key].invalidate(reload)
      }
    }

    function setAsCurrent() {
      updateActions()
      App.setCurrentView(currentLogView())
    }

    function updateActions() {
      App.actions.followLog.checked = followLog
    }

    function setFollowLog(v) {
      const shouldMoveToBottom = v && !followLog
      followLog = v

      if (shouldMoveToBottom) {
        for (const key in logMap) {
          logMap[key].moveToBottom()
        }
      }
    }

    function onSave() {
      const ret = {}
      ret.openArg = openArg
      //TODO: support save sub logs(consider that some sub log(as other's parent) may already be closed)
      ret.highlights = highlights
      ret.timeline = timeline.onSave()
      ret.syntax = {pattern: setSyntax.pattern, segs: setSyntax.segs}
      ret.rootLogView = rootLogView.onSave()
      return ret
    }

    function onLoad(cfg) {
      highlights = cfg.highlights
      highlightBar.load(cfg.highlights)
      timeline.onLoad(cfg.timeline)
      setSyntax.pattern = cfg.syntax.pattern
      setSyntax.setSegConfig(cfg.syntax.segs)
      if (setSyntax.pattern.length > 0) {
        core.sendMessage(CoreDef.CmdSetLineSegment, {
                                                  pattern: setSyntax.pattern,
                                                  segs: setSyntax.segs,
                                                  caseSense: true
                                                })
      }
      rootLogView.onLoad(cfg.rootLogView)
    }

    function exportLog() {
      const curLog = currentLogView()
//      if (curLog === rootLogView) {
//        App.showToast("不支持导出原始日志，请选择一个过滤窗口来导出")
//        return
//      }

      if (curLog){
        App.main.userSaveFile('选择日志导出路径', ['*'], function(path){
          core.sendModalMessage(CoreDef.CmdExportLog, {
                                  logId: curLog.logId,
                                  path: NativeHelper.encodePath(path)})
            .then(function(){
              App.showToast('导出成功')
            })
        })
      }
    }
}
