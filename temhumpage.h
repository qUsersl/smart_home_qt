#ifndef TEMHUMPAGE_H
#define TEMHUMPAGE_H

#include <QWidget>
#include <QLabel>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定温湿度模块使用的 topic：0 = 云端，1 = 硬件
#define TEMHUM_MODE  1

// 传感器数量
#define TEMHUM_COUNT 1

class TemHumRow : public QWidget
{
    Q_OBJECT
public:
    explicit TemHumRow(int id, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void temperatureChanged(double tem, int id);
    void lightChanged(double lux, int id);

private:
    int     m_id;
    QLabel *m_lblTem;
    QLabel *m_lblHum;
    QLabel *m_lblLight;
};


class TemHumPage : public QWidget
{
    Q_OBJECT
public:
    explicit TemHumPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();
    void temperatureChanged(double tem, int id);
    void lightChanged(double lux, int id);

private:
    TemHumRow *m_rows[TEMHUM_COUNT];
};

#endif // TEMHUMPAGE_H
