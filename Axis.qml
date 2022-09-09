import QtQuick 2.0
import QtQuick.Layouts 1.3

ColumnLayout {
    spacing: 0
    Rectangle {
        width: 10
        height: 10
        radius: 5
        color: 'grey'
        Layout.alignment: Qt.AlignCenter
    }

    Rectangle {
        color: 'grey'
        Layout.preferredWidth: 2
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignCenter
    }
}
