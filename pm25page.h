#ifndef PM25PAGE_H
#define PM25PAGE_H

#include <QWidget>
#include <QLabel>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定 PM2.5 模块使用的 topic：0 = 云端，1 = 硬件
#define PM25_MODE  0

// 传感器数量
#define PM25_COUNT 1

class Pm25Row : public QWidget
{
    Q_OBJECT
public:
    explicit Pm25Row(int id, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private:
    int     m_id;
    QLabel *m_lblPm25;
};


class Pm25Page : public QWidget
{
    Q_OBJECT
public:
    explicit Pm25Page(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    Pm25Row *m_rows[PM25_COUNT];
};

#endif // PM25PAGE_H
