#ifndef EVENTS_H
#define EVENTS_H

#include <QEvent>
#include <functional>
#include <QString>
#include <memory>
#include "ilog.h"

using namespace std;

enum EventsType{
    evError = 1001,
    evLogChange,
    evSubLogCreated,
    evEmphasizeLine,
    evFindDone,
    evSourceReady,
};

typedef function<void()> EventAction;
class ILogSource;
class ILog;
class SubLog;

class BaseEvent: public QEvent {
public:
    BaseEvent(Type t)
        :QEvent(t){}
};

#define DEFINE_EVENT(name)\
    class name##Event : public BaseEvent {\
    public:\
        name##Event()\
            :BaseEvent(Type(ev##name))\
        {}\
    };

#define DEFINE_EVENT1(name, argType1, argName1)\
    class name##Event : public BaseEvent {\
    public:\
        name##Event(const argType1& in_##argName1)\
            :BaseEvent(Type(ev##name)), \
             argName1(in_##argName1)\
        {}\
    argType1 argName1;\
    };

#define DEFINE_EVENT2(name, argType1, argName1, argType2, argName2)\
    class name##Event : public BaseEvent {\
    public:\
        name##Event(const argType1& in_##argName1, const argType2& in_##argName2)\
            :BaseEvent(Type(ev##name)), \
             argName1(in_##argName1), \
             argName2(in_##argName2)\
        {}\
    argType1 argName1;\
    argType2 argName2;\
    };

#define DEFINE_EVENT3(name, argType1, argName1, argType2, argName2, argType3, argName3)\
    class name##Event : public BaseEvent {\
    public:\
        name##Event(const argType1& in_##argName1, const argType2& in_##argName2, const argType3& in_##argName3)\
            :BaseEvent(Type(ev##name)), \
             argName1(in_##argName1), \
             argName2(in_##argName2),\
             argName3(in_##argName3)\
        {}\
    argType1 argName1;\
    argType2 argName2;\
    argType3 argName3;\
    };

DEFINE_EVENT2(LogChange, shared_ptr<ILog>, who, EventAction, updateLog)

DEFINE_EVENT1(SubLogCreated, shared_ptr<SubLog>, log)

DEFINE_EVENT3(EmphasizeLine, shared_ptr<ILog>, who, int, line, bool, isSource)

DEFINE_EVENT2(FindDone, shared_ptr<ILog>, who, SearchResult, ret);

DEFINE_EVENT1(Error, EventAction, cleanup);

DEFINE_EVENT(SourceReady);

#endif // EVENTS_H
