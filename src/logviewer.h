#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include "logedit.h"
#include "focusmanager.h"

class LogViewer : public QWidget
{
public:
    LogViewer(FocusManager* focusManager, bool allowUserSetMode);
public:
    LogEdit* display();

private:
    QWidget* buildSwitchModeButton();

private:
    LogEdit* mLogEdit;
};

#endif // LOGVIEWER_H
