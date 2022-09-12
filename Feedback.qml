import QtQuick 2.0
import QtQuick.Dialogs 1.3

Dialog {
  title: '问题与反馈'
  Text {
    text: `**作者邮箱**: compilelife@163.com\n\n**本次运行日志**：${NativeHelper.logPath()}\n\n**上次运行日志**: ${NativeHelper.logBackupPath()}\n\n**项目地址**：\n\n- https://github.com/compilelife/loginsight\n\n- https://gitee.com/compilelife/loginsight`
    textFormat: Text.MarkdownText
  }
}
