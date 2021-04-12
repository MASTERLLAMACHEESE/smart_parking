#ifndef IOTCONFIG_H
#define IOTCONFIG_H
#include <Arduino.h>
#include <ArduinoJson.h>

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_end;

#define JBSIZE 1024

namespace ITT {
enum BootMode { normalMode, configMode };

class IoTConfig
{
public:
    IoTConfig();
    static IoTConfig &getInstance();
    bool setJsonConfig(String jCfg);
    String getJsonConfig();
    JsonObject *getCfg();
    bool isValid();
    const uint8_t getBootFlag();
    void setBootFlag(uint8_t bootMode);


private:
    uint32_t _flashSector;
    StaticJsonBuffer<JBSIZE> _jb;
    JsonObject *_cfg;
    char _buff[1024];
    uint32_t calculateCRC32(const uint8_t *data, size_t length);

    struct {
      uint32_t crc32;
      uint32_t bootFlag;
    } rtcData;

};

}
#endif // IOTCONFIG_H
