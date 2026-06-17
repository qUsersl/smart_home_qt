#ifndef PEOPLEPAGE_H
#define PEOPLEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定人体检测模块使用的 topic：0 = 云端，1 = 硬件
#define PEOPLE_MODE  1

// 指定闯入检测模块使用的 topic：0 = 云端，1 = 硬件
#define INTRUSION_MODE  0

// 人体检测器数量
#define PEOPLE_COUNT 1

// 闯入检测器数量
#define INTRUSION_COUNT 1

class PeopleRow : public QWidget
{
    Q_OBJECT
public:
    explicit PeopleRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setEnabledByAuto(bool on);   // 自动控制器调用，强制设为开启/关闭

signals:
    void userToggled(bool on, int id); // 用户手动切换

private slots:
    void onToggleClicked();

private:
    int          m_id;
    bool         m_state;       // 检测器开关状态
    bool         m_intruded;    // 当前是否检测到人体
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QLabel      *m_lblIntrude;
    QPushButton *m_btnToggle;

    void    publishState(bool on);
    void    updateIndicator();
    void    updateIntrudeLabel();
    QString pubTopic() const;
};


class IntrusionRow : public QWidget
{
    Q_OBJECT
public:
    explicit IntrusionRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setEnabledByAuto(bool on);

signals:
    void intrusionDetected(int id);
    void userToggled(bool on, int id);

private slots:
    void onToggleClicked();

private:
    int          m_id;
    bool         m_state;       // 闯入检测器开关
    bool         m_intruded;    // 当前是否有闯入
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QLabel      *m_lblStatus;
    QPushButton *m_btnToggle;

    void    publishState(bool on);
    void    updateIndicator();
    QString pubTopic() const;
};


class PeoplePage : public QWidget
{
    Q_OBJECT
public:
    explicit PeoplePage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);
    void setEnabledByAuto(bool on, int id = 0);
    void setIntrusionEnabledByAuto(bool on, int id = 0);

signals:
    void backRequested();
    void userToggled(bool on, int id);
    void userIntrusionToggled(bool on, int id);

private slots:
    void onIntrusionDetected(int id);

private:
    PeopleRow    *m_rows[PEOPLE_COUNT];
    IntrusionRow *m_intrusionRows[INTRUSION_COUNT];
};

#endif // PEOPLEPAGE_H
