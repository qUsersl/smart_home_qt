#include "sunshadepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>

// ─── SunshadeRow ──────────────────────────────────────────

SunshadeRow::SunshadeRow(int id, QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_state("stop")
    , m_mqtt(mqtt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("遮阳板 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblStatus = new QLabel("已停止", this);
    m_lblStatus->setMinimumWidth(80);
    m_lblStatus->setAlignment(Qt::AlignCenter);

    m_btnForward = new QPushButton("正转", this);
    m_btnReverse = new QPushButton("反转", this);
    m_btnStop    = new QPushButton("停止", this);
    m_btnForward->setFixedWidth(56);
    m_btnReverse->setFixedWidth(56);
    m_btnStop->setFixedWidth(56);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblStatus);
    layout->addWidget(m_btnForward);
    layout->addWidget(m_btnReverse);
    layout->addWidget(m_btnStop);

    updateIndicator();
    connect(m_btnForward, &QPushButton::clicked, this, &SunshadeRow::onForward);
    connect(m_btnReverse, &QPushButton::clicked, this, &SunshadeRow::onReverse);
    connect(m_btnStop,    &QPushButton::clicked, this, &SunshadeRow::onStop);
}

QString SunshadeRow::pubTopic() const
{
    return (SUNSHADE_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void SunshadeRow::updateIndicator()
{
    if (m_state == "forward") {
        m_lblIndicator->setStyleSheet("color: #4a90e2; font-size: 22px;");
        m_lblStatus->setText("正转中");
        m_lblStatus->setStyleSheet("color:#4a90e2;");
    } else if (m_state == "reverse") {
        m_lblIndicator->setStyleSheet("color: #8a6cd1; font-size: 22px;");
        m_lblStatus->setText("反转中");
        m_lblStatus->setStyleSheet("color:#8a6cd1;");
    } else {
        m_lblIndicator->setStyleSheet("color: gray; font-size: 22px;");
        m_lblStatus->setText("已停止");
        m_lblStatus->setStyleSheet("color:#888;");
    }
}

void SunshadeRow::publishState(const QString &action)
{
    QJsonObject obj;
    obj["sunshade"] = action;
    obj["id"]       = m_id;
    m_mqtt->publish(QMqttTopicName(pubTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SunshadeRow::onForward()
{
    m_state = "forward";
    updateIndicator();
    publishState(m_state);
}

void SunshadeRow::onReverse()
{
    m_state = "reverse";
    updateIndicator();
    publishState(m_state);
}

void SunshadeRow::onStop()
{
    m_state = "stop";
    updateIndicator();
    publishState(m_state);
}

void SunshadeRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("sunshade") || obj.value("id").toInt() != m_id)
        return;
    m_state = obj.value("sunshade").toString();
    updateIndicator();
}

// ─── SunshadePage ─────────────────────────────────────────

SunshadePage::SunshadePage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("电动遮阳板", this);
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

    for (int i = 0; i < SUNSHADE_COUNT; ++i) {
        m_rows[i] = new SunshadeRow(i, mqtt, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &SunshadePage::backRequested);
}

void SunshadePage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < SUNSHADE_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
