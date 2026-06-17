#include "peoplepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>

// ─── PeopleRow ────────────────────────────────────────────

PeopleRow::PeopleRow(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_state(false)
    , m_intruded(false)
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("人体检测 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblIntrude = new QLabel(this);
    m_lblIntrude->setFont(f);
    m_lblIntrude->setMinimumWidth(110);
    m_lblIntrude->setAlignment(Qt::AlignCenter);

    m_btnToggle = new QPushButton("开启", this);
    m_btnToggle->setFixedWidth(72);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblIntrude);
    layout->addWidget(m_btnToggle);

    updateIndicator();
    updateIntrudeLabel();
    connect(m_btnToggle, &QPushButton::clicked, this, &PeopleRow::onToggleClicked);
}

QString PeopleRow::pubTopic() const
{
    return (PEOPLE_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void PeopleRow::updateIndicator()
{
    if (m_state) {
        m_lblIndicator->setStyleSheet("color: #30c060; font-size: 22px;");
        m_btnToggle->setText("关闭");
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_btnToggle->setText("开启");
    }
}

void PeopleRow::updateIntrudeLabel()
{
    if (!m_state) {
        m_lblIntrude->setText("未启用");
        m_lblIntrude->setStyleSheet("color:#aaa;");
    } else if (m_intruded) {
        m_lblIntrude->setText("⚠ 检测到闯入");
        m_lblIntrude->setStyleSheet("color:#ff3030; font-weight:bold;");
    } else {
        m_lblIntrude->setText("无人");
        m_lblIntrude->setStyleSheet("color:#1f8a4c;");
    }
}

void PeopleRow::publishState(bool on)
{
    QJsonObject obj;
    obj["device"] = "people_detect";
    obj["status"] = on;
    obj["id"]     = m_id;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void PeopleRow::onToggleClicked()
{
    m_state = !m_state;
    if (!m_state)
        m_intruded = false;     // 关闭时清除闯入状态
    updateIndicator();
    updateIntrudeLabel();
    publishState(m_state);
    emit userToggled(m_state, m_id);
}

void PeopleRow::setEnabledByAuto(bool on)
{
    if (m_state == on) return;
    m_state = on;
    if (!m_state)
        m_intruded = false;
    updateIndicator();
    updateIntrudeLabel();
    publishState(m_state);
}

void PeopleRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.value("id").toInt() != m_id)
        return;

    QString device = obj.value("device").toString();

    if (device == "people_detect") {
        // 检测器开关状态变更
        m_state = obj.value("status").toBool();
        if (!m_state)
            m_intruded = false;
        updateIndicator();
        updateIntrudeLabel();
    } else if (device == "people") {
        // 闯入检测结果：仅更新界面，不弹窗
        m_intruded = obj.value("status").toBool();
        updateIntrudeLabel();
    }
}

// ─── PeoplePage ───────────────────────────────────────────

PeoplePage::PeoplePage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("人体检测", this);
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

    for (int i = 0; i < PEOPLE_COUNT; ++i) {
        m_rows[i] = new PeopleRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
        connect(m_rows[i], &PeopleRow::userToggled,
                this,     &PeoplePage::userToggled);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &PeoplePage::backRequested);
}

void PeoplePage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < PEOPLE_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}

void PeoplePage::setEnabledByAuto(bool on, int id)
{
    if (id >= 0 && id < PEOPLE_COUNT)
        m_rows[id]->setEnabledByAuto(on);
}
