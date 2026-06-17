#ifndef SUNSHADEPAGE_H
#define SUNSHADEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <qmqttclient.h>
#include "mqttconfig.h"

// 指定遮阳板模块使用的 topic：0 = 云端，1 = 硬件
#define SUNSHADE_MODE  0

// 遮阳板数量
#define SUNSHADE_COUNT 1

class SunshadeRow : public QWidget
{
    Q_OBJECT
public:
    explicit SunshadeRow(int id, QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

private slots:
    void onForward();
    void onReverse();
    void onStop();

private:
    int          m_id;
    QString      m_state;     // "forward" / "reverse" / "stop"
    QMqttClient *m_mqtt;
    QLabel      *m_lblIndicator;
    QLabel      *m_lblStatus;
    QPushButton *m_btnForward;
    QPushButton *m_btnReverse;
    QPushButton *m_btnStop;

    void    publishState(const QString &action);
    void    updateIndicator();
    QString pubTopic() const;
};


class SunshadePage : public QWidget
{
    Q_OBJECT
public:
    explicit SunshadePage(QMqttClient *mqtt, QWidget *parent = nullptr);
    void onMessageReceived(QByteArray data);

signals:
    void backRequested();

private:
    SunshadeRow *m_rows[SUNSHADE_COUNT];
};

#endif // SUNSHADEPAGE_H
