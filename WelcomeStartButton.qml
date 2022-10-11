import QtQuick 2.0
import QtQuick.Controls 2.12

Row {
  property var action: null
  property string prompt: ''

  Image {
    visible: action.iconSource.toString().length > 0
    source: action.iconSource
    width: link.font.pixelSize+4
    height: width
  }

  Label{
    id: link
    text: `<a href="${action.text}">${prompt.length > 0 ? prompt : action.text}</a>`
    linkColor: 'blue'
    onLinkActivated: action.trigger(this)
    enabled: action.enabled
  }
}
