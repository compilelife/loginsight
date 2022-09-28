import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
  property alias tryCountDown: _tryCountDown
  FlatToolButton {
    action1: actions.followLog
  }
  FlatToolButton {
    action1: actions.copyLines
  }
//  ToolSeparator {} //会把RowLayout撑高
  FlatToolButton {
    action1: actions.search
  }
  FlatToolButton {
    action1: actions.filter
  }
  FlatToolButton {
    action1: actions.goTo
  }
  FlatToolButton {
    action1: actions.goBack
  }
  FlatToolButton {
    action1: actions.goForward
  }
//  ToolSeparator {}
  FlatToolButton {
    action1: actions.addToTimeLine
  }
  FlatToolButton {
    action1: actions.clearTimeLine
  }
  FlatToolButton {
    action1: actions.shotTimeLine
  }
//  ToolSeparator {}
  TryCountDown {
    id: _tryCountDown
    visible: false
  }
}
