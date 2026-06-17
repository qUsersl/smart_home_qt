#include "voicepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

VoicePage::VoicePage(QMqttClient *mqtt, QWidget *parent)
    : QWidget(parent)
    , m_mqtt(mqtt)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    // 标题栏 + 返回按钮
    QHBoxLayout *titleBar = new QHBoxLayout;
    QLabel *title = new QLabel("语音助手", this);
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

    // 录音按钮（按住说话）
    m_btnRecord = new QPushButton("按住说话", this);
    m_btnRecord->setMinimumHeight(64);
    m_btnRecord->setCursor(Qt::PointingHandCursor);
    m_btnRecord->setStyleSheet(
        "QPushButton{background:#4a90e2; color:#fff; border:none; border-radius:10px; padding:10px 14px; font-size:14px;}"
        "QPushButton:pressed{background:#2f6db3;}");
    layout->addWidget(m_btnRecord);

    m_lblHint = new QLabel(
        "提示：按住按钮说话，松开后开始识别。\n"
        "门锁：开门 / 关门 / 锁门\n"
        "灯光：打开灯光一 / 关闭灯一 / 打开所有灯 / 关闭灯光\n"
        "风扇：打开风扇一档 / 二档 / 三档 / 关闭风扇\n"
        "报警：打开报警器 / 关闭报警器", this);
    m_lblHint->setStyleSheet("color:#7a849a; font-size:12px;");
    layout->addWidget(m_lblHint);

    QLabel *lblTextTitle = new QLabel("识别结果：", this);
    lblTextTitle->setStyleSheet("color:#20324a; font-weight:bold;");
    layout->addWidget(lblTextTitle);

    m_lblText = new QLabel("(尚未识别)", this);
    m_lblText->setStyleSheet(
        "background:#fff; border:1px solid #dfe3eb; border-radius:6px; "
        "padding:8px; color:#1a2540; font-size:14px;");
    m_lblText->setMinimumHeight(40);
    m_lblText->setWordWrap(true);
    layout->addWidget(m_lblText);

    QLabel *lblLogTitle = new QLabel("操作日志：", this);
    lblLogTitle->setStyleSheet("color:#20324a; font-weight:bold;");
    layout->addWidget(lblLogTitle);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setStyleSheet(
        "QTextEdit{background:#fff; border:1px solid #dfe3eb; border-radius:6px; "
        "padding:6px; color:#333;}");
    layout->addWidget(m_log, 1);

    // 录音文件保存路径：项目目录上一级
    m_filepath = "/home/hqyj/myproject/home_client/home_client_voice.pcm";

    connect(btnBack,     &QPushButton::clicked,  this, &VoicePage::backRequested);
    connect(m_btnRecord, &QPushButton::pressed,  this, &VoicePage::onRecordPressed);
    connect(m_btnRecord, &QPushButton::released, this, &VoicePage::onRecordReleased);
}

QString VoicePage::doorLockTopic() const
{
    return (VOICE_DOORLOCK_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

QString VoicePage::ledTopic() const
{
    return (VOICE_LED_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

QString VoicePage::fanTopic() const
{
    return (VOICE_FAN_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

QString VoicePage::alarmTopic() const
{
    return (VOICE_ALARM_MODE == 0) ? MQTT_CLOUD_PUB : MQTT_HW_PUB;
}

void VoicePage::appendLog(const QString &line)
{
    QString stamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_log->append(QString("[%1] %2").arg(stamp, line));
}

void VoicePage::onRecordPressed()
{
    m_btnRecord->setText("正在录音... 松开识别");
    m_lblText->setText("(录音中...)");
    m_capture.startCapture(m_filepath);
}

void VoicePage::onRecordReleased()
{
    m_btnRecord->setText("按住说话");
    m_capture.stopCapture();

    m_lblText->setText("识别中...");
    QCoreApplication::processEvents();

    const QAudioFormat format = m_capture.captureFormat();
    const QString text = m_recognizer.speechIdentify(m_filepath, format);

    if (text.isEmpty()) {
        m_lblText->setText("(未识别到内容)");
        appendLog("未识别到内容");
        return;
    }

    m_lblText->setText(text);
    appendLog(QString("识别：%1").arg(text));

    if (!handleCommand(text)) {
        appendLog("未匹配到可执行指令");
    }
}

int VoicePage::parseLedIndex(const QString &text) const
{
    // 中文数字 / 阿拉伯数字 → 0..2（页面里 id 从 0 开始，文案是"灯光 1"）
    if (text.contains("一") || text.contains("1") || text.contains("壹")) return 0;
    if (text.contains("二") || text.contains("2") || text.contains("贰") || text.contains("两")) return 1;
    if (text.contains("三") || text.contains("3") || text.contains("叁")) return 2;
    return -1;
}

bool VoicePage::handleCommand(const QString &text)
{
    // ── 门锁 ───────────────────────────────────────────
    static const QStringList doorClose = {
        "关门", "关锁", "锁门", "关闭门锁", "关闭门", "把门关", "把门锁"
    };
    for (const QString &kw : doorClose) {
        if (text.contains(kw)) {
            publishDoorLock(false);
            appendLog("→ 门锁：关锁");
            return true;
        }
    }
    static const QStringList doorOpen = {
        "开门", "开锁", "打开门", "打开门锁", "把门打开", "把门开"
    };
    for (const QString &kw : doorOpen) {
        if (text.contains(kw)) {
            publishDoorLock(true);
            appendLog("→ 门锁：开锁");
            return true;
        }
    }

    // ── 报警器 ─────────────────────────────────────────
    if (text.contains("报警")) {
        // 先看关闭语义
        if (text.contains("关闭报警") || text.contains("关报警")
            || text.contains("停止报警") || text.contains("关掉报警")) {
            publishAlarm(false);
            appendLog("→ 报警器：关闭");
            return true;
        }
        if (text.contains("打开报警") || text.contains("开报警")
            || text.contains("启动报警") || text.contains("开启报警")) {
            publishAlarm(true);
            appendLog("→ 报警器：打开");
            return true;
        }
    }

    // ── 风扇 ───────────────────────────────────────────
    if (text.contains("风扇")) {
        if (text.contains("关闭风扇") || text.contains("关风扇")
            || text.contains("停风扇") || text.contains("关掉风扇")) {
            publishFan(0);
            appendLog("→ 风扇：关闭");
            return true;
        }
        if (text.contains("一档") || text.contains("1档") || text.contains("一挡") || text.contains("1挡")) {
            publishFan(1);
            appendLog("→ 风扇：1档");
            return true;
        }
        if (text.contains("二档") || text.contains("2档") || text.contains("二挡") || text.contains("2挡")
            || text.contains("两档") || text.contains("两挡")) {
            publishFan(2);
            appendLog("→ 风扇：2档");
            return true;
        }
        if (text.contains("三档") || text.contains("3档") || text.contains("三挡") || text.contains("3挡")) {
            publishFan(3);
            appendLog("→ 风扇：3档");
            return true;
        }
        if (text.contains("打开风扇") || text.contains("开风扇") || text.contains("启动风扇")) {
            publishFan(1);
            appendLog("→ 风扇：开（默认1档）");
            return true;
        }
    }

    // ── 灯光 ───────────────────────────────────────────
    const bool mentionLight =
        text.contains("灯") || text.contains("灯光");

    if (mentionLight) {
        const bool wantClose =
            text.contains("关闭") || text.contains("关掉") || text.contains("关一下")
            || text.startsWith("关") || text.contains("熄灭");
        const bool wantOpen =
            text.contains("打开") || text.contains("开启") || text.contains("开一下")
            || (text.startsWith("开") && !wantClose);

        if (wantClose || wantOpen) {
            const bool on = wantOpen && !wantClose;
            const bool all = text.contains("所有") || text.contains("全部")
                              || text.contains("全开") || text.contains("全关");
            if (all) {
                publishLed(-1, on);
                appendLog(QString("→ 灯光：全部%1").arg(on ? "打开" : "关闭"));
                return true;
            }
            const int idx = parseLedIndex(text);
            if (idx >= 0) {
                publishLed(idx, on);
                appendLog(QString("→ 灯光 %1：%2").arg(idx + 1).arg(on ? "打开" : "关闭"));
                return true;
            }
            // 未指定编号 → 默认全部
            publishLed(-1, on);
            appendLog(QString("→ 灯光：全部%1").arg(on ? "打开" : "关闭"));
            return true;
        }
    }

    return false;
}

void VoicePage::publishDoorLock(bool open)
{
    QJsonObject obj;
    obj["doorLock"] = open;
    obj["id"]       = 0;
    m_mqtt->publish(QMqttTopicName(doorLockTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void VoicePage::publishLed(int id, bool on)
{
    auto sendOne = [this, on](int i) {
        QJsonObject obj;
        obj["device"] = "light";
        obj["status"] = on;
        obj["id"]     = i;
        m_mqtt->publish(QMqttTopicName(ledTopic()),
                        QJsonDocument(obj).toJson(QJsonDocument::Compact));
    };
    if (id < 0) {
        for (int i = 0; i < 3; ++i) sendOne(i);
    } else {
        sendOne(id);
    }
}

void VoicePage::publishFan(int level)
{
    QJsonObject obj;
    obj["device"] = "fan";
    obj["status"] = (level > 0);
    obj["id"]     = 0;
    if (level > 0) obj["level"] = level;
    m_mqtt->publish(QMqttTopicName(fanTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void VoicePage::publishAlarm(bool on)
{
    QJsonObject obj;
    obj["device"] = "alarm";
    obj["status"] = on;
    obj["id"]     = 0;
    m_mqtt->publish(QMqttTopicName(alarmTopic()),
                    QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
