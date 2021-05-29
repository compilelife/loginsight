#include "usercontrol.h"

#define ADD_ACTION(intent, iconName, textHint, s)\
    a = new QAction(QIcon(":/res/img/" iconName ".png"), textHint);\
    {QString shortcut(s);\
    if (!shortcut.isEmpty()) {\
        a->setShortcut(shortcut);\
    }}\
    mActions[intent] = a;

#define ADD_ACTION1(intent, textHint, s)\
    a = new QAction(textHint);\
    {QString shortcut(s);\
    if (!shortcut.isEmpty()) {\
        a->setShortcut(shortcut);\
    }}\
    mActions[intent] = a;

UserControl::UserControl()
{
    QAction* a;

    ADD_ACTION(GoBackwardIntent, "left", "后退", "Ctrl+[");
    ADD_ACTION(GoForwardIntent, "right", "前进", "Ctrl+]");
    ADD_ACTION(FilterIntent, "filter", "过滤", "Ctrl+L");
    ADD_ACTION(OpenFileIntent, "open", "打开本地文件...", "Ctrl+O");
    ADD_ACTION(LocateLineIntent, "locate", "跳转到行", "Ctrl+G");
    ADD_ACTION(CopyTimeLineIntent, "clipboard", "复制时间线到粘贴板", "Ctrl+S");
    ADD_ACTION(SaveTimeLineIntent, "export", "导出时间线到文件", "Ctrl+Shift+S");
    ADD_ACTION1(ClearTimeLineIntent, "清空", "");
    ADD_ACTION1(FindIntent, "查找", "Ctrl+F");
    ADD_ACTION1(RevertFindIntent, "反向查找", "Ctrl+Shift+F");
    ADD_ACTION1(FindNextIntent, "查找下一个", "Ctrl+N");
    ADD_ACTION1(FindPreviousIntent, "查找上一个", "Ctrl+M");
    ADD_ACTION1(HighlightIntent, "添加高亮规则", "Ctrl+J");
    ADD_ACTION1(UsageIntent, "使用文档", "");
    ADD_ACTION1(ShortcutIntent, "快捷键", "");
}

QString UserControl::getShortcutHint()
{
    QStringList seqs;
    seqs.push_back("[全局]");
    for (auto&& action : mActions.values()) {
        if(action->shortcut().isEmpty())
            continue;
        auto description = action->text() + ":" + action->shortcut().toString(QKeySequence::NativeText);
        seqs.push_back(description);
    }

    seqs.push_back("\n------------------");
    seqs.push_back("[日志文本框]");
    seqs.push_back("高亮所选词：Ctrl+E");
    seqs.push_back("过滤所选词：Ctrl+R");
    seqs.push_back("添加当前行到时间线：Ctrl+T");

    return seqs.join('\n');
}

UserControl &UserControl::instance()
{
    static UserControl impl;
    return impl;
}
