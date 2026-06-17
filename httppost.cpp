#include "httppost.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>

httppost::httppost(QObject *parent) : QObject(parent)
{
}

bool httppost::postMsg(QString url, QMap<QString, QString> headerdata,
                       QByteArray requestData, QByteArray &replyData)
{
    QNetworkAccessManager manager;

    QNetworkRequest request;
    request.setUrl(QUrl(url));

    QMapIterator<QString, QString> it(headerdata);
    while (it.hasNext()) {
        it.next();
        request.setRawHeader(it.key().toLatin1(), it.value().toLatin1());
    }

    QNetworkReply *reply = manager.post(request, requestData);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply != nullptr && reply->error() == QNetworkReply::NoError) {
        replyData = reply->readAll();
        qDebug() << replyData;
        reply->deleteLater();
        return true;
    } else {
        qDebug() << "请求失败";
        if (reply != nullptr) {
            qDebug() << "Error:" << reply->errorString();
            reply->deleteLater();
        }
        return false;
    }
}
