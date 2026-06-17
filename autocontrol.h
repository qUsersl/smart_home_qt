#ifndef AUTOCONTROL_H
#define AUTOCONTROL_H

#include <QObject>
#include <qmqttclient.h>

// 默认阈值
#define AUTO_DEFAULT_TEMP_HIGH  28.0
#define AUTO_DEFAULT_TEMP_LOW   22.0
#define AUTO_DEFAULT_LIGHT_HIGH 400.0   // 高于此值光线充足，关灯
#define AUTO_DEFAULT_LIGHT_LOW  150.0   // 低于此值光线不足，开灯
// 自动开启风扇时使用的档位
#define AUTO_FAN_LEVEL 1

class FanPage;
class PeoplePage;
class FirePage;
class LedPage;

class AutoController : public QObject
{
    Q_OBJECT
public:
    explicit AutoController(QMqttClient *mqtt,
                            FanPage     *fanPage,
                            PeoplePage  *peoplePage,
                            FirePage    *firePage,
                            LedPage     *ledPage,
                            QObject     *parent = nullptr);

    bool   isEnabled()  const { return m_enabled; }
    double tempHigh()   const { return m_tempHigh; }
    double tempLow()    const { return m_tempLow; }
    double lightHigh()  const { return m_lightHigh; }
    double lightLow()   const { return m_lightLow; }

public slots:
    void setEnabled(bool on);
    void setThresholds(double tempHigh, double tempLow,
                       double lightHigh, double lightLow);
    void onTemperatureChanged(double tem, int id);
    void onLightChanged(double lux, int id);
    void onUserFanToggled(int level, int id);
    void onUserLedToggled(bool on, int id);
    void onUserPeopleToggled(bool on, int id);
    void onUserIntrusionToggled(bool on, int id);
    void onUserFireToggled(bool on, int id);

signals:
    void enabledChanged(bool on);
    void thresholdsChanged();

private:
    QMqttClient *m_mqtt;
    FanPage     *m_fanPage;
    PeoplePage  *m_peoplePage;
    FirePage    *m_firePage;
    LedPage     *m_ledPage;

    bool   m_enabled;
    int    m_fanIntent;   // -1 = 未决定, 0 = 关, 1 = 开
    int    m_ledIntent;   // -1 = 未决定, 0 = 关, 1 = 开

    double m_tempHigh;
    double m_tempLow;
    double m_lightHigh;
    double m_lightLow;
};

#endif // AUTOCONTROL_H
