#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定 LED 模块使用的 topic：0 = 云端，1 = 硬件
#define LED_MODE  0

class LedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LedWidget(int id, QMqttClient *mqtt, QWidget *parent = nullptr);

public slots:
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

#endif // LEDWIDGET_H
