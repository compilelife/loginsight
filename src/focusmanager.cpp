#include "focusmanager.h"
#include <QApplication>

FocusManager::FocusManager() {
    mConnectId = connect(qApp, &QApplication::focusChanged, [this](QWidget*, QWidget* now){
        if (mItems.contains(now)) {
            auto old = mCurrent;
            mCurrent = now;
            emit focusChanged(old, mCurrent);
        }
    });
}

FocusManager::~FocusManager()
{
    disconnect(mConnectId);
}


void FocusManager::add(QWidget *edit) {
    mItems.insert(edit);

    auto old = mCurrent;
    mCurrent = edit;
    emit focusChanged(old, mCurrent);
}

void FocusManager::remove(QWidget *edit) {
    mItems.remove(edit);
    if (mCurrent == edit && !mItems.isEmpty()) {
        auto old = mCurrent;
        mCurrent = *(mItems.begin());
        emit focusChanged(old, mCurrent);
    }
}

bool FocusManager::isInFocus(QWidget *edit) {
    return mCurrent == edit;
}

QWidget* FocusManager::current() {
    return mCurrent;
}
