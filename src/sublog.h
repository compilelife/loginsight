#ifndef SUBLOG_H
#define SUBLOG_H

#include "ilog.h"
#include <QList>

class SubLog : public ILog
{
public:
    SubLog(shared_ptr<ILog> parent, SearchArg arg);
    ~SubLog();

public:
    QString readLines(int from, int to) override;
    QByteArray readRawLines(int from, int to) override;
    Range availRange() override;

    int fromParentLine(int line) override;
    int toParentLine(int line) override;

    SearchArg getSearchArg();

    bool onParentRangeChanged(Range before, Range after) override;

    bool saveTo(QFile& path, QProgressDialog& progess) override;

private:
    SearchArg mSearchArg;
    friend class ILog;
    QVector<int> mLineInParent;
    int mFromLineNum{1};
};

#endif // SUBLOG_H
