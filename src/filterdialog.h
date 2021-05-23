#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include "ilog.h"

class FilterDialog : public QDialog
{
public:
    FilterDialog();

    SearchArg getArg(){return mArg;}

private:
    SearchArg mArg;
};

#endif // FILTERDIALOG_H
