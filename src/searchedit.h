#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>

class SearchEdit: public QLineEdit
{
    Q_OBJECT
public:
    SearchEdit(QWidget* parent = nullptr);

public:
    void setSearchFoward(bool foward);
    bool isSearchFoward();

public slots:
    void transferReturnBehavior();
    void switchSearchDirection();
signals:
    void searchBackward();
    void searchFoward();
private slots:
    void editComplete();
private:
    bool mFoward{true};
    QAction* mSearchDirectionStatus;
    QStringListModel* mModel;
};

#endif // SEARCHEDIT_H
