import QtQuick 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4 as QC1

Item {
    id: root
    property var config: ({})

    signal clicked()
    signal doubleClicked()
    signal recommendHeight(int height)
    signal requestDelete()

    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {//单击时聚焦到备注框
            commentEdit.forceActiveFocus()
            if (mouse.button === Qt.RightButton) {
                optionMenu.popup()
            }
        }
        onDoubleClicked: {
            root.doubleClicked()
        }
    }

    onActiveFocusChanged: {//获取焦点时聚焦到备注框
        commentEdit.forceActiveFocus()
    }

    ColorDialog {
        id: selectColorDialog
        onAccepted: {
            config.color = String(color)
        }
    }

    QC1.Menu {
        id: optionMenu
        QC1.MenuItem {
          text: '追踪该行'
          onTriggered: doubleClicked()
        }
        QC1.MenuItem {
            text: '选择颜色'
            onTriggered: {
                selectColorDialog.open()
            }
        }
        QC1.MenuItem {
            text: '删除'
            onTriggered: requestDelete()
        }
    }

    RowLayout {
        id: body
        anchors.fill: parent

        property int lineNumWidth: 50
        property int circleSize: 10
        property int circleMargin: 15
        property int mainAreaWidth: 200

        onWidthChanged: {
            mainAreaWidth = Math.max(150, width - lineNumWidth - circleSize - 2 * circleMargin)
        }

        Text {
            text: config.line + 1
            color: config.color
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
            wrapMode: Text.WrapAnywhere

            Layout.minimumWidth: body.lineNumWidth
            Layout.maximumWidth: body.lineNumWidth
        }

        Rectangle {//圆点
            width: body.circleSize
            height: body.circleSize
            radius: body.circleSize/2
            color: config.color
            Layout.leftMargin: body.circleMargin
            Layout.rightMargin: body.circleMargin
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            width: body.mainAreaWidth

            Rectangle {//白色背景
                id: background
                anchors.fill: parent
                color: 'white'
            }

            DropShadow {//制造一个阴影
                anchors.fill: background
                verticalOffset: 1
                horizontalOffset: 1
                radius: 8
                color: '#80000000'
                source: background
            }

            Rectangle {
                id: barline
                width: 3
                color: config.color
                //以下两行可以填满纵向
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            ColumnLayout {
                id: mainArea
                anchors.left: barline.right
                anchors.leftMargin: 4
                anchors.rightMargin: 10
                width: body.mainAreaWidth - 3 - anchors.leftMargin - anchors.rightMargin

                onHeightChanged: {//这里发出信号，是为了告诉外层布局最佳高度是多少
                    recommendHeight(mainArea.height)
                }

                Text {
                    text: config.log
                    Layout.fillWidth: true
                    Layout.maximumHeight: 120
                    Layout.minimumHeight: 35
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    color: '#303030'
                }

                Rectangle {//中间的分割虚线
                    height: 1
                    Layout.preferredWidth: 4*parent.width/5
                    Layout.alignment: Qt.AlignHCenter
                    color: '#d0d0d0'
                }

                TextEdit {
                    id: commentEdit
                    text: config.comment
                    wrapMode: TextInput.Wrap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: config.color
                    onTextChanged: {
                        config.comment = text
                    }
                    onActiveFocusChanged: {
                        if (activeFocus) {
                            clicked()
                            //强制光标到最后位置
                            forceActiveFocus()
                            cursorPosition = text.length
                        }
                    }
                }
            }
        }
    }
}
