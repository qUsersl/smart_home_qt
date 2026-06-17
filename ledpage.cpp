#include "ledpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>

// ─── LedRow ───────────────────────────────────────────────

LedRow::LedRow(int id, QMqttClient *mqtt, QWidget *parent)
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

    QLabel *lblName = new QLabel(QString("灯光 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_btnToggle = new QPushButton("打开", this);
    m_btnToggle->setFixedWidth(72);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_btnToggle);

    updateIndicator();
    connect(m_btnToggle, &QPushButton::clicked, this, &LedRow::onToggleClicked);
}

QString LedRow::pubTopic() const
{
    return (LED_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void LedRow::updateIndicator()
{
    if (m_state) {
        m_lblIndicator->setStyleSheet("color: #f0c040; font-size: 22px;");
        m_btnToggle->setText("关闭");
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_btnToggle->setText("打开");
    }
}

void LedRow::publishState(bool on)
{
    QJsonObject obj;
    obj["device"] = "light";
    obj["status"] = on;
    obj["id"]     = m_id;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void LedRow::onToggleClicked()
{
    m_state = !m_state;
    updateIndicator();
    publishState(m_state);
    emit userToggled(m_state, m_id);
}

void LedRow::setStateByAuto(bool on)
{
    if (m_state == on) return;
    m_state = on;
    updateIndicator();
    publishState(on);
}

void LedRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.value("device").toString() != "light" || obj.value("id").toInt() != m_id)
        return;
    m_state = obj.value("status").toBool();
    updateIndicator();
}

// ─── LedPage ──────────────────────────────────────────────

LedPage::LedPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    // 顶部：标题 + 返回按钮
    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("灯光控制", this);
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

    // 分隔线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    // 3 个 LED 行
    for (int i = 0; i < LED_COUNT; ++i) {
        m_rows[i] = new LedRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
        connect(m_rows[i], &LedRow::userToggled,
                this,     &LedPage::userToggled);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &LedPage::backRequested);
}

void LedPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < LED_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}

void LedPage::setStateByAuto(bool on)
{
    for (int i = 0; i < LED_COUNT; ++i)
        m_rows[i]->setStateByAuto(on);
}
