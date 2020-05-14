#include "searchedit.h"
#include <QIcon>
#include <QAction>

SearchEdit::SearchEdit(QWidget* parent)
    :QLineEdit(parent)
{
    auto a = addAction(QIcon(":/res/img/up.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SIGNAL(searchBackward()));
    a = addAction(QIcon(":/res/img/down.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SIGNAL(searchFoward()));
    connect(this, SIGNAL(returnPressed()), this, SLOT(transferReturnBehavior()));
}

void SearchEdit::setSearchFoward(bool foward)
{
    mFoward = foward;
}

void SearchEdit::focusInEvent(QFocusEvent *)
{
    selectAll();
}

void SearchEdit::transferReturnBehavior()
{
    if (this->text().isEmpty()) {
        return;
    }

    if (mFoward)
        emit searchFoward();
    else
        emit searchBackward();
}
