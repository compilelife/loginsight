#include "util.h"
#include <QDir>
#include <QProcess>
#include <QApplication>

void relaunchApp()
{
    qApp->quit();
    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
}

QString makeShortPath(QString path, int maxLen)
{
    auto homePath = QDir::homePath();
#ifndef Q_OS_WIN
    if (path.startsWith(homePath))
        path.replace(0, homePath.length(), "~");
#endif

    QString shortPath = path;
    if (path.length() > maxLen) {
        auto segs = path.split(QDir::separator());
        if (segs.length() > 3){
            int head = 0;
            int tail = segs.size()-1;
            maxLen -= (segs[head++].length() + segs[tail--].length());
            while (maxLen > 0 && tail > head) {
                bool added = false;

                auto headSize = segs[head].length();
                if (maxLen - headSize > 0) {
                    head++;
                    maxLen -= headSize;
                    added = true;
                }
                auto tailSize = segs[tail].length();
                if (maxLen - tailSize > 0) {
                    tail--;
                    maxLen -= tailSize;
                    added = true;
                }

                if (!added)
                    break;
            }
            shortPath = segs.mid(0, head).join(QDir::separator())
                    + "/.../"
                    +   segs.mid(tail+1).join(QDir::separator());
        } else {
            shortPath = path.mid(0, maxLen) ;
        }
    }
    return shortPath;
}
