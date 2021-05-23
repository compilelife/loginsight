#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include "ilog.h"
#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QStringListModel>

class SearchBar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

private:
    QLineEdit* buildKeywordEdit();
    QToolButton* buildForwardBtn();
    QToolButton* buildBackwardBtn();

public slots:
    void fill(const QString& keyword);
    void setSearchForward(bool searchForward);
    void buildSearchRequest();
    void strongFocus();
    void forwardBtnClicked();
    void backwardBtnClicked();

signals:
    void searchRequest(SearchArg arg, bool forward);

private:
    QLineEdit* mKeywordEdit;
    QToolButton* mBackwardBtn;
    QToolButton* mForwardBtn;
    QCheckBox* mCaseSensitiveCheckBox;
    QCheckBox* mUseRegexCheckBox;
    QStringListModel mCompleterModel;
    bool mSearchForward{true};
};

#endif // SEARCHBAR_H
