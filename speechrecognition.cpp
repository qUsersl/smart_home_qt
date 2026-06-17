#include "speechrecognition.h"
#include "httppost.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

const QString baiduTokenUrl =
    "https://aip.baidubce.com/oauth/2.0/token"
    "?grant_type=client_credentials"
    "&client_id=%1"
    "&client_secret=%2";

const QString client_id     = "sYnF4bQLNMTgD2FBmpwdd7i7";
const QString client_secret = "RDUKa3CpZsBznQMjGb0U1DJ8YRKpESOB";

const QString baiduSpeechUrl =
    "http://vop.baidu.com/server_api"
    "?dev_pid=1537"
    "&cuid=%1"
    "&token=%2";

speechrecognition::speechrecognition(QObject *parent) : QObject(parent)
{
    accessToken = "";
}

QString speechrecognition::speechIdentify(QString filename, const QAudioFormat &format)
{
    const bool validPcmFormat =
        format.isValid() &&
        format.codec() == "audio/pcm" &&
        format.channelCount() == 1 &&
        format.sampleSize() == 16 &&
        format.sampleType() == QAudioFormat::SignedInt &&
        format.byteOrder() == QAudioFormat::LittleEndian;

    if (!validPcmFormat) {
        qDebug() << "Invalid PCM format for speech recognition";
        return "";
    }

    const int sampleRate = format.sampleRate() > 0 ? format.sampleRate() : 16000;
    const QString tokenUrl = QString(baiduTokenUrl).arg(client_id).arg(client_secret);

    QMap<QString, QString> tokenHeaders;
    tokenHeaders.insert(QString("Content-Type"),
                        QString("application/x-www-form-urlencoded"));

    QByteArray requestData;
    QByteArray replyData;

    httppost httpUtil;
    const bool success = httpUtil.postMsg(tokenUrl, tokenHeaders, requestData, replyData);
    if (success) {
        accessToken = getJsonvalue(replyData, QString("access_token"));
        if (accessToken.isEmpty()) {
            qDebug() << "Empty access_token";
            return "";
        }
    } else {
        qDebug() << "Failed to get access_token";
        return "";
    }

    const QString baiduSpeech =
        QString(baiduSpeechUrl).arg("home_client").arg(accessToken);

    QFile file;
    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open audio file:" << file.errorString();
        return "";
    }

    requestData = file.readAll();
    file.close();

    if (requestData.isEmpty()) {
        qDebug() << "Empty audio file";
        return "";
    }

    replyData.clear();

    QMap<QString, QString> speechHeaders;
    speechHeaders.insert(QString("Content-Type"),
                         QString("audio/pcm;rate=%1").arg(sampleRate));

    const bool result = httpUtil.postMsg(baiduSpeech, speechHeaders, requestData, replyData);
    if (result) {
        qDebug() << "Speech response:" << replyData;
        QString text = getJsonvalue(replyData, QString("result")).trimmed();
        text.replace(QRegularExpression("\\s+"), "");
        return text;
    }

    qDebug() << "Speech recognition failed";
    return "";
}

QString speechrecognition::getJsonvalue(QByteArray ba, QString key)
{
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(ba, &parseError);

    if (parseError.error == QJsonParseError::NoError && jsonDocument.isObject()) {
        const QJsonObject jsonObj = jsonDocument.object();
        if (jsonObj.contains(key)) {
            const QJsonValue jsonVal = jsonObj.value(key);
            if (jsonVal.isString()) {
                return jsonVal.toString();
            }
            if (jsonVal.isArray()) {
                const QJsonArray arr = jsonVal.toArray();
                if (!arr.isEmpty()) {
                    return arr.at(0).toString();
                }
            }
        }
    }

    return "";
}
