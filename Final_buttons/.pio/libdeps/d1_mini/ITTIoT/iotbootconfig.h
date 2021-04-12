#ifndef IOTBOOTCONFIG_H
#define IOTBOOTCONFIG_H
#include <ESP8266WebServer.h>
#include <iotboot.h>
#include <ittiot.h>

namespace ITT {

class IoTBootConfig : public IoTBoot
{
public:
    IoTBootConfig();
    void setup();
    void handle();
private:
    uint32_t _cfgTimeout;
};

}
#endif // IOTBOOTCONFIG_H
