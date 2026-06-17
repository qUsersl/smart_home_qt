#include "autocontrol.h"
#include "fanpage.h"
#include "peoplepage.h"
#include "firepage.h"
#include "ledpage.h"

AutoController::AutoController(QMqttClient *mqtt,
                               FanPage     *fanPage,
                               PeoplePage  *peoplePage,
                               FirePage    *firePage,
                               LedPage     *ledPage,
                               QObject     *parent)
    : QObject(parent)
    , m_mqtt(mqtt)
    , m_fanPage(fanPage)
    , m_peoplePage(peoplePage)
    , m_firePage(firePage)
    , m_ledPage(ledPage)
    , m_enabled(false)
    , m_fanIntent(-1)
    , m_ledIntent(-1)
    , m_tempHigh(AUTO_DEFAULT_TEMP_HIGH)
    , m_tempLow(AUTO_DEFAULT_TEMP_LOW)
    , m_lightHigh(AUTO_DEFAULT_LIGHT_HIGH)
    , m_lightLow(AUTO_DEFAULT_LIGHT_LOW)
{
    Q_UNUSED(m_mqtt)
}

void AutoController::setEnabled(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    m_fanIntent = -1;
    m_ledIntent = -1;

    if (on) {
        // 自动模式打开 → 立即开启人体检测、闯入检测和火焰报警
        m_peoplePage->setEnabledByAuto(true);
        m_peoplePage->setIntrusionEnabledByAuto(true);
        m_firePage->setEnabledByAuto(true);
        // 风扇和灯光的状态依赖下一次温度/光照上报后再决定
    }

    emit enabledChanged(on);
}

void AutoController::setThresholds(double tempHigh, double tempLow,
                                   double lightHigh, double lightLow)
{
    m_tempHigh  = tempHigh;
    m_tempLow   = tempLow;
    m_lightHigh = lightHigh;
    m_lightLow  = lightLow;
    // 阈值变化后重新评估
    m_fanIntent = -1;
    m_ledIntent = -1;
    emit thresholdsChanged();
}

void AutoController::onTemperatureChanged(double tem, int id)
{
    if (!m_enabled) return;

    if (tem > m_tempHigh && m_fanIntent != 1) {
        m_fanIntent = 1;
        m_fanPage->setLevelByAuto(AUTO_FAN_LEVEL, id);
    } else if (tem < m_tempLow && m_fanIntent != 0) {
        m_fanIntent = 0;
        m_fanPage->setLevelByAuto(0, id);
    }
}

void AutoController::onLightChanged(double lux, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;

    if (lux < m_lightLow && m_ledIntent != 1) {
        m_ledIntent = 1;
        m_ledPage->setStateByAuto(true);
    } else if (lux > m_lightHigh && m_ledIntent != 0) {
        m_ledIntent = 0;
        m_ledPage->setStateByAuto(false);
    }
}

void AutoController::onUserFanToggled(int level, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;

    bool userOn = (level > 0);
    bool conflict = (m_fanIntent == 1 && !userOn) || (m_fanIntent == 0 && userOn);
    if (conflict)
        setEnabled(false);
}

void AutoController::onUserLedToggled(bool on, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;

    bool conflict = (m_ledIntent == 1 && !on) || (m_ledIntent == 0 && on);
    if (conflict)
        setEnabled(false);
}

void AutoController::onUserPeopleToggled(bool on, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;
    if (!on)
        setEnabled(false);
}

void AutoController::onUserIntrusionToggled(bool on, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;
    // 自动模式期望闯入检测一直开启，用户关闭即冲突
    if (!on)
        setEnabled(false);
}

void AutoController::onUserFireToggled(bool on, int id)
{
    Q_UNUSED(id)
    if (!m_enabled) return;
    if (!on)
        setEnabled(false);
}
