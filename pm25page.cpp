#include "pm25page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

// ─── Pm25Row ──────────────────────────────────────────────

Pm25Row::Pm25Row(int id, QWidget *parent)
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

    m_lblPm25 = new QLabel("PM2.5: ---- g/m³", this);
    m_lblPm25->setFont(f);

    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblPm25);
}

void Pm25Row::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("PM2.5") || obj.value("id").toInt() != m_id)
        return;

    double pm25 = obj.value("PM2.5").toDouble();
    m_lblPm25->setText(QString("PM2.5: %1 g/m³").arg(pm25, 0, 'f', 1));
}

// ─── Pm25Page ─────────────────────────────────────────────

Pm25Page::Pm25Page(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    Q_UNUSED(mqtt)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("PM2.5 浓度", this);
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

    for (int i = 0; i < PM25_COUNT; ++i) {
        m_rows[i] = new Pm25Row(i, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &Pm25Page::backRequested);
}

void Pm25Page::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < PM25_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
