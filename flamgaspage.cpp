#include "flamgaspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

// ─── FlamGasRow ───────────────────────────────────────────

FlamGasRow::FlamGasRow(int id, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_detected(false)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    m_lblIndicator = new QLabel("●", this);
    m_lblIndicator->setFixedSize(28, 28);
    m_lblIndicator->setAlignment(Qt::AlignCenter);

    QLabel *lblName = new QLabel(QString("可燃气体 %1").arg(id + 1), this);
    QFont f = lblName->font();
    f.setPointSize(12);
    lblName->setFont(f);

    m_lblStatus = new QLabel(this);
    m_lblStatus->setFont(f);
    m_lblStatus->setMinimumWidth(120);
    m_lblStatus->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_lblIndicator);
    layout->addWidget(lblName);
    layout->addStretch();
    layout->addWidget(m_lblStatus);

    updateIndicator();
}

void FlamGasRow::updateIndicator()
{
    if (m_detected) {
        m_lblIndicator->setStyleSheet("color: #ff3030; font-size: 22px;");
        m_lblStatus->setText("⚠ 检测到可燃气体");
        m_lblStatus->setStyleSheet("color:#ff3030; font-weight:bold;");
    } else {
        m_lblIndicator->setStyleSheet("color: #30c060; font-size: 22px;");
        m_lblStatus->setText("正常");
        m_lblStatus->setStyleSheet("color:#1f8a4c;");
    }
}

void FlamGasRow::onMessageReceived(QByteArray data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (!obj.contains("flamGas") || obj.value("id").toInt() != m_id)
        return;
    m_detected = obj.value("flamGas").toBool();
    updateIndicator();
}

// ─── FlamGasPage ──────────────────────────────────────────

FlamGasPage::FlamGasPage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
{
    Q_UNUSED(mqtt)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("可燃气体检测", this);
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

    for (int i = 0; i < FLAMGAS_COUNT; ++i) {
        m_rows[i] = new FlamGasRow(i, this);
        layout->addWidget(m_rows[i]);
    }

    layout->addStretch();

    connect(btnBack, &QPushButton::clicked, this, &FlamGasPage::backRequested);
}

void FlamGasPage::onMessageReceived(QByteArray data)
{
    for (int i = 0; i < FLAMGAS_COUNT; ++i)
        m_rows[i]->onMessageReceived(data);
}
