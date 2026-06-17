#include "fanpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>

// ─── FanRow ───────────────────────────────────────────────

FanRow::FanRow(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_level(0)
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("风扇 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblLevel = new QLabel("关", this);
    m_lblLevel->setMinimumWidth(40);
    m_lblLevel->setAlignment(Qt::AlignCenter);

    m_btnOff = new QPushButton("关", this);
    m_btnL1  = new QPushButton("一档", this);
    m_btnL2  = new QPushButton("二档", this);
    m_btnL3  = new QPushButton("三档", this);
    m_btnOff->setFixedWidth(56);
    m_btnL1->setFixedWidth(56);
    m_btnL2->setFixedWidth(56);
    m_btnL3->setFixedWidth(56);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblLevel);
    layout->addWidget(m_btnOff);
    layout->addWidget(m_btnL1);
    layout->addWidget(m_btnL2);
    layout->addWidget(m_btnL3);

    updateIndicator();
    connect(m_btnOff, &QPushButton::clicked, this, [this]{ onLevelClicked(0); });
    connect(m_btnL1,  &QPushButton::clicked, this, [this]{ onLevelClicked(1); });
    connect(m_btnL2,  &QPushButton::clicked, this, [this]{ onLevelClicked(2); });
    connect(m_btnL3,  &QPushButton::clicked, this, [this]{ onLevelClicked(3); });
}

QString FanRow::pubTopic() const
{
    return (FAN_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void FanRow::updateIndicator()
{
    if (m_level > 0) {
        m_lblIndicator->setStyleSheet("color: #40c0f0; font-size: 22px;");
        m_lblLevel->setText(QString("%1档").arg(m_level));
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_lblLevel->setText("关");
    }
}

void FanRow::publishState(int level)
{
    QJsonObject obj;
    obj["device"] = "fan";
    obj["status"] = (level > 0);
    obj["id"]     = m_id;
    if (level > 0)
        obj["level"] = level;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void FanRow::onLevelClicked(int level)
{
    m_level = level;
    updateIndicator();
    publishState(level);
    emit userLevelChanged(level, m_id);
}

void FanRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.value("device").toString() != "fan" || obj.value("id").toInt() != m_id)
        return;
    bool on = obj.value("status").toBool();
    m_level = on ? obj.value("level").toInt(1) : 0;
    updateIndicator();
}

void FanRow::setLevelByAuto(int level)
{
    if (m_level == level) return;
    m_level = level;
    updateIndicator();
    publishState(level);
}

// ─── FanPage ──────────────────────────────────────────────

FanPage::FanPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    // 顶部：标题 + 返回按钮
    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("风扇控制", this);
    QFont f = title->font();
    f.setPointSize(14);
    f.setBold(true);
    title->setFont(f);

    QPushButton *btnBack = new QPushButton("返回", this);
    btnBack->setFixedWidth(60);

    titleBar->addWidget(title);
    titleBar->addStretch();
    titleBar->addWidget(btnBack);
    layout->addLayout(titleBar);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    for (int i = 0; i < FAN_COUNT; ++i) {
        m_rows[i] = new FanRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
        connect(m_rows[i], &FanRow::userLevelChanged,
                this,     &FanPage::userLevelChanged);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &FanPage::backRequested);
}

void FanPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < FAN_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}

void FanPage::setLevelByAuto(int level, int id)
{
    if (id >= 0 && id < FAN_COUNT)
        m_rows[id]->setLevelByAuto(level);
}
