#ifndef COREBOOT_H
#define COREBOOT_H

#include <QObject>
#include <QProcess>

class CoreBoot : public QObject
{
    Q_OBJECT
public:
    explicit CoreBoot(QObject *parent = nullptr);

public:
    Q_INVOKABLE void startLocal();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void send(QString s);

signals:
    void stateChanged(bool running);
    void newLine(QString line);

private:
    QProcess mProcess;
    QByteArray mLastBuf;
};

#endif // COREBOOT_H
