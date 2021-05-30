#include "ilogsource.h"
#include "filesource.h"


ILogSource *ILogSource::createLogSourceFromType(QString type, QObject* eventHandler)
{
    if (type == "file") {
        return new FileSource(eventHandler);
    }
    return nullptr;
}

QJsonValue ILogSource::saveToJson() {
    QJsonObject o;
    o["type"] = mType;
    o["codec"] = QString(mCodec->name());
    return o;
}

void ILogSource::loadFromJson(const QJsonValue &jo) {
    mCodec = QTextCodec::codecForName(jo.toObject()["codec"].toString().toLocal8Bit());
}
