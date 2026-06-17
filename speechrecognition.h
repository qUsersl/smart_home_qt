#ifndef SPEECHRECOGNITION_H
#define SPEECHRECOGNITION_H

#include <QObject>
#include <QAudioFormat>
#include <QByteArray>
#include <QString>

class speechrecognition : public QObject
{
    Q_OBJECT
public:
    explicit speechrecognition(QObject *parent = nullptr);
    QString speechIdentify(QString filename, const QAudioFormat &format);

private:
    QString getJsonvalue(QByteArray ba, QString key);
    QString accessToken;
};

#endif // SPEECHRECOGNITION_H
