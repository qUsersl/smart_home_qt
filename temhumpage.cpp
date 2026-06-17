#include "temhumpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

// ─── TemHumRow ────────────────────────────────────────────

TemHumRow::TemHumRow(int id, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(20);

    QLabel *lblName = new QLabel(QString("传感器 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblTem = new QLabel("温度: --.- °C", this);
    m_lblTem->setFont(f);

    m_lblHum = new QLabel("湿度: --.- %", this);
    m_lblHum->setFont(f);

    m_lblLight = new QLabel("光照: --.- lx", this);
    m_lblLight->setFont(f);

    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblTem);
    layout->addWidget(m_lblHum);
    layout->addWidget(m_lblLight);
}

void TemHumRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.value("id").toInt() != m_id)
        return;

    QString device = obj.value("device").toString();
    double  value  = obj.value("value").toDouble();

    if (device == "temp") {
        m_lblTem->setText(QString("温度: %1 °C").arg(value, 0, 'f', 1));
        emit temperatureChanged(value, m_id);
    } else if (device == "humi")
        m_lblHum->setText(QString("湿度: %1 %").arg(value, 0, 'f', 1));
    else if (device == "light_sensor") {
        m_lblLight->setText(QString("光照: %1 lx").arg(value, 0, 'f', 1));
        emit lightChanged(value, m_id);
    }
}

// ─── TemHumPage ───────────────────────────────────────────

TemHumPage::TemHumPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    Q_UNUSED(mqtt)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("环境监测", this);
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

    for (int i = 0; i < TEMHUM_COUNT; ++i) {
        m_rows[i] = new TemHumRow(i, this);
        layout->addWidget(m_rows[i]);
        connect(m_rows[i], &TemHumRow::temperatureChanged,
                this,     &TemHumPage::temperatureChanged);
        connect(m_rows[i], &TemHumRow::lightChanged,
                this,     &TemHumPage::lightChanged);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &TemHumPage::backRequested);
}

void TemHumPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < TEMHUM_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
