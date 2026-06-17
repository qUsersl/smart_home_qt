#ifndef HTTPPOST_H
#define HTTPPOST_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QString>

class httppost : public QObject
{
    Q_OBJECT
public:
    explicit httppost(QObject *parent = nullptr);
    bool postMsg(QString url, QMap<QString, QString> headerdata,
                 QByteArray requestData, QByteArray &replyData);
};

#endif // HTTPPOST_H
