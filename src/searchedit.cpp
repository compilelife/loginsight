#include "searchedit.h"
#include <QIcon>
#include <QAction>
#include <QStringListModel>

SearchEdit::SearchEdit(QWidget* parent)
    :QLineEdit(parent)
{
    auto a = addAction(QIcon(":/res/img/up.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SIGNAL(searchBackward()));
    a = addAction(QIcon(":/res/img/down.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SIGNAL(searchFoward()));
    connect(this, SIGNAL(returnPressed()), this, SLOT(transferReturnBehavior()));

    mSearchDirectionStatus = addAction(QIcon(":/res/img/enter_down.png"), QLineEdit::LeadingPosition);
    connect(mSearchDirectionStatus, SIGNAL(triggered()), this, SLOT(switchSearchDirection()));
    connect(this, SIGNAL(editingFinished()), this, SLOT(editComplete()));

    QCompleter* completer = new QCompleter;
    completer->setCaseSensitivity(Qt::CaseSensitive);
    mModel = new QStringListModel;
    completer->setModel(mModel);
    setCompleter(completer);

    connect(completer, SIGNAL(highlighted(const QString &)), this, SLOT(setText(const QString &)));
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(setText(const QString &)));
}

void SearchEdit::setSearchFoward(bool foward)
{
    mFoward = foward;
    mSearchDirectionStatus->setIcon(QIcon(mFoward ? ":/res/img/enter_down.png" : ":/res/img/enter_up.png"));
}

bool SearchEdit::isSearchFoward()
{
    return mFoward;
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

void SearchEdit::switchSearchDirection()
{
    mFoward = !mFoward;
    mSearchDirectionStatus->setIcon(QIcon(mFoward ? ":/res/img/enter_down.png" : ":/res/img/enter_up.png"));
}

void SearchEdit::editComplete()
{
    QString text = this->text();
    if(QString::compare(text, QString("")) != 0) {
        QStringList candidates = mModel->stringList();
        if (!candidates.contains(text, Qt::CaseInsensitive)) {
            candidates.append(text);
            mModel->setStringList(candidates);
        }
    }

    selectAll();//方便搜索新的关键字
}
