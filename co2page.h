#ifndef CO2PAGE_H
#define CO2PAGE_H

#include <QWidget>
#include <QLabel>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定 CO2 模块使用的 topic：0 = 云端，1 = 硬件
#define CO2_MODE  0

// 传感器数量
#define CO2_COUNT 1

class Co2Row : public QWidget
{
    Q_OBJECT
public:
    explicit Co2Row(int id, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private:
    int     m_id;
    QLabel *m_lblCo2;
};


class Co2Page : public QWidget
{
    Q_OBJECT
public:
    explicit Co2Page(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    Co2Row *m_rows[CO2_COUNT];
};

#endif // CO2PAGE_H
