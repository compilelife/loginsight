#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <functional>
#include <QMap>

class HttpManager : public QObject
{
    Q_OBJECT
private:
    explicit HttpManager(QObject *parent = nullptr);

public:
    static HttpManager& instance();
    void get(const QNetworkRequest& req, std::function<void(QNetworkReply*)> handler);

private:
    QNetworkAccessManager mNetworkAccess;
};

#endif // HTTPMANAGER_H
