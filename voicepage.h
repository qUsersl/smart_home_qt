#ifndef VOICEPAGE_H
#define VOICEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <qmqttclient.h>
#include "mqttconfig.h"
#include "audiocapture.h"
#include "speechrecognition.h"

// 各模块发布 topic 的模式（与对应页面保持一致，0=云端, 1=硬件）
#define VOICE_DOORLOCK_MODE  0
#define VOICE_LED_MODE       1
#define VOICE_FAN_MODE       1
#define VOICE_ALARM_MODE     1

class VoicePage : public QWidget
{
    Q_OBJECT
public:
    explicit VoicePage(QMqttClient *mqtt, QWidget *parent = nullptr);

signals:
    void backRequested();

private slots:
    void onRecordPressed();
    void onRecordReleased();

private:
    QMqttClient       *m_mqtt;
    AudioCapture       m_capture;
    speechrecognition  m_recognizer;
    QString            m_filepath;

    QPushButton *m_btnRecord;
    QLabel      *m_lblHint;
    QLabel      *m_lblText;
    QTextEdit   *m_log;

    void    appendLog(const QString &line);
    bool    handleCommand(const QString &text);

    // 各模块独立的 pub topic
    QString doorLockTopic() const;
    QString ledTopic()      const;
    QString fanTopic()      const;
    QString alarmTopic()    const;

    void    publishDoorLock(bool open);
    void    publishLed(int id, bool on);   // id<0 表示全部
    void    publishFan(int level);         // 0 = 关
    void    publishAlarm(bool on);

    // 工具：从文本里识别灯编号 (1/2/3)，没有 → -1
    int     parseLedIndex(const QString &text) const;
};

#endif // VOICEPAGE_H
