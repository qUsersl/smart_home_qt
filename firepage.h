#ifndef FIREPAGE_H
#define FIREPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定火焰报警模块使用的 topic：0 = 云端，1 = 硬件
#define FIRE_MODE  1

// 火焰检测器数量
#define FIRE_COUNT 1

class FireRow : public QWidget
{
    Q_OBJECT
public:
    explicit FireRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setEnabledByAuto(bool on);

signals:
    void fireDetected(int id);
    void userToggled(bool on, int id);

private slots:
    void onToggleClicked();

private:
    int          m_id;
    bool         m_enabled;     // 火焰报警开关
    bool         m_fire;        // 当前是否检测到火焰
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QLabel      *m_lblStatus;
    QPushButton *m_btnToggle;

    void    publishState(bool on);
    void    updateIndicator();
    QString pubTopic() const;
};


class FirePage : public QWidget
{
    Q_OBJECT
public:
    explicit FirePage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setEnabledByAuto(bool on, int id = 0);

signals:
    void backRequested();
    void userToggled(bool on, int id);

private slots:
    void onFireDetected(int id);

private:
    FireRow *m_rows[FIRE_COUNT];
};

#endif // FIREPAGE_H
