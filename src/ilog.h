#ifndef ILOG_H
#define ILOG_H

#include <memory>
#include <QString>
#include <list>
#include <QJsonValue>
#include "longtimeoperation.h"
#include <QFile>
#include <QProgressDialog>

using namespace std;

struct Location {
    int line{0};
    int offset{0};
    Location(){}
    Location(int line, int offset) {
        this->line = line;
        this->offset = offset;
    }
};

struct Range {
    int from{1};
    int to{0};

    bool contains(int value);
    Range(){}
    Range(int from, int to){
        this->from = from;
        this->to = to;
    }

    bool isValid() {
        return to >= from;
    }

    Range intersect(Range& r);

    bool operator == (const Range& r) {
        return from == r.from && to == r.to;
    }

    int size() const{
        return to - from + 1;
    }
};

struct SearchArg {
    bool regex{false};
    bool caseSensitive{false};
    QString pattern;
    bool revert{false};

    QJsonObject saveToJson();
    void loadFromJson(QJsonObject o);
};

struct SearchResult : public Location {
    SearchArg arg;
    bool forward;

    QString matchedText{""};
    bool isValid() const{
        return offset >= 0;
    }
};

class ILogSource;
class BaseEvent;

class ILog: public enable_shared_from_this<ILog>
{
public:
    ILog(QString type);
    virtual ~ILog(){}

public:
    virtual QString readLines(int from, int to);
    virtual QByteArray readRawLines(int from, int to) = 0;
    virtual Range availRange() = 0;

    virtual int fromParentLine(int){return -1;}
    virtual int toParentLine(int){return -1;}
    virtual bool onParentRangeChanged(Range, Range){return false;};

    int toRootLine(int line);
    shared_ptr<ILog> rootLog();

    QString readLine(int line){
        return readLines(line,line);
    }

    shared_ptr<ILog> parent();
    list<shared_ptr<ILog>> children();

    shared_ptr<LongtimeOperation> createChild(SearchArg arg, QString id = "");
    shared_ptr<LongtimeOperation> find(SearchArg arg, Location loc, bool forward);

    void setSource(const shared_ptr<ILogSource>& source) {
        mSource = source;
    }

    void post(BaseEvent* ev);

    QString type();

    virtual bool saveTo(QFile&, QProgressDialog&) {return false;};

public:
    QString getType(){return mType;}
    void setId(QString id){mId = id;}
    QString getId() {return mId;}
    shared_ptr<ILog> findLogById(QString id);

private:
    shared_ptr<ILog> deepFindLogById(QString id);

protected:
    shared_ptr<ILog> mParent;
    list<weak_ptr<ILog>> mChildren;
    shared_ptr<ILogSource> mSource;
    QString mType;
    QString mId;
};

#endif // ILOG_H
