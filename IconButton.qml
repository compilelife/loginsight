import QtQuick 2.0
import QtQuick.Controls 2.15

Button {
  property int size: 0
  property string source: ''
  width: size
  height: size
  display: AbstractButton.IconOnly
  icon.source: source
  flat: true
  padding: 0
}
