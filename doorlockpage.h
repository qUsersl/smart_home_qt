#ifndef DOORLOCKPAGE_H
#define DOORLOCKPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定门锁模块使用的 topic：0 = 云端，1 = 硬件
#define DOORLOCK_MODE  0

// 门锁数量
#define DOORLOCK_COUNT 1

class DoorLockRow : public QWidget
{
    Q_OBJECT
public:
    explicit DoorLockRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
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


class DoorLockPage : public QWidget
{
    Q_OBJECT
public:
    explicit DoorLockPage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    DoorLockRow *m_rows[DOORLOCK_COUNT];
};

#endif // DOORLOCKPAGE_H
