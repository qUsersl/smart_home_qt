#ifndef LEDPAGE_H
#define LEDPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定 LED 页面使用的 topic：0 = 云端，1 = 硬件
#define LED_MODE  1

// LED 数量
#define LED_COUNT 3

class LedRow : public QWidget
{
    Q_OBJECT
public:
    explicit LedRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setStateByAuto(bool on);

signals:
    void userToggled(bool on, int id);

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


class LedPage : public QWidget
{
    Q_OBJECT
public:
    explicit LedPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setStateByAuto(bool on);   // 自动控制：开关全部 LED

signals:
    void backRequested();
    void userToggled(bool on, int id);

private:
    LedRow *m_rows[LED_COUNT];
};

#endif // LEDPAGE_H
