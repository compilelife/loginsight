#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class Updater : public QObject
{
    Q_OBJECT
private:
    explicit Updater(QObject *parent = nullptr);
public:
    static Updater& instance();
    void checkNewVersion();

    QString currentVersion() {return mCurrentVersion;}
    QString latestVersion() {return mLatestVersion;}
    QString changeLog() {return mChangeLog;}

    bool hasCheckUpdate(){return mHasCheckUpdate;}

signals:
    void newVersionFound(const QString& version);
    void noNewVersion();

private:
    QString mCurrentVersion;
    QString mLatestVersion;
    bool mHasCheckUpdate{false};
    QString mChangeLog;
};

#endif // UPDATER_H
