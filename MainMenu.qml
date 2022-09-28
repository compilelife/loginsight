import QtQuick 2.0
import QtQuick.Controls 1.4
import './app.js' as App
import './coredef.js' as CoreDef
import './constants.js' as C

Menu {
  Menu {
    title: "文件"
    MenuItem {
      action: actions.open
    }
    MenuItem {
      action: actions.openMulti
    }
    MenuItem {
      action: actions.openProcess
    }
    MenuItem {
      action: actions.openClipboard
    }
    Menu {
      title: "最近文件"
      id: recentMenu
      function fillItems(recents, getText, onAction) {
        while (items.length > 0)
          removeItem(items[0])
        recents.forEach(function (openAction) {
          if (!openAction.action)
            //低版本使用的是字符串，直接过滤掉
            return
          addItem(getText(openAction))
              .triggered.connect(() => onAction(openAction))
        })
      }
    }
    MenuSeparator {}
    MenuItem {
      action: actions.saveProject
    }
    MenuItem {
      action: actions.exportLog
    }
    MenuSeparator {}
    MenuItem {
      action: actions.close
    }
    MenuSeparator {}
    Menu {
      title: '编码'
      Repeater {
        model: NativeHelper.supportCodecs()
        delegate: MenuItem {
          text: modelData
          onTriggered: {
            const session = App.currentSession
            if (!session)
              return
            session.textCodec.name = text
            session.invalidate(true)
          }
        }
      }
    }
  }
  Menu {
    title: "检视"
    MenuItem {
      action: actions.filter
    }
    MenuItem {
      action: actions.search
    }
    MenuSeparator {}
    MenuItem {
      action: actions.goTo
    }
    MenuItem {
      action: actions.goBack
    }
    MenuItem {
      action: actions.goForward
    }
    MenuSeparator {}
    MenuItem {
      action: actions.copyLines
    }
    MenuItem {
      action: actions.setSyntax
    }
  }
  Menu {
    title: "时间线"
    MenuItem {
      action: actions.addToTimeLine
    }
    MenuItem {
      action: actions.clearTimeLine
    }
    MenuItem {
      action: actions.shotTimeLine
    }
  }
  Menu {
    title: "其它"
    id: otherMenu
    MenuItem {
      action: actions.toggleMenuBar
    }
    MenuItem {
      action: actions.settings
    }
    MenuItem {
      action: actions.about
    }
    MenuItem {
      action: actions.feedback
    }
  }

  function handleRegisterState(rstate) {
    if (rstate === CoreDef.RSTry || rstate === CoreDef.RSTryEnd) {
      otherMenu.addItem('').action = actions.register
    } else if (rstate === CoreDef.RSOpenSource) {
      otherMenu.addItem('开源版本，点击试用专业版').triggered.connect(function(){
        Qt.openUrlExternally(C.WEB_DOWNLOAD_URL)
      })
    } else if (rstate === CoreDef.RSRegistered) {
      otherMenu.addItem('已购买!')
    }
  }

  function updateRecents() {
    const recents = App.settings.recents
    recentMenu.fillItems(recents, App.main.userTextOfOpenAction, App.main.replayOpenAction)
  }
}
