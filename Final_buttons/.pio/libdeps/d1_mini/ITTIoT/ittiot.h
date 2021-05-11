#ifndef ITTIOT_H
#define ITTIOT_H
#include "Arduino.h"
#include <ESP8266httpUpdate.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "iotconfig.h"
#include "iotbootnormal.h"
#include "iotbootconfig.h"
#include "iotboot.h"
#include "Ticker.h"

namespace ITT {

class ITTIoT
{
    struct stat_t {
        uint32_t uptime;
        int32_t rssi;
        uint32_t msg;
        uint32_t msgOk;
        uint32_t freeHeap;
    };

    stat_t st;

public:
    ITTIoT();

    void setup();
    void handle();
    JsonObject *cfg;
    bool setBootPin(uint8_t pin);
    const char* getMainTopic();
    void setBootFlag(uint8_t bootMode);
    void restart(uint8_t bootMode = normalMode);

    void log(const char *msg);
    void publishMsg(const char *topic,const char *msg, bool retain = false);
    void publishMsgTo(const char *topic,const char *msg, bool retain);
    void publishMsg(const char* topic,const uint8_t *msg, uint32_t len, bool retain = false);
    void publishMsgTo(const char* topic,const uint8_t *msg, uint32_t len, bool retain = false);
    void subscribe(const char* topic);
    void sendLocalInfo(const char* to = NULL);
    void sendStat(String topic, bool retain = false);
    void sendStat() { _statFlag = true; }
    void printConfig() { _printConfigFlag = true; }


private:

    Ticker statTicker;
    bool _statFlag;
    bool _printConfigFlag;

    uint8_t _bootPin = 0;
    uint8_t _bootMode;
    IoTBoot *_iotboot;
    IoTBoot *getBoot();

    void config_get(String msg);
    void config_set(String msg);
    void config_del(String msg);

    enum imageType { FIRMWARE, SPIFFS };
    void updateFW(String url, imageType type);

    void mqttCallback(char* topic, byte* payload, unsigned int length);
    void mqttReconnect();
    void _printConfig();

    PubSubClient *mqttClient;
    WiFiClient *wifiClient;

};
}
extern ITT::ITTIoT iot;

#endif // ITTIOT_H
