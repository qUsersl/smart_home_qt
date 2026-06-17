#ifndef MQTTCONFIG_H
#define MQTTCONFIG_H

#include <QString>

// ========== MQTT 连接参数 ==========
extern const QString MQTT_BROKER_IP;
extern const int     MQTT_BROKER_PORT;
extern const QString MQTT_CLIENT_ID;

// 与云端交互的 topic
extern const QString MQTT_CLOUD_SUB;
extern const QString MQTT_CLOUD_PUB;

// 与硬件交互的 topic
extern const QString MQTT_HW_SUB;
extern const QString MQTT_HW_PUB;

#endif // MQTTCONFIG_H
