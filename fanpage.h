#ifndef FANPAGE_H
#define FANPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定风扇模块使用的 topic：0 = 云端，1 = 硬件
#define FAN_MODE  1

// 风扇数量
#define FAN_COUNT 1

class FanRow : public QWidget
{
    Q_OBJECT
public:
    explicit FanRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setLevelByAuto(int level);

signals:
    void userLevelChanged(int level, int id);   // 用户点击按钮触发

private slots:
    void onLevelClicked(int level); // 0 = 关，1/2/3 = 档位

private:
    int          m_id;
    int          m_level;   // 0 表示关闭
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QLabel      *m_lblLevel;
    QPushButton *m_btnOff;
    QPushButton *m_btnL1;
    QPushButton *m_btnL2;
    QPushButton *m_btnL3;

    void    publishState(int level);
    void    updateIndicator();
    QString pubTopic() const;
};


class FanPage : public QWidget
{
    Q_OBJECT
public:
    explicit FanPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setLevelByAuto(int level, int id = 0);

signals:
    void backRequested();
    void userLevelChanged(int level, int id);

private:
    FanRow *m_rows[FAN_COUNT];
};

#endif // FANPAGE_H
