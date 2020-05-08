#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>

class SearchEdit: public QLineEdit
{
    Q_OBJECT
public:
    SearchEdit(QWidget* parent = nullptr);

public:
    void setSearchFoward(bool foward);

protected:
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
private slots:
    void transferReturnBehavior();
signals:
    void searchBackward();
    void searchFoward();
private:
    bool mFoward=true;
};

#endif // SEARCHEDIT_H
