#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

// 创建一个"卡片式"功能按钮：emoji 图标 + 名称
QPushButton *makeCardButton(const QString &icon, const QString &name,
                            const QString &kind, QWidget *parent)
{
    QPushButton *btn = new QPushButton(parent);
    btn->setText(QString("%1   %2").arg(icon, name));
    btn->setMinimumHeight(72);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("kind", kind); // "hw" 或 "cloud"，用于 QSS 选择器
    QFont f = btn->font();
    f.setPointSize(13);
    btn->setFont(f);
    return btn;
}

// 分组标题：左侧色条 + 文字
QWidget *makeSectionHeader(const QString &title, const QString &accent,
                           const QString &subtitle, QWidget *parent)
{
    QWidget *box = new QWidget(parent);
    QHBoxLayout *h = new QHBoxLayout(box);
    h->setContentsMargins(0, 8, 0, 8);
    h->setSpacing(10);

    QFrame *bar = new QFrame(box);
    bar->setFixedSize(4, 22);
    bar->setStyleSheet(QString("background:%1; border-radius:2px;").arg(accent));

    QLabel *lblTitle = new QLabel(title, box);
    QFont f = lblTitle->font();
    f.setPointSize(13);
    f.setBold(true);
    lblTitle->setFont(f);
    lblTitle->setStyleSheet("color:#222;");

    QLabel *lblSub = new QLabel(subtitle, box);
    lblSub->setStyleSheet("color:#888; font-size:11px;");

    h->addWidget(bar);
    h->addWidget(lblTitle);
    h->addWidget(lblSub);
    h->addStretch();
    return box;
}

} // namespace

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // ── 全局 QSS 主题 ──
    this->setStyleSheet(R"(
        QWidget#Widget { background: #f4f6fa; }
        QLabel#lblStatus { font-size: 13px; padding: 4px 10px; border-radius: 10px; background: #ffffff; }
        QPushButton#btnRefresh {
            background: #ffffff; border: 1px solid #d0d4dc; border-radius: 6px;
            padding: 4px 14px; color: #333;
        }
        QPushButton#btnRefresh:hover  { background: #eef1f7; }
        QPushButton#btnRefresh:pressed{ background: #e2e6ee; }

        /* 设备端按钮：蓝色调 */
        QPushButton[kind="hw"] {
            background: #ffffff; border: 1px solid #dfe3eb;
            border-radius: 10px; padding: 8px 14px; text-align: left;
            color: #20324a;
        }
        QPushButton[kind="hw"]:hover {
            background: #eaf2ff; border-color: #4a90e2; color: #1a4d8a;
        }
        QPushButton[kind="hw"]:pressed { background: #d8e6fb; }

        /* 仅云端按钮：浅紫调 */
        QPushButton[kind="cloud"] {
            background: #ffffff; border: 1px solid #dfe3eb;
            border-radius: 10px; padding: 8px 14px; text-align: left;
            color: #3a2d4f;
        }
        QPushButton[kind="cloud"]:hover {
            background: #f1ecfb; border-color: #8a6cd1; color: #4a3585;
        }
        QPushButton[kind="cloud"]:pressed { background: #e3d9f4; }
    )");

    mqtt = new QMqttClient(this);
    connect(mqtt, &QMqttClient::connected,       this, &Widget::onConnected);
    connect(mqtt, &QMqttClient::disconnected,    this, &Widget::onDisconnected);
    connect(mqtt, &QMqttClient::messageReceived, this, &Widget::onMessageReceived);

    // ── 构建 QStackedWidget ──────────────────────────────
    m_stack = new QStackedWidget(ui->contentArea);
    QVBoxLayout *areaLayout = new QVBoxLayout(ui->contentArea);
    areaLayout->setContentsMargins(0, 0, 0, 0);
    areaLayout->addWidget(m_stack);

    // page 0：主页（卡片网格）
    QWidget *homePage = new QWidget;
    homePage->setStyleSheet("background: transparent;");

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(homePage);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{background:transparent;} QScrollArea>QWidget>QWidget{background:transparent;}");

    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setAlignment(Qt::AlignTop);
    homeLayout->setSpacing(8);
    homeLayout->setContentsMargins(20, 16, 20, 20);

    // 顶部欢迎标题
    QLabel *welcome = new QLabel("智能家居控制台", homePage);
    {
        QFont f = welcome->font();
        f.setPointSize(18);
        f.setBold(true);
        welcome->setFont(f);
        welcome->setStyleSheet("color:#1a2540;");
    }
    homeLayout->addWidget(welcome);

    QLabel *welcomeSub = new QLabel("选择一个模块开始控制或查看", homePage);
    welcomeSub->setStyleSheet("color:#7a849a; font-size:12px;");
    homeLayout->addWidget(welcomeSub);
    homeLayout->addSpacing(6);

    // ── 设备端模块（卡片网格）──
    homeLayout->addWidget(makeSectionHeader("设备端", "#4a90e2", "实际硬件可触达", homePage));

    QGridLayout *gridHw = new QGridLayout;
    gridHw->setSpacing(12);
    gridHw->setContentsMargins(0, 0, 0, 0);

    QPushButton *btnLed    = makeCardButton("💡", "灯光",       "hw", homePage);
    QPushButton *btnFan    = makeCardButton("🌀", "风扇",       "hw", homePage);
    QPushButton *btnTemHum = makeCardButton("🌡", "温湿度光照", "hw", homePage);
    QPushButton *btnAlarm  = makeCardButton("🔔", "报警器",     "hw", homePage);
    QPushButton *btnPeople = makeCardButton("🚶", "人体检测",   "hw", homePage);
    QPushButton *btnFire   = makeCardButton("🔥", "火焰报警",   "hw", homePage);

    gridHw->addWidget(btnLed,    0, 0);
    gridHw->addWidget(btnFan,    0, 1);
    gridHw->addWidget(btnTemHum, 0, 2);
    gridHw->addWidget(btnAlarm,  1, 0);
    gridHw->addWidget(btnPeople, 1, 1);
    gridHw->addWidget(btnFire,   1, 2);
    homeLayout->addLayout(gridHw);

    homeLayout->addSpacing(8);

    // ── 仅云端模块（卡片网格）──
    homeLayout->addWidget(makeSectionHeader("仅云端", "#8a6cd1", "无对应硬件", homePage));

    QGridLayout *gridCloud = new QGridLayout;
    gridCloud->setSpacing(12);
    gridCloud->setContentsMargins(0, 0, 0, 0);

    QPushButton *btnDoorLock = makeCardButton("🔒", "门锁",       "cloud", homePage);
    QPushButton *btnSoil     = makeCardButton("🌱", "土壤温湿度", "cloud", homePage);
    QPushButton *btnCo2      = makeCardButton("🫧", "二氧化碳",   "cloud", homePage);
    QPushButton *btnPm25     = makeCardButton("🌫", "PM2.5",      "cloud", homePage);
    QPushButton *btnSunshade = makeCardButton("🪟", "遮阳板",     "cloud", homePage);
    QPushButton *btnFlamGas  = makeCardButton("🛢", "可燃气体",   "cloud", homePage);

    gridCloud->addWidget(btnDoorLock, 0, 0);
    gridCloud->addWidget(btnSoil,     0, 1);
    gridCloud->addWidget(btnCo2,      0, 2);
    gridCloud->addWidget(btnPm25,     1, 0);
    gridCloud->addWidget(btnSunshade, 1, 1);
    gridCloud->addWidget(btnFlamGas,  1, 2);
    homeLayout->addLayout(gridCloud);

    homeLayout->addSpacing(12);

    // ── 自动控制开关卡片 ──
    QWidget *autoCard = new QWidget(homePage);
    autoCard->setStyleSheet(
        "background:#fff; border:1px solid #dfe3eb; border-radius:10px;");
    QHBoxLayout *autoLay = new QHBoxLayout(autoCard);
    autoLay->setContentsMargins(16, 12, 16, 12);
    autoLay->setSpacing(10);

    QLabel *lblAutoIcon = new QLabel("⚙", autoCard);
    {
        QFont f = lblAutoIcon->font();
        f.setPointSize(18);
        lblAutoIcon->setFont(f);
        lblAutoIcon->setStyleSheet("border:none; color:#4a90e2;");
    }
    QLabel *lblAutoTitle = new QLabel("自动控制", autoCard);
    {
        QFont f = lblAutoTitle->font();
        f.setPointSize(13);
        f.setBold(true);
        lblAutoTitle->setFont(f);
        lblAutoTitle->setStyleSheet("border:none; color:#20324a;");
    }
    m_lblAutoStatus = new QLabel("已关闭", autoCard);
    m_lblAutoStatus->setStyleSheet("border:none; color:#888; font-size:12px;");

    m_btnAuto = new QPushButton("开启自动", autoCard);
    m_btnAuto->setMinimumWidth(96);
    m_btnAuto->setCursor(Qt::PointingHandCursor);
    m_btnAuto->setStyleSheet(
        "QPushButton{background:#4a90e2; color:#fff; border:none; border-radius:8px; padding:8px 14px;}"
        "QPushButton:hover{background:#3a7fcc;}"
        "QPushButton:pressed{background:#2f6db3;}");

    QPushButton *btnAutoCfg = new QPushButton("阈值设置", autoCard);
    btnAutoCfg->setCursor(Qt::PointingHandCursor);
    btnAutoCfg->setStyleSheet(
        "QPushButton{background:#fff; color:#4a90e2; border:1px solid #4a90e2; border-radius:8px; padding:8px 14px;}"
        "QPushButton:hover{background:#eaf2ff;}"
        "QPushButton:pressed{background:#d8e6fb;}");

    autoLay->addWidget(lblAutoIcon);
    autoLay->addWidget(lblAutoTitle);
    autoLay->addSpacing(8);
    autoLay->addWidget(m_lblAutoStatus);
    autoLay->addStretch();
    autoLay->addWidget(btnAutoCfg);
    autoLay->addWidget(m_btnAuto);

    homeLayout->addWidget(autoCard);

    homeLayout->addStretch();

    m_stack->addWidget(scroll);   // index 0

    // page 1：LED 控制页
    m_ledPage = new LedPage(mqtt);
    m_stack->addWidget(m_ledPage);  // index 1

    // page 2：风扇控制页
    m_fanPage = new FanPage(mqtt);
    m_stack->addWidget(m_fanPage);  // index 2

    // page 3：门锁控制页
    m_doorLockPage = new DoorLockPage(mqtt);
    m_stack->addWidget(m_doorLockPage);  // index 3

    // page 4：温湿度监测页
    m_temHumPage = new TemHumPage(mqtt);
    m_stack->addWidget(m_temHumPage);  // index 4

    // page 5：报警器控制页
    m_alarmPage = new AlarmPage(mqtt);
    m_stack->addWidget(m_alarmPage);  // index 5

    // page 6：土壤温湿度页
    m_soilPage = new SoilPage(mqtt);
    m_stack->addWidget(m_soilPage);   // index 6

    // page 7：人体检测页
    m_peoplePage = new PeoplePage(mqtt);
    m_stack->addWidget(m_peoplePage); // index 7

    // page 8：二氧化碳页
    m_co2Page = new Co2Page(mqtt);
    m_stack->addWidget(m_co2Page);    // index 8

    // page 9：火焰报警页
    m_firePage = new FirePage(mqtt);
    m_stack->addWidget(m_firePage);   // index 9

    // page 10：PM2.5 页
    m_pm25Page = new Pm25Page(mqtt);
    m_stack->addWidget(m_pm25Page);   // index 10

    // page 11：遮阳板页
    m_sunshadePage = new SunshadePage(mqtt);
    m_stack->addWidget(m_sunshadePage); // index 11

    // page 12：可燃气体页
    m_flamGasPage = new FlamGasPage(mqtt);
    m_stack->addWidget(m_flamGasPage);  // index 12

    connect(btnLed,         &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(1); });
    connect(m_ledPage,      &LedPage::backRequested,      this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnFan,         &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(2); });
    connect(m_fanPage,      &FanPage::backRequested,      this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnDoorLock,    &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(3); });
    connect(m_doorLockPage, &DoorLockPage::backRequested, this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnTemHum,      &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(4); });
    connect(m_temHumPage,   &TemHumPage::backRequested,   this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnAlarm,       &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(5); });
    connect(m_alarmPage,    &AlarmPage::backRequested,    this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnSoil,        &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(6); });
    connect(m_soilPage,     &SoilPage::backRequested,     this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnPeople,      &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(7); });
    connect(m_peoplePage,   &PeoplePage::backRequested,   this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnCo2,         &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(8); });
    connect(m_co2Page,      &Co2Page::backRequested,      this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnFire,        &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(9); });
    connect(m_firePage,     &FirePage::backRequested,     this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnPm25,        &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(10); });
    connect(m_pm25Page,     &Pm25Page::backRequested,     this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnSunshade,    &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(11); });
    connect(m_sunshadePage, &SunshadePage::backRequested, this, [this]{ m_stack->setCurrentIndex(0); });
    connect(btnFlamGas,     &QPushButton::clicked,        this, [this]{ m_stack->setCurrentIndex(12); });
    connect(m_flamGasPage,  &FlamGasPage::backRequested,  this, [this]{ m_stack->setCurrentIndex(0); });
    // ─────────────────────────────────────────────────────

    // ── 自动控制器接线 ──
    m_autoCtl = new AutoController(mqtt, m_fanPage, m_peoplePage, m_firePage, m_ledPage, this);

    connect(m_temHumPage, &TemHumPage::temperatureChanged,
            m_autoCtl,    &AutoController::onTemperatureChanged);
    connect(m_temHumPage, &TemHumPage::lightChanged,
            m_autoCtl,    &AutoController::onLightChanged);
    connect(m_fanPage,    &FanPage::userLevelChanged,
            m_autoCtl,    &AutoController::onUserFanToggled);
    connect(m_ledPage,    &LedPage::userToggled,
            m_autoCtl,    &AutoController::onUserLedToggled);
    connect(m_peoplePage, &PeoplePage::userToggled,
            m_autoCtl,    &AutoController::onUserPeopleToggled);
    connect(m_peoplePage, &PeoplePage::userIntrusionToggled,
            m_autoCtl,    &AutoController::onUserIntrusionToggled);
    connect(m_firePage,   &FirePage::userToggled,
            m_autoCtl,    &AutoController::onUserFireToggled);

    connect(m_btnAuto, &QPushButton::clicked, this, [this]{
        m_autoCtl->setEnabled(!m_autoCtl->isEnabled());
    });
    connect(btnAutoCfg, &QPushButton::clicked, this, &Widget::onConfigureThresholds);

    auto refreshAutoLabel = [this]{
        if (m_autoCtl->isEnabled()) {
            m_lblAutoStatus->setText(QString("已开启 · 温>%1℃开风扇<%2℃关 · 光<%3lx开灯>%4关")
                                         .arg(m_autoCtl->tempHigh(),  0, 'f', 1)
                                         .arg(m_autoCtl->tempLow(),   0, 'f', 1)
                                         .arg(m_autoCtl->lightLow(),  0, 'f', 0)
                                         .arg(m_autoCtl->lightHigh(), 0, 'f', 0));
            m_lblAutoStatus->setStyleSheet("border:none; color:#1f8a4c; font-size:12px;");
            m_btnAuto->setText("关闭自动");
            m_btnAuto->setStyleSheet(
                "QPushButton{background:#e74c3c; color:#fff; border:none; border-radius:8px; padding:8px 14px;}"
                "QPushButton:hover{background:#cf3f33;}"
                "QPushButton:pressed{background:#b2362c;}");
        } else {
            m_lblAutoStatus->setText("已关闭");
            m_lblAutoStatus->setStyleSheet("border:none; color:#888; font-size:12px;");
            m_btnAuto->setText("开启自动");
            m_btnAuto->setStyleSheet(
                "QPushButton{background:#4a90e2; color:#fff; border:none; border-radius:8px; padding:8px 14px;}"
                "QPushButton:hover{background:#3a7fcc;}"
                "QPushButton:pressed{background:#2f6db3;}");
        }
    };
    connect(m_autoCtl, &AutoController::enabledChanged,    this, refreshAutoLabel);
    connect(m_autoCtl, &AutoController::thresholdsChanged, this, refreshAutoLabel);

    doConnect();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::doConnect()
{
    if (mqtt->state() != QMqttClient::Disconnected)
        return;

    ui->lblStatus->setText("● 连接中...");
    ui->lblStatus->setStyleSheet("font-size:13px; padding:4px 10px; border-radius:10px;"
                                 "background:#fff5e0; color:#c87800; font-weight:bold;");

    mqtt->setHostname(MQTT_BROKER_IP);
    mqtt->setPort(MQTT_BROKER_PORT);
    mqtt->setProtocolVersion(QMqttClient::MQTT_3_1_1);
    mqtt->setClientId(MQTT_CLIENT_ID);
    mqtt->connectToHost();
}

void Widget::updateStatus(bool connected)
{
    if (connected) {
        ui->lblStatus->setText("● 已连接");
        ui->lblStatus->setStyleSheet("font-size:13px; padding:4px 10px; border-radius:10px;"
                                     "background:#e6f7ec; color:#1f8a4c; font-weight:bold;");
    } else {
        ui->lblStatus->setText("● 未连接");
        ui->lblStatus->setStyleSheet("font-size:13px; padding:4px 10px; border-radius:10px;"
                                     "background:#f0f0f3; color:#888; font-weight:bold;");
    }
}

void Widget::onConnected()
{
    updateStatus(true);
    mqtt->subscribe(QMqttTopicFilter(MQTT_CLOUD_SUB));
    mqtt->subscribe(QMqttTopicFilter(MQTT_HW_SUB));
}

void Widget::onDisconnected()
{
    updateStatus(false);
}

void Widget::on_btnRefresh_clicked()
{
    if (mqtt->state() == QMqttClient::Connected)
        return;
    mqtt->disconnectFromHost();
    doConnect();
}

void Widget::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    QString device = obj.value("device").toString();

    if (device == "light")
        m_ledPage->onMessageReceived(data);
    else if (device == "fan")
        m_fanPage->onMessageReceived(data);
    else if (device == "alarm")
        m_alarmPage->onMessageReceived(data);
    else if (device == "people_detect")
        m_peoplePage->onMessageReceived(data);
    else if (device == "people")
        m_peoplePage->onMessageReceived(data);
    else if (device == "intrusion_alarm" || device == "intrusion")
        m_peoplePage->onMessageReceived(data);
    else if (device == "fire" || device == "fire_alarm")
        m_firePage->onMessageReceived(data);
    else if (device == "temp" || device == "humi" || device == "light_sensor")
        m_temHumPage->onMessageReceived(data);
    else if (obj.contains("doorLock"))
        m_doorLockPage->onMessageReceived(data);
    else if (obj.contains("tem"))
        m_temHumPage->onMessageReceived(data);
    else if (obj.contains("soiltem"))
        m_soilPage->onMessageReceived(data);
    else if (obj.contains("co2"))
        m_co2Page->onMessageReceived(data);
    else if (obj.contains("PM2.5"))
        m_pm25Page->onMessageReceived(data);
    else if (obj.contains("sunshade"))
        m_sunshadePage->onMessageReceived(data);
    else if (obj.contains("flamGas"))
        m_flamGasPage->onMessageReceived(data);
}

void Widget::onConfigureThresholds()
{
    QDialog dlg(this);
    dlg.setWindowTitle("自动控制 · 阈值设置");
    dlg.setStyleSheet("QDialog{background:#f4f6fa;}"
                      "QLabel{color:#20324a;}"
                      "QDoubleSpinBox{background:#fff; border:1px solid #dfe3eb; border-radius:6px; padding:4px 6px;}");

    QFormLayout *form = new QFormLayout(&dlg);
    form->setSpacing(10);
    form->setContentsMargins(20, 20, 20, 12);

    QDoubleSpinBox *spTempHigh = new QDoubleSpinBox(&dlg);
    spTempHigh->setRange(-20.0, 80.0);
    spTempHigh->setSuffix(" ℃");
    spTempHigh->setDecimals(1);
    spTempHigh->setSingleStep(0.5);
    spTempHigh->setValue(m_autoCtl->tempHigh());

    QDoubleSpinBox *spTempLow = new QDoubleSpinBox(&dlg);
    spTempLow->setRange(-20.0, 80.0);
    spTempLow->setSuffix(" ℃");
    spTempLow->setDecimals(1);
    spTempLow->setSingleStep(0.5);
    spTempLow->setValue(m_autoCtl->tempLow());

    QDoubleSpinBox *spLightHigh = new QDoubleSpinBox(&dlg);
    spLightHigh->setRange(0.0, 100000.0);
    spLightHigh->setSuffix(" lx");
    spLightHigh->setDecimals(0);
    spLightHigh->setSingleStep(10.0);
    spLightHigh->setValue(m_autoCtl->lightHigh());

    QDoubleSpinBox *spLightLow = new QDoubleSpinBox(&dlg);
    spLightLow->setRange(0.0, 100000.0);
    spLightLow->setSuffix(" lx");
    spLightLow->setDecimals(0);
    spLightLow->setSingleStep(10.0);
    spLightLow->setValue(m_autoCtl->lightLow());

    QLabel *hint = new QLabel("温度高于上限自动开风扇，低于下限自动关闭。\n"
                              "光照低于下限自动开灯，高于上限自动关灯。", &dlg);
    hint->setStyleSheet("color:#7a849a; font-size:12px;");

    form->addRow("温度上限", spTempHigh);
    form->addRow("温度下限", spTempLow);
    form->addRow("光照上限", spLightHigh);
    form->addRow("光照下限", spLightLow);
    form->addRow(hint);

    QDialogButtonBox *box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    box->button(QDialogButtonBox::Ok)->setText("应用");
    box->button(QDialogButtonBox::Cancel)->setText("取消");
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    double tH = spTempHigh->value();
    double tL = spTempLow->value();
    double lH = spLightHigh->value();
    double lL = spLightLow->value();

    // 自动保证 low <= high
    if (tL > tH) std::swap(tL, tH);
    if (lL > lH) std::swap(lL, lH);

    m_autoCtl->setThresholds(tH, tL, lH, lL);
}
