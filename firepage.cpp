#include "firepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

// ─── FireRow ──────────────────────────────────────────────

FireRow::FireRow(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_enabled(false)
    , m_fire(false)
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("火焰检测 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblStatus = new QLabel("未启用", this);
    m_lblStatus->setFont(f);
    m_lblStatus->setMinimumWidth(110);
    m_lblStatus->setAlignment(Qt::AlignCenter);

    m_btnToggle = new QPushButton("开启", this);
    m_btnToggle->setFixedWidth(72);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblStatus);
    layout->addWidget(m_btnToggle);

    updateIndicator();
    connect(m_btnToggle, &QPushButton::clicked, this, &FireRow::onToggleClicked);
}

QString FireRow::pubTopic() const
{
    return (FIRE_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void FireRow::updateIndicator()
{
    if (!m_enabled) {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_lblStatus->setText("未启用");
        m_lblStatus->setStyleSheet("color:#aaa;");
        m_btnToggle->setText("开启");
    } else if (m_fire) {
        m_lblIndicator->setStyleSheet("color: #ff3030; font-size: 22px;");
        m_lblStatus->setText("⚠ 检测到火焰");
        m_lblStatus->setStyleSheet("color:#ff3030; font-weight:bold;");
        m_btnToggle->setText("关闭");
    } else {
        m_lblIndicator->setStyleSheet("color: #30c060; font-size: 22px;");
        m_lblStatus->setText("正常");
        m_lblStatus->setStyleSheet("color:#1f8a4c;");
        m_btnToggle->setText("关闭");
    }
}

void FireRow::publishState(bool on)
{
    QJsonObject obj;
    obj["device"] = "fire_alarm";
    obj["status"] = on;
    obj["id"]     = m_id;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void FireRow::onToggleClicked()
{
    m_enabled = !m_enabled;
    if (!m_enabled)
        m_fire = false;
    updateIndicator();
    publishState(m_enabled);
    emit userToggled(m_enabled, m_id);
}

void FireRow::setEnabledByAuto(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    if (!m_enabled)
        m_fire = false;
    updateIndicator();
    publishState(m_enabled);
}

void FireRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.value("id").toInt() != m_id)
        return;

    QString device = obj.value("device").toString();

    if (device == "fire_alarm") {
        // 开关状态回写
        m_enabled = obj.value("status").toBool();
        if (!m_enabled)
            m_fire = false;
        updateIndicator();
    } else if (device == "fire") {
        // 火焰检测结果
        bool newFire = obj.value("status").toBool();
        bool wasOff  = !m_fire;
        m_fire = newFire;
        updateIndicator();

        // 仅在已启用 && 从无火焰跳变到有火焰时弹一次
        if (m_enabled && newFire && wasOff)
            emit fireDetected(m_id);
    }
}

// ─── FirePage ─────────────────────────────────────────────

FirePage::FirePage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("火焰报警", this);
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

    for (int i = 0; i < FIRE_COUNT; ++i) {
        m_rows[i] = new FireRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
        connect(m_rows[i], &FireRow::fireDetected, this, &FirePage::onFireDetected);
        connect(m_rows[i], &FireRow::userToggled,  this, &FirePage::userToggled);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &FirePage::backRequested);
}

void FirePage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < FIRE_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}

void FirePage::onFireDetected(int id)
{
    QMessageBox::warning(nullptr, "火焰报警",
                         QString("⚠ 检测到火焰！\n传感器编号：%1").arg(id));
}

void FirePage::setEnabledByAuto(bool on, int id)
{
    if (id >= 0 && id < FIRE_COUNT)
        m_rows[id]->setEnabledByAuto(on);
}
