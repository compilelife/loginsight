#include "httpmanager.h"

HttpManager::HttpManager(QObject *parent) : QObject(parent)
{
}

HttpManager &HttpManager::instance()
{
    static HttpManager impl;
    return impl;
}

void HttpManager::get(const QNetworkRequest &req, std::function<void (QNetworkReply *)> handler)
{
    auto reply = mNetworkAccess.get(req);
    connect(reply, &QNetworkReply::finished, [handler, reply](){
        handler(reply);
        reply->deleteLater();
    });
}
