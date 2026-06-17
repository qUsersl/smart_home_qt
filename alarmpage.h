#ifndef ALARMPAGE_H
#define ALARMPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定报警器模块使用的 topic：0 = 云端，1 = 硬件
#define ALARM_MODE  1

// 报警器数量
#define ALARM_COUNT 1

class AlarmRow : public QWidget
{
    Q_OBJECT
public:
    explicit AlarmRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private slots:
    void onToggleClicked();

private:
    int          m_id;
    bool         m_state;
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QPushButton *m_btnToggle;

    void    publishState(bool on);
    void    updateIndicator();
    QString pubTopic() const;
};


class AlarmPage : public QWidget
{
    Q_OBJECT
public:
    explicit AlarmPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    AlarmRow *m_rows[ALARM_COUNT];
};

#endif // ALARMPAGE_H
