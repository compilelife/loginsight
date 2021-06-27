#ifndef USERCONTROL_H
#define USERCONTROL_H

#include <QAction>
#include <QMap>
#include "searchbar.h"
#include "taglistwidget.h"

class UserControl
{
public:
    enum Intent {
        GoBackwardIntent,
        GoForwardIntent,
        FilterIntent,
        OpenFileIntent,
        OpenClipboardIntent,
        OpenProcessIntent,
        CloseTabIntent,
        LocateLineIntent,
        CopyTimeLineIntent,
        SaveTimeLineIntent,
        ClearTimeLineIntent,
        FindIntent,
        RevertFindIntent,
        FindNextIntent,
        FindPreviousIntent,
        HighlightIntent,
        UsageIntent,
        ShortcutIntent,
        PauseSourceIntent,
        ExportSubLogIntent,
        SaveProjectIntent,
    };
private:
    UserControl();

public:
    QAction* actionFor(Intent intent) {
        return mActions[intent];
    }

    SearchBar* searchBar() {
        return mSearchBar;
    }

    void setSearchBar(SearchBar* bar) {
        mSearchBar = bar;
    }

    TagListWidget* tagList() {
        return mTagList;
    }

    void setTagList(TagListWidget* taglist) {
        mTagList = taglist;
    }

    QString getShortcutHint();

    static UserControl& instance();

    QVector<QAction*> actionsDisableWhenNoDoc(){
        return mNodocDisbaleActions;
    }

private:
    QMap<Intent, QAction*> mActions;
    QVector<QAction*> mNodocDisbaleActions;
    SearchBar* mSearchBar;
    TagListWidget* mTagList;
};

#endif // USERCONTROL_H
