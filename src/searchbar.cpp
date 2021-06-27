#include "searchbar.h"
#include <QHBoxLayout>
#include <QCompleter>
#include <QLabel>
#include <QSettings>

SearchBar::SearchBar(QWidget *parent) : QWidget(parent)
{
    auto box = new QHBoxLayout;
    box->setMargin(0);

    mKeywordCombo = buildKeywordCombo();
    box->addWidget(mKeywordCombo);
    box->setStretch(0, 1);

    mForwardBtn = buildForwardBtn();
    box->addWidget(mForwardBtn);

    mBackwardBtn = buildBackwardBtn();
    box->addWidget(mBackwardBtn);

    mCaseSensitiveCheckBox = new QCheckBox("大小写敏感");
    box->addWidget(mCaseSensitiveCheckBox);

    mUseRegexCheckBox = new QCheckBox("使用正则表达式");
    box->addWidget(mUseRegexCheckBox);

    setLayout(box);

    QSettings config;
    auto caseSentive = config.value("caseSensitive").toBool();
    mCaseSensitiveCheckBox->setChecked(caseSentive);
}

QComboBox *SearchBar::buildKeywordCombo()
{
    auto completer = new QCompleter;
    completer->setCaseSensitivity(Qt::CaseSensitive);
    completer->setModel(&mCompleterModel);
    connect(completer, SIGNAL(highlighted(const QString &)), this, SLOT(fill(const QString &)));
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(fill(const QString &)));

    auto edit = new QComboBox;
    edit->setEditable(true);
    edit->setCompleter(completer);
    connect(edit, QOverload<int>::of(&QComboBox::activated), this, &SearchBar::buildSearchRequest);

    return edit;
}

QToolButton *SearchBar::buildForwardBtn()
{
    auto btn = new QToolButton;
    btn->setIcon(QIcon(":/res/img/down.png"));
    connect(btn, &QToolButton::clicked, this, &SearchBar::forwardBtnClicked);
    return btn;
}

QToolButton *SearchBar::buildBackwardBtn()
{
    auto btn = new QToolButton;
    btn->setIcon(QIcon(":/res/img/up.png"));
    connect(btn, &QToolButton::clicked, this, &SearchBar::backwardBtnClicked);
    return btn;
}

void SearchBar::fill(const QString &keyword)
{
    mKeywordCombo->setEditText(keyword);
    strongFocus();
}

void SearchBar::setSearchForward(bool searchForward)
{
    mSearchForward = searchForward;
    if (searchForward) {
        mForwardBtn->setIcon(QIcon(":/res/img/enter_down.png"));
        mBackwardBtn->setIcon(QIcon(":/res/img/up.png"));
    } else {
        mForwardBtn->setIcon(QIcon(":/res/img/down.png"));
        mBackwardBtn->setIcon(QIcon(":/res/img/enter_up.png"));
    }
}

void SearchBar::strongFocus()
{
    //TODO: 用动画强调
    mKeywordCombo->setFocus();
    mKeywordCombo->showPopup();
}

void SearchBar::buildSearchRequest()
{
    if (mKeywordCombo->currentText().isEmpty())
        return;

    SearchArg arg;
    arg.caseSensitive = mCaseSensitiveCheckBox->isChecked();
    arg.regex = mUseRegexCheckBox->isChecked();
    arg.pattern = mKeywordCombo->currentText();
    emit searchRequest(arg, mSearchForward);

    if(QString::compare(arg.pattern, QString("")) != 0) {
        QStringList candidates = mCompleterModel.stringList();
        if (!candidates.contains(arg.pattern, Qt::CaseInsensitive)) {
            candidates.append(arg.pattern);
            mCompleterModel.setStringList(candidates);
        }
    }
}

void SearchBar::forwardBtnClicked()
{
    mSearchForward = true;
    mForwardBtn->setIcon(QIcon(":/res/img/enter_down.png"));
    mBackwardBtn->setIcon(QIcon(":/res/img/up.png"));

    buildSearchRequest();
}

void SearchBar::backwardBtnClicked()
{
    mSearchForward = false;
    mForwardBtn->setIcon(QIcon(":/res/img/down.png"));
    mBackwardBtn->setIcon(QIcon(":/res/img/enter_up.png"));

    buildSearchRequest();
}
