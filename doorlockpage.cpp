#include "doorlockpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>

// ─── DoorLockRow ──────────────────────────────────────────

DoorLockRow::DoorLockRow(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_state(false)
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("门锁 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_btnToggle = new QPushButton("开锁", this);
    m_btnToggle->setFixedWidth(72);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_btnToggle);

    updateIndicator();
    connect(m_btnToggle, &QPushButton::clicked, this, &DoorLockRow::onToggleClicked);
}

QString DoorLockRow::pubTopic() const
{
    return (DOORLOCK_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void DoorLockRow::updateIndicator()
{
    if (m_state) {
        m_lblIndicator->setStyleSheet("color: #e04040; font-size: 22px;");
        m_btnToggle->setText("关锁");
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_btnToggle->setText("开锁");
    }
}

void DoorLockRow::publishState(bool on)
{
    QJsonObject obj;
    obj["doorLock"] = on;
    obj["id"]      = m_id;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void DoorLockRow::onToggleClicked()
{
    m_state = !m_state;
    updateIndicator();
    publishState(m_state);
}

void DoorLockRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("doorLock") || obj.value("id").toInt() != m_id)
        return;
    m_state = obj.value("doorLock").toBool();
    updateIndicator();
}

// ─── DoorLockPage ─────────────────────────────────────────

DoorLockPage::DoorLockPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("门锁控制", this);
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

    for (int i = 0; i < DOORLOCK_COUNT; ++i) {
        m_rows[i] = new DoorLockRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &DoorLockPage::backRequested);
}

void DoorLockPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < DOORLOCK_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
