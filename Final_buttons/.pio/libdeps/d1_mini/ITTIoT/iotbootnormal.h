#ifndef IOTBOOTNORMAL_H
#define IOTBOOTNORMAL_H
#include "iotboot.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ittiot.h"

namespace ITT {

class IoTBootNormal : public IoTBoot
{
public:
    IoTBootNormal();
    void setup();
    void handle();
private:
    const JsonObject *cfg;


};

};
#endif // IOTBOOTNORMAL_H
