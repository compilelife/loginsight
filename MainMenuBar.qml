import QtQuick 2.0
import QtQuick.Controls 1.4
import './app.js' as App

MenuBar {
  readonly property MainMenu tinyMenuBar: MainMenu{}
  readonly property MainMenu template: MainMenu{}

  Component.onCompleted: {
    for (let i = 0; i < template.items.length; i++) {
      menus.push(template.items[i])
    }
  }

  function handleRegisterState(rstate) {
    tinyMenuBar.handleRegisterState(rstate)
    template.handleRegisterState(rstate)
  }

  function loadSettings() {
    App.actions.toggleMenuBar.checked = App.settings.showMenuBar
    setVisible(App.settings.showMenuBar)//FIXME:正常上面这行应该是会触发setVisible的，然而没有，为什么？
    updateRecents()
  }

  function updateRecents() {
    tinyMenuBar.updateRecents()
    template.updateRecents()
  }

  function setVisible(v) {
    for (let i = 0; i < template.items.length; i++) {
      template.items[i].visible = v
    }
  }
}
