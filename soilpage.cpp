#include "soilpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

// ─── SoilRow ──────────────────────────────────────────────

SoilRow::SoilRow(int id, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(20);

    QLabel *lblName = new QLabel(QString("土壤 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblTem = new QLabel("温度: --.- °C", this);
    m_lblTem->setFont(f);

    m_lblHum = new QLabel("湿度: --.- %", this);
    m_lblHum->setFont(f);

    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblTem);
    layout->addWidget(m_lblHum);
}

void SoilRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("soiltem") || obj.value("id").toInt() != m_id)
        return;

    double tem = obj.value("soiltem").toDouble();
    double hum = obj.value("soilhum").toDouble();
    m_lblTem->setText(QString("温度: %1 °C").arg(tem, 0, 'f', 1));
    m_lblHum->setText(QString("湿度: %1 %").arg(hum, 0, 'f', 1));
}

// ─── SoilPage ─────────────────────────────────────────────

SoilPage::SoilPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    Q_UNUSED(mqtt)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("土壤温湿度", this);
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

    for (int i = 0; i < SOIL_COUNT; ++i) {
        m_rows[i] = new SoilRow(i, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &SoilPage::backRequested);
}

void SoilPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < SOIL_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
