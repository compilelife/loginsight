#include "nativehelper.h"
#include <QApplication>
#include <QClipboard>
#include <QProcess>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QSysInfo>

NativeHelper::NativeHelper(QObject *parent)
    : QObject{parent}
{

}

void NativeHelper::clipboardSetImage(const QImage img)
{
    qApp->clipboard()->setImage(img);
}

void NativeHelper::clipboardSetText(const QString &txt)
{
    qApp->clipboard()->setText(txt);
}

QString NativeHelper::writeClipboardToTemp()
{
    auto content = qApp->clipboard()->text();
    if (content.isEmpty()) {
        qDebug()<<"clipboard text is empty";
        return "";
    }

    auto path = QDir::temp().filePath("loginsight-clipboard.txt");

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug()<<"failed to open "<<path;
        return "";
    }

    f.write(content.toLocal8Bit());
    f.close();

    return path;
}

void NativeHelper::relaunch()
{
    //FIXME: 本意是要实现relaunch，即启动一个新的进程，然后退出当前进程
    //然而实际效果却是新的进程起来了，但是旧的进程也没退出
    //QProcess::startDetached(qApp->applicationFilePath(), QStringList());
    qApp->quit();
}

bool NativeHelper::writeToFile(QString path, QString txt)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(txt.toLocal8Bit());
        f.close();
        return true;
    }
    return false;
}

QString NativeHelper::readFile(QString path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        auto s = QString::fromLocal8Bit(f.readAll());
        f.close();
        return s;
    }
    return "";
}

QString NativeHelper::settingsPath()
{
    return QDir::home().filePath(".loginsight/settings.json");
}

QString NativeHelper::myDir()
{
    return QDir::home().filePath(".loginsight");
}

QString NativeHelper::uniqueId()
{
    return QSysInfo::machineUniqueId();
}

QString NativeHelper::encodePath(QString path)
{
    return path.toLocal8Bit().toBase64();
}

QVector<QString> NativeHelper::supportCodecs()
{
    QVector<QString> list;
    list.push_back("UTF-8");
//    list.push_back("UTF-16LE");
//    list.push_back("UTF-16BE");
    list.push_back("GBK");
    return list;
}

QString NativeHelper::getBinDir()
{
    return qApp->applicationDirPath();
}

QString NativeHelper::exeNativeName(QString name)
{
#ifdef Q_OS_LINUX
    return name + ".linux";
#elif defined(Q_OS_MACOS)
    return name + ".macosx";
#elif defined(Q_OS_WIN)
    return name + ".windows.exe";
#else
    return name;
#endif
}
