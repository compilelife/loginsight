#include "usercontrol.h"

#define ADD_ACTION(intent, iconName, textHint, s, nodocDisable)\
    a = new QAction(QIcon(":/res/img/" iconName ".png"), textHint);\
    {QString shortcut(s);\
    if (!shortcut.isEmpty()) {\
        a->setShortcut(shortcut);\
    }}\
    mActions[intent] = a;\
    if (nodocDisable) mNodocDisbaleActions.append(a);

#define ADD_ACTION1(intent, textHint, s, nodocDisable)\
    a = new QAction(textHint);\
    {QString shortcut(s);\
    if (!shortcut.isEmpty()) {\
        a->setShortcut(shortcut);\
    }}\
    mActions[intent] = a;\
    if (nodocDisable) mNodocDisbaleActions.append(a);

UserControl::UserControl()
{
    QAction* a;

    ADD_ACTION(GoBackwardIntent, "left", "后退", "Ctrl+[", true);
    ADD_ACTION(GoForwardIntent, "right", "前进", "Ctrl+]", true);
    ADD_ACTION(FilterIntent, "filter", "过滤", "Ctrl+L", true);
    ADD_ACTION(OpenFileIntent, "open", "打开本地文件...", "Ctrl+O", false);
    ADD_ACTION(PauseSourceIntent, "pause", "暂停日志源", "Ctrl+P", true);
    ADD_ACTION1(CloseTabIntent, "关闭当前标签页", "Ctrl+W", true);
    ADD_ACTION(LocateLineIntent, "locate", "跳转到行", "Ctrl+G", true);
    ADD_ACTION(CopyTimeLineIntent, "clipboard", "复制时间线到粘贴板", "Ctrl+S", true);
    ADD_ACTION(SaveTimeLineIntent, "export", "导出时间线到文件", "Ctrl+Shift+S", true);
    ADD_ACTION1(ClearTimeLineIntent, "清空", "", true);
    ADD_ACTION1(FindIntent, "查找", "Ctrl+F", true);
    ADD_ACTION1(RevertFindIntent, "反向查找", "Ctrl+Shift+F", true);
    ADD_ACTION1(FindNextIntent, "查找下一个", "Ctrl+N", true);
    ADD_ACTION1(FindPreviousIntent, "查找上一个", "Ctrl+M", true);
    ADD_ACTION1(HighlightIntent, "添加高亮规则", "Ctrl+J", true);
    ADD_ACTION1(UsageIntent, "使用文档", "", false);
    ADD_ACTION1(ShortcutIntent, "快捷键", "", false);
    ADD_ACTION1(SaveProjectIntent, "保存工程...", "", true);
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
