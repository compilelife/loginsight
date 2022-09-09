#include "coreboot.h"
#include <QDebug>
#include <QTcpServer>
#include "nativehelper.h"

CoreBoot::CoreBoot(QObject *parent)
    : QObject{parent}
{
    connect(&mProcess, &QProcess::stateChanged, [this](QProcess::ProcessState state){
        if (state == QProcess::NotRunning) {
            emit stateChanged(false);
        } else if (state == QProcess::Running) {
            emit stateChanged(true);
        }
    });

    connect(&mProcess, &QProcess::readyRead, [this]{
        //canReadLine -> readLine 在mac下工作异常，只能手动分行了
        auto buf = mProcess.readAll();
        if (mLastBuf.size() > 0) {
            buf.insert(0, mLastBuf);
            mLastBuf.clear();
        }
        auto pos = buf.indexOf('\n');
        while (pos >= 0) {
            auto line = buf.left(pos+1);
            buf.remove(0, pos+1);
            emit newLine(line);
            pos = buf.indexOf('\n');
        }
        if (buf.size() > 0) {
            mLastBuf = buf;
        }
    });
}

void CoreBoot::startLocal() {
    NativeHelper native;
    mProcess.setProgram(native.getBinDir()+"/"+native.exeNativeName("core"));
    mProcess.start();
}

void CoreBoot::stop() {
    mProcess.close();
}

void CoreBoot::send(QString s) {
    mProcess.write((s+'\n').toLocal8Bit());
}
