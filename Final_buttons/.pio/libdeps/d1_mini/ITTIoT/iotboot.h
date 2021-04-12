#ifndef IOTBOOT_H
#define IOTBOOT_H
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "iotconfig.h"

namespace ITT {

class IoTBoot
{
public:
    explicit IoTBoot();
    void setupOTA();
    virtual void setup();
    virtual void handle();
};

}
#endif // IOTBOOT_H
