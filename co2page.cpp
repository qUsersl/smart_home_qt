#include "co2page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

// ─── Co2Row ───────────────────────────────────────────────

Co2Row::Co2Row(int id, QWidget *parent)
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

    m_lblCo2 = new QLabel("CO₂: ---- ppm", this);
    m_lblCo2->setFont(f);

    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblCo2);
}

void Co2Row::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("co2") || obj.value("id").toInt() != m_id)
        return;

    double co2 = obj.value("co2").toDouble();
    m_lblCo2->setText(QString("CO₂: %1 ppm").arg(co2, 0, 'f', 1));
}

// ─── Co2Page ──────────────────────────────────────────────

Co2Page::Co2Page(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    Q_UNUSED(mqtt)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("二氧化碳浓度", this);
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

    for (int i = 0; i < CO2_COUNT; ++i) {
        m_rows[i] = new Co2Row(i, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &Co2Page::backRequested);
}

void Co2Page::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < CO2_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
