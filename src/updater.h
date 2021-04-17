#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAtomicInt>

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

signals:
    void newVersionFound(const QString& version);
    void noNewVersion();

public slots:
private slots:
    void httpReplied(QNetworkReply* reply);
private:
    QNetworkAccessManager mNetwork;
    QString mCurrentVersion;
    QString mLatestVersion;
};

#endif // UPDATER_H
