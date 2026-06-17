# Smart Home Qt Client

基于 **Qt 5.12 + MQTT** 的智能家居客户端，作为云端 / 嵌入式硬件设备的统一控制台。

应用通过 `QStackedWidget` 管理多个功能页面，主窗口维护一个共享的 `QMqttClient`，所有功能页面复用同一连接。消息按 JSON 中的 `device` 字段分发到对应页面，控制类设备的状态变化也会回流到 UI 上。

---

## 功能模块

### 设备端模块（实际硬件）

| 模块 | 描述 |
| ---- | ---- |
| 💡 灯光 | 3 路 LED 开关（`device:"light"`） |
| 🌀 风扇 | 关 / 一档 / 二档 / 三档（`device:"fan"` + `level`） |
| 🌡 温湿度光照 | 显示温度、湿度、光照（`device:"temp"` / `"humi"` / `"light_sensor"`） |
| 🔔 报警器 | 启停（`device:"alarm"`） |
| 🚶 人体检测 | 检测器开关 + 闯入边沿弹窗（`device:"people_detect"` / `"people"`） |
| 🔥 火焰报警 | 检测器开关 + 火焰边沿弹窗（`device:"fire_alarm"` / `"fire"`） |

### 仅云端模块

| 模块 | 描述 |
| ---- | ---- |
| 🔒 门锁 | 状态展示与控制 |
| 🌱 土壤温湿度 | 显示土壤温湿度（`soiltem` / `soilhum`） |
| 🫧 二氧化碳 | 显示 CO₂ 浓度（`co2`） |

### 自动控制

主页底部「自动控制」开关 + 阈值配置：

- 温度高于上限自动开风扇一档，低于下限自动关闭
- 光照低于下限自动开灯，高于上限自动关灯
- 自动模式开启时同步启用人体检测和火焰报警
- 用户手动发出与自动期望相反的指令 → 自动退出，用户操作生效（手动优先）
- 阈值（温度高 / 低、光照高 / 低）支持运行时通过对话框修改

---

## 消息格式约定

控制类（开关 / 档位）：

```json
{ "device": "light", "status": true,  "id": 0 }
{ "device": "fan",   "status": true,  "id": 0, "level": 2 }
{ "device": "alarm", "status": false, "id": 0 }
```

传感器类：

```json
{ "device": "temp",         "value": 26.5,   "id": 0 }
{ "device": "humi",         "value": 60.0,   "id": 0 }
{ "device": "light_sensor", "value": 320.0,  "id": 0 }
{ "soiltem": 60.4, "soilhum": 90.4, "id": 0 }
{ "co2": 4521.8, "id": 0 }
```

事件类（边沿触发弹窗）：

```json
{ "device": "people", "status": true, "id": 0 }
{ "device": "fire",   "status": true, "id": 0 }
```

---

## 目录结构

```
home_client/
├── main.cpp
├── widget.{h,cpp,ui}        主窗口：MQTT 连接、页面切换
├── mqttconfig.{h,cpp}       MQTT 参数集中定义
├── ledpage.{h,cpp}          灯光控制
├── fanpage.{h,cpp}          风扇控制
├── temhumpage.{h,cpp}       温湿度光照
├── alarmpage.{h,cpp}        报警器
├── peoplepage.{h,cpp}       人体检测
├── firepage.{h,cpp}         火焰报警
├── doorlockpage.{h,cpp}     门锁
├── soilpage.{h,cpp}         土壤温湿度
├── co2page.{h,cpp}          二氧化碳
├── autocontrol.{h,cpp}      自动控制器
└── home_client.pro
```

---

## MQTT 配置

所有连接参数和 topic 集中在 `mqttconfig.cpp`，避免散落到各模块：

```cpp
const QString MQTT_BROKER_IP   = "mqtt.yyzlab.com.cn";
const int     MQTT_BROKER_PORT = 1883;
const QString MQTT_CLIENT_ID   = "lq123456ll";

const QString MQTT_CLOUD_SUB   = "1781148959353/AIOTSIM2APP";
const QString MQTT_CLOUD_PUB   = "1781148959353/APP2AIOTSIM";
const QString MQTT_HW_SUB      = "AIOTSIM2Device";
const QString MQTT_HW_PUB      = "Device2AIOTSIM";
```

每个控制模块顶部用宏 `*_MODE` 切换走云端 (`0`) 还是硬件 (`1`)：

```cpp
#define LED_MODE    1
#define FAN_MODE    1
#define ALARM_MODE  0
```

订阅时同时订阅云端和硬件两个 topic，无论从哪一端推送状态都能收到。

---

## 构建与运行

依赖：

- Qt 5.12（含 `QtMqtt` 模块）
- C++11

构建：

```bash
qmake home_client.pro
make
./home_client
```

或在 Qt Creator 中直接打开 `home_client.pro`。

---

## 新增功能模块的步骤

1. 新建 `xxxpage.{h,cpp}`，继承 `QWidget`
   - 顶部含标题 + 返回按钮，返回按钮 `emit backRequested()`
   - 通过持有的 `QMqttClient *m_mqtt` 调用 `publish` 发送
   - `onMessageReceived(QByteArray)` 处理订阅消息（按 `device` + `id` 路由到具体设备）
2. 在 `widget.h` 添加成员，在 `widget.cpp` 创建实例并注册到 `m_stack`
3. 主页加入入口按钮，连接 `setCurrentIndex` 跳转
4. `Widget::onMessageReceived` 增加按 `device` 字段分发的分支
5. `home_client.pro` 添加 `SOURCES` / `HEADERS`

控制类页面如果想被自动控制器接管，还需要：

- Row 增加 `userToggled` / `userLevelChanged` 信号（**只在手动点击时**触发）
- Row 增加 `setStateByAuto` / `setLevelByAuto` 方法（不发用户信号，避免回环）
- `AutoController` 监听信号检测冲突 → 自动退出

---

## Qt 5.12 注意事项

- `.ui` 文件 margin 需用 `leftMargin` / `topMargin` / `rightMargin` / `bottomMargin` 四个独立属性，不用 `contentsMargins`
- 固定宽度用 `minimumWidth` + `maximumWidth`，不用 `fixedWidth`
- `.pro` 中需添加 `QT += mqtt`

---

## License

MIT
