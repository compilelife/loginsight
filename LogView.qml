import QtQuick 2.15
import QtQuick.Controls 2.15
import "./coredef.js" as CoreDef
import "./QuickPromise/promise.js" as Q
import "./app.js" as App
import './util.js' as Util
import QtQml 2.15

Rectangle {
  id: root

  property Navigate navigate: Navigate{}

  property bool checked: false
  property var exclusiveGroup: null
  property int curIndex: -1
  property var core: null
  property int logId: 0
  property var logModel: ({
                            "count": 0,
                            "range": {
                              "begin": 0,
                              "end": 0
                            },
                            "dataAt": function () {
                              return null
                            }
                          })

  property var session: null

  property int curFocusIndex: 0

  //for continue search
  property var _lastSearchPos: ({fromLine: 0, fromChar: 0, len: 0})

  onExclusiveGroupChanged: {
    if (exclusiveGroup)
      exclusiveGroup.bindCheckable(root)
  }

  onCheckedChanged: {
    if (checked) {
      App.setCurrentView(this)
      App.actions.goBack.enabled = navigate.canGoBack
      App.actions.goForward.enabled = navigate.canGoForward
    }
  }

  Connections {
    enabled: checked
    target: navigate
    function onCanGoBackChanged(){
      if (!App.isCurrentSession(session))
        return
      App.actions.goBack.enabled = navigate.canGoBack
    }
    function onCanGoForwardChanged() {
      console.log('nav1', navigate.canGoForward)
      if (!App.isCurrentSession(session))
        return
      App.actions.goForward.enabled = navigate.canGoForward
    }
  }

  LogViewContextMenu {
    id: menu
    session: root.session
    logview: parent
  }

  Text {
    id: contentMeasure
    visible: false
    width: parent.width - 8 - indicatorMeasure.width
    wrapMode: Text.WrapAnywhere
    font {
      family: App.settings.logView.font.family
      pixelSize: App.settings.logView.font.size
    }
  }

  TextMetrics {
    id: indicatorMeasure
    text: '' + logModel.range.end
  }



  MouseArea {
    id: list
    anchors.fill: parent
    enabled: false
    Column {
      id: contentHolder
      height: list.height
      clip: true
      Repeater {
        id: content
        //max line count correspond to list.height, may or may not
        model: Math.round(list.height / (indicatorMeasure.height + App.settings.logView.lineSpacing))
        LogLine {
          width: list.width
          model: logModel.dataAt(curIndex + index)
          lineNumWidth: indicatorMeasure.width
          session: root.session
          isViewChecked: root.checked
          isFocusLine: curFocusIndex === (curIndex + index)
          onContextMenu: {
            menu.selectText = select
            menu.lineModel = model
            menu.popup()
          }
          onFocusLine: {
            root.checked = true
            curFocusIndex = lineIndex
          }
        }
      }
    }

    Timer {
      id: followScorllBarTimer
      running: false
      repeat: true
      interval: 200
      onTriggered: {
        show(_positionToLineIndex(vbar.position))
      }
    }

    ScrollBar {
      id: vbar
      visible: false
      hoverEnabled: true
      active: hovered || pressed
      orientation: Qt.Vertical
      position: _lineIndexToPosition(curIndex)
      size: 0.05
      stepSize: 1 / logModel.count
      policy: ScrollBar.AsNeeded
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      onPressedChanged: {
        if (vbar.pressed) {
          root.checked = true
          //user begin scrolling vbar
          followScorllBarTimer.running = true
        } else {
          //user end scrolling vbar
          followScorllBarTimer.running = false
          show(_positionToLineIndex(position))
        }
      }
    }

    function _findShownEndIndex() {
      const maxAvail = content.model

      let endIndex = curIndex
      let height = 0
      for (var i = 0; i < maxAvail; i++) {
        height += content.itemAt(i).height

        if (height > list.height)
          break
        endIndex++
      }

      return endIndex
    }

    onWheel: function (ev) {
      root.checked = true
      const indexDelta = -ev.angleDelta.y / 120
      const {beginLine, endLine} = logModel.range

        let index = curIndex + indexDelta
        if (index > endLine)
          index = endLine
        else if (index < beginLine)
          index = beginLine

        show(index)
      }
    }

  SearchBar {
    id: searchBar
    visible: false
    anchors.top: parent.top
    anchors.right: parent.right
    onSearch: {
      let searchPos = _lastSearchPos
      if (isContinue && _lastSearchPos) {
        if (!reverse)
          searchPos.fromChar += searchPos.len
      }

      session.search({pattern: keyword, caseSense: isCaseSense, regex: isRegex, reverse}, searchPos)
    }
    onVisibleChanged: {
      _lastSearchPos = null
      if (!visible) {
        clearSearchResult()
      }
    }
  }

  FilterDialog {
    id: filterDialog
    anchors.centerIn: parent
    visible: false
    onAccepted: {
      session.filter(getFilterArgs())
    }
  }

  GotoDialog {
    id: gotoDialog
    visible: false
    range: logModel.range
    onAccepted: {
      show(index, {placeAt: 'middle', remember: true})
        .then(function(){
          curFocusIndex = index
        })
    }
  }

  Keys.onPressed: {
    if (event.key === Qt.Key_PageDown) {
      const index = getVisibleLastLineIndex()
      show(index, {placeAt:'top'})
    } else if (event.key === Qt.Key_PageUp) {
      const index = getVisibleFirstLineIndex()
      show(index, {placeAt:'bottom'})
    }
  }

  function getVisibleFirstLineIndex() {
    return curIndex
  }

  function getVisibleLastLineIndex() {
    let height = 0
    for (let i = 0; i < content.model; i++) {
      height += content.itemAt(i).height
      if (height > contentHolder.height) {
        return Math.max(curIndex, curIndex + i - 1)
      } else if (height === contentHolder.height) {
        return curIndex + i
      }
    }
    return curIndex + content.model - 1
  }

    //因为vbar的滚动范围是0-1.0，而log的index范围是range.begin-range.end，直觉是建立0-1.0与range.begin-range.end间的映射
    //但并不是，实际上滚动条滚动到最低的时候，对应的日志最后一行减去可视区行数
    //不过计算最后可视区的行数有点麻烦，偷懒的方式是映射[0, 1-vbar.size] 到 [range.begin, range.end]
    //也就是下面两个函数做的事情了
    function _positionToLineIndex(position) {
      return Math.floor(
            position / (1 - vbar.size) * (logModel.count - 1)) + logModel.range.begin
    }

    function _lineIndexToPosition(index) {
      return (index - logModel.range.begin) / (logModel.count - 1) * (1 - vbar.size)
    }



  function initLogModel(id, range) {
    console.log('init log model', id, range.begin, range.end)
    logId = id
    logModel = {
      "logId": logId,
      "range": range,
      "count": range.end - range.begin + 1,
      "cache": [],
      "inCache": function (begin, end = begin) {
        const cache = logModel.cache
        if (cache.length === 0)
          return false

        return cache[0].index <= begin && end <= cache[cache.length - 1].index
      },
      "dataAt": function (i) {
        const cache = logModel.cache
        return logModel.inCache(i, i) ? cache[i - cache[0].index] : null
      }
    }

    show(range.begin)
    vbar.visible = true
    list.enabled = true
  }

  function updateRange(r) {
    logModel.range = r
    logModel.count = r.end - r.begin + 1
    indicatorMeasure.text = r.end

    show(r.end, {placeAt:'bottom', remember: false})
  }

  function _limitRange(r) {
    if (r.begin < logModel.range.begin)
      r.begin = logModel.range.begin
    if (r.end > logModel.range.end)
      r.end = logModel.range.end
    return r
  }

  function _getShowRange(index, placeAt) {
    if (placeAt === 'bottom') {
      const prefer = _limitRange({
        begin: index - (content.model - 1),
        end: index
      })

      if (logModel.inCache(prefer.begin, prefer.end)) {
        let height = 0
        for (let i = prefer.end; i >= prefer.begin; i--) {
          contentMeasure.text = logModel.dataAt(i).content
          height += contentMeasure.height + App.settings.logView.lineSpacing
          if (height > contentHolder.height) {
            prefer.begin = Math.min(i + 1, prefer.end)
            break
          } else if (height === contentHolder.height) {
            prefer.begin = i
            break
          }
        }
        //if not return in for loop, it means all lines' height are not enough to fill contentHolder
        //no additon op should take, just show
        return prefer
      } else {
        //if not in range,let show() function do loading, then call me again
        //next call will run into the upper branch
        return prefer
      }
    } else if (placeAt === 'middle') {
      const prefer = {
        "begin": index - 3,
        "end": index - 3 + content.model - 1
      }
      return _limitRange(prefer)
    } else {
      const prefer = {
        "begin": index,
        "end": index + content.model - 1
      }
      return _limitRange(prefer)
    }
  }

  function _getCacheRange(index) {
    const prefer = {
      "begin": index - 50,
      "end": index + 50
    }
    return _limitRange(prefer)
  }

  function onSyntaxChanged() {
    logModel.cache = [] //drop cache
    show(curIndex)
  }

  function invalidate(reload) {
    if (reload) {
      logModel.cache = []
      show(curIndex)
    } else {
      forceRefresh(curIndex)
    }
  }

  function forceRefresh(index) {
    for (let i = 0; i < content.model; i++) {
      const item = content.itemAt(i)
      item.model = logModel.dataAt(i + index)
      item.invalidate()
    }
  }

  function _show(index) {
    curIndex = index
    forceRefresh(index)
  }

  //async
  function show(index, param) {
    const {placeAt, remember} = Util.merge({placeAt: 'top', remember: false}, param)

     if (logModel.range.end < logModel.range.begin)
       return Q.resolved()

     if (remember)
       navigate.addPos({index, placeAt})

    const {begin, end} = _getShowRange(index, placeAt)
    if (logModel.inCache(begin, end)) {
      _show(begin)
      return Q.resolved()
    }

    const cacheRange = _getCacheRange(index)
    return core.sendMessage(CoreDef.CmdGetLines, {
                       "logId": logModel.logId,
                       "range": cacheRange
                     })
      .then(function(msg){
         logModel.cache = msg.lines
        logModel.cache.forEach(l=>l.content = session.textCodec.toVisualByte(l.content))

         if (placeAt === 'bottom') {
           _show(_getShowRange(index, placeAt).begin)
         } else {
           _show(begin)
         }
      })
    }

  function _isInView(index) {
    if (!logModel.inCache(index))
        return false

    const nth = index - curIndex
    if (nth < 0 || nth >= content.model)
        return false

    //calculate real height
    let height = 0
    const maxHeight = list.height
    for (let i = 0; i <= nth; i++) {
      height += content.itemAt(i).height
    }

    return height <= maxHeight
  }

  function showIntoView(index, param) {
    param = Util.merge({placeAt: 'middle', remember: false}, param)

    if (_isInView(index)) {
      curFocusIndex = index
      if (param.remember) {
        navigate.addPos({index: curFocusIndex, placeAt: param.placeAt})
      }

      return Q.resolved()
    } else {
      return show(index, param)
        .then({curFocusIndex = index})
    }
  }

  function _getLogLine(index) {
    const indexInView = index - curIndex
    const line = content.itemAt(indexInView)
    return line
  }

  function getSearchPos() {
    const line = _getLogLine(curFocusIndex)
    return line ? line.getSearchPos() : {fromLine: curFocusIndex, fromChar: 0}
  }

  function showSearchResult({index, offset, len}) {
    _lastSearchPos = {fromLine: index, fromChar: offset, len}

    showIntoView(index, {remember: true})
      .then(function(){
        for (const cacheLine of logModel.cache) {
          if (cacheLine.index === index) {
            cacheLine.searchResult = {offset, len}
          } else {
            cacheLine.searchResult = null
          }
        }
        forceRefresh(curIndex)
      })
  }

  function clearSearchResult() {
    for (const cacheLine of logModel.cache) {
      cacheLine.searchResult = null
    }
    forceRefresh(curIndex)
  }

  function filterAction() {
    filterDialog.visible = true
  }

  function searchAction() {
    searchBar.visible = true
  }

  function gotoAction() {
    gotoDialog.setIndex(curFocusIndex)
    gotoDialog.visible = true
  }

  function goForward() {
    if (!navigate.canGoForward)
        return

    let pos = navigate.goForward()
    while (pos && pos.index === curFocusIndex) {
      pos = navigate.goForward()
    }
    if (!pos)
      return

    const {index, placeAt} = pos
    show(index, {placeAt, remember:false})
    curFocusIndex = index
  }

  function goBack() {
    if (!navigate.canGoBack)
      return

    let pos = navigate.goBack()
    while (pos && pos.index === curFocusIndex) {
      pos = navigate.goBack()
    }
    if (!pos)
      return

    const {index, placeAt} = pos
    show(index, {placeAt, remember:false})
    curFocusIndex = index
  }

  function getTopLines(n) {
    const startIndexInCache = curIndex - logModel.cache[0].index
    return logModel.cache.slice(startIndexInCache, startIndexInCache+n)
  }

  function moveToBottom() {
    session.core.sendMessage(CoreDef.CmdGetRange, {logId})
      .then(function(reply){
        logModel.range = reply.range
        show(logModel.range.end, {placeAt: 'bottom'})
      })
  }

  function onSave() {
    return {
      checked,
      curIndex,
      curFocusIndex,
      navigate: navigate.onSave()
    }
  }

  function onLoad(cfg) {
    checked = cfg.checked
    curFocusIndex = cfg.curFocusIndex
    show(cfg.curIndex)
    navigate.onLoad(cfg.navigate)
  }
}
