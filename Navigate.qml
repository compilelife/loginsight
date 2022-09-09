import QtQuick 2.0

Item {
  property var history: []
  property int currentIndex: -1
  readonly property bool canGoBack: currentIndex >= 0 && history.length > 0
  readonly property bool canGoForward: currentIndex < history.length - 1 && history.length > 0

  onCanGoForwardChanged: {
    console.log('nav', canGoForward)
  }

  function _isInOldTime() {
    return canGoForward
  }

  function addPos(pos) {
    if (_isInOldTime()) {
      //drop newer pos from currentIndex
      history = history.slice(0, currentIndex + 1)
    }
    history.push(pos)
    currentIndex = history.length - 1
  }

  function goBack() {
    return history[currentIndex--]
  }

  function goForward() {
    return history[++currentIndex]
  }

  function onSave() {
    return {
      history,
      currentIndex
    }
  }

  function onLoad(cfg) {
    history = cfg.history
    currentIndex = cfg.currentIndex
  }
}
