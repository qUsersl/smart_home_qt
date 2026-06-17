#ifndef FLAMGASPAGE_H
#define FLAMGASPAGE_H

#include <QWidget>
#include <QLabel>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定可燃气体模块使用的 topic：0 = 云端，1 = 硬件
#define FLAMGAS_MODE  0

// 传感器数量
#define FLAMGAS_COUNT 1

class FlamGasRow : public QWidget
{
    Q_OBJECT
public:
    explicit FlamGasRow(int id, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private:
    int     m_id;
    bool    m_detected;
    QLabel *m_lblIndicator;
    QLabel *m_lblStatus;

    void updateIndicator();
};


class FlamGasPage : public QWidget
{
    Q_OBJECT
public:
    explicit FlamGasPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    FlamGasRow *m_rows[FLAMGAS_COUNT];
};

#endif // FLAMGASPAGE_H
