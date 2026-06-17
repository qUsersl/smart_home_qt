#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <qmqttclient.h>
#include "mqttconfig.h"
#include "ledpage.h"
#include "fanpage.h"
#include "doorlockpage.h"
#include "temhumpage.h"
#include "soilpage.h"
#include "peoplepage.h"
#include "co2page.h"
#include "pm25page.h"
#include "sunshadepage.h"
#include "flamgaspage.h"
#include "firepage.h"
#include "alarmpage.h"
#include "autocontrol.h"
#include "voicepage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btnRefresh_clicked();
    void onConnected();
    void onDisconnected();
    void onMessageReceived(QByteArray data);
    void onConfigureThresholds();

private:
    void doConnect();
    void updateStatus(bool connected);

    Ui::Widget    *ui;
    QMqttClient   *mqtt;
    QStackedWidget *m_stack;
    LedPage        *m_ledPage;
    FanPage        *m_fanPage;
    DoorLockPage   *m_doorLockPage;
    TemHumPage     *m_temHumPage;
    SoilPage       *m_soilPage;
    PeoplePage     *m_peoplePage;
    Co2Page        *m_co2Page;
    Pm25Page       *m_pm25Page;
    SunshadePage   *m_sunshadePage;
    FlamGasPage    *m_flamGasPage;
    FirePage       *m_firePage;
    AlarmPage      *m_alarmPage;
    AutoController *m_autoCtl;
    VoicePage      *m_voicePage;
    class QPushButton *m_btnAuto;
    class QLabel      *m_lblAutoStatus;
};

#endif // WIDGET_H
