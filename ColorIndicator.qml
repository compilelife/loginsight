import QtQuick 2.0
import QtQuick.Dialogs 1.2

//TODO: 优化当前默认颜色选择器：点击后右侧展开常用色列表，及更多；更多中选择的颜色将更新常用色列表
Item {
    id: root
    property string color: 'blue'
    height: 20

    Text {
        id: hint
        anchors.verticalCenter: parent.verticalCenter
        text: '新建节点颜色：'
    }

    Rectangle {
        anchors.left: hint.right
        anchors.verticalCenter: parent.verticalCenter
        width: root.height
        height: root.height
        color: root.color
        border.color: 'black'
        MouseArea{
            anchors.fill: parent
            onClicked: {
                selectColorDialog.open()
            }
        }
    }

    ColorDialog {
        id: selectColorDialog
        onAccepted: {
            root.color = color
        }
    }
}
