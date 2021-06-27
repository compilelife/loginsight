#include "logviewer.h"
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QToolBar>
#include <QAction>

LogViewer::LogViewer(FocusManager* focusManager, bool allowUserSetMode)
{
    auto vbar = new QScrollBar;
    mLogEdit = new LogEdit(focusManager);
    mLogEdit->setExternalVScrollbar(vbar);

    auto layout = new QHBoxLayout;
    layout->addWidget(mLogEdit);

    if (allowUserSetMode) {
        auto box = new QGridLayout();
        box->addWidget(vbar,0,0,Qt::AlignHCenter);
        box->addWidget(buildSwitchModeButton(),1,0,Qt::AlignCenter);
        box->setRowStretch(0,1);
        layout->addLayout(box);

        mLogEdit->setMode(LogEdit::WatchMode);
    } else {
        layout->addWidget(vbar);
    }

    layout->setSpacing(0);
    layout->setMargin(0);

    setLayout(layout);
}

LogEdit *LogViewer::display()
{
    return mLogEdit;
}

QWidget *LogViewer::buildSwitchModeButton()
{
    auto btn = new QToolButton;
    btn->setChecked(false);
    btn->setToolTip("当前是review模式，点击切换");
    btn->setCheckable(true);
    btn->setMaximumSize(18,18);
    btn->setMinimumSize(18,18);
    btn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    btn->setIcon(QIcon(":/res/img/sync.png"));
    btn->setStyleSheet("QToolButton{background-color:#00FFFFFF;}");

    connect(btn, &QToolButton::clicked, [this](bool checked){
        mLogEdit->setMode(checked ? LogEdit::WatchMode : LogEdit::ReviewMode);
    });

    connect(mLogEdit, &LogEdit::onModeChanged, [btn](LogEdit::Mode newMode){
       if (newMode == LogEdit::WatchMode) {
           btn->setChecked(true);
           btn->setIcon(QIcon(":/res/img/sync.png"));
           btn->setToolTip("当前是watch模式，点击切换");
       } else {
           btn->setChecked(false);
           btn->setIcon(QIcon(":/res/img/eye.png"));
           btn->setToolTip("当前是review模式，点击切换");
       }
    });

    return btn;
}
