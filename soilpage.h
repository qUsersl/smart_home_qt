#ifndef SOILPAGE_H
#define SOILPAGE_H

#include <QWidget>
#include <QLabel>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定土壤温湿度模块使用的 topic：0 = 云端，1 = 硬件
#define SOIL_MODE  0

// 传感器数量
#define SOIL_COUNT 1

class SoilRow : public QWidget
{
    Q_OBJECT
public:
    explicit SoilRow(int id, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private:
    int     m_id;
    QLabel *m_lblTem;
    QLabel *m_lblHum;
};


class SoilPage : public QWidget
{
    Q_OBJECT
public:
    explicit SoilPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    SoilRow *m_rows[SOIL_COUNT];
};

#endif // SOILPAGE_H
