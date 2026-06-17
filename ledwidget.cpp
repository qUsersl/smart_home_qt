#include "ledwidget.h"
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>

LedWidget::LedWidget(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_state(false)
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(24, 24);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("LED-%1").arg(m_id), this);

    m_btnToggle = new QPushButton("打开", this);
    m_btnToggle->setFixedWidth(60);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_btnToggle);

    updateIndicator();

    connect(m_btnToggle, &QPushButton::clicked, this, &LedWidget::onToggleClicked);
}

QString LedWidget::pubTopic() const
{
    return (LED_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void LedWidget::updateIndicator()
{
    if (m_state) {
        m_lblIndicator->setStyleSheet("color: green; font-size: 20px;");
        m_btnToggle->setText("关闭");
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 20px;");
        m_btnToggle->setText("打开");
    }
}

void LedWidget::publishState(bool on)
{
    QJsonObject obj;
    obj["lamp"] = on;
    obj["id"]   = m_id;
    QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    m_mqtt->publish(QMqttTopicName(pubTopic()), payload);
}

void LedWidget::onToggleClicked()
{
    m_state = !m_state;
    updateIndicator();
    publishState(m_state);
}

void LedWidget::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("lamp"))
        return;
    if (obj.value("id").toInt() != m_id)
        return;

    m_state = obj.value("lamp").toBool();
    updateIndicator();
}
