#ifndef REGISTER_H
#define REGISTER_H

#include <QString>
#include <QJsonArray>
#include <QJsonObject>

class Register
{
public:
    enum RegisterStatus{
        eRegistered,
        eTry,
        eTryEnd,
    };

private:
    Register();

public:
    static QString thisPlatform();
    static QString thisMachineId();
    static Register& instance();

public:
    void init();
    RegisterStatus getState();
    QString getStateStr();
    qint64 getLeftSecs();
    void registerToServer(QString orderid, QString machineid, QString platform);

private:
    QString xordecode(QJsonArray& arr, int key);
    bool verify(QJsonObject& license);

private:
    RegisterStatus mStatus{eTryEnd};
    qint64 mLeftSecs{0};
};

#endif // REGISTER_H
