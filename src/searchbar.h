#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include "ilog.h"
#include <QComboBox>
#include <QToolButton>
#include <QCheckBox>
#include <QStringListModel>

class SearchBar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

private:
    QComboBox *buildKeywordCombo();
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
    QComboBox* mKeywordCombo;
    QToolButton* mBackwardBtn;
    QToolButton* mForwardBtn;
    QCheckBox* mCaseSensitiveCheckBox;
    QCheckBox* mUseRegexCheckBox;
    QStringListModel mCompleterModel;
    bool mSearchForward{true};
};

#endif // SEARCHBAR_H
