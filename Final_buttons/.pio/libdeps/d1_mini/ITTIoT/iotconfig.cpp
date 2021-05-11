#include "iotconfig.h"

using namespace ITT;

IoTConfig::IoTConfig()
{
    _flashSector = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    noInterrupts();
    spi_flash_read(_flashSector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_buff), 1024);
    interrupts();
    _cfg = &_jb.parseObject(_buff);
    if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
        uint32_t crcOfData = calculateCRC32(((uint8_t*) &rtcData) + 4, sizeof(rtcData) - 4);
        if (crcOfData != rtcData.crc32) { // first boot
            rtcData.bootFlag = normalMode;
            rtcData.crc32 = calculateCRC32(((uint8_t*) &rtcData) + 4, sizeof(rtcData) - 4);
            ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
        }
    }
}

IoTConfig &IoTConfig::getInstance()
{
    static IoTConfig instance;
    return instance;
}

bool IoTConfig::setJsonConfig(String jCfg)
{
    StaticJsonBuffer<1024> tmpBuff;
    JsonObject& newCfg = tmpBuff.parseObject(jCfg);
    if(!newCfg.success()) return false; // Json valideerub ?

    bool res=false;
    noInterrupts();
    if(spi_flash_erase_sector(_flashSector) == SPI_FLASH_RESULT_OK) {
        if(spi_flash_write(_flashSector * SPI_FLASH_SEC_SIZE, (uint32_t*)(jCfg.c_str()), 1024) == SPI_FLASH_RESULT_OK) {
            res = true;
        }
    }
    interrupts();
    return res;
}

String IoTConfig::getJsonConfig()
{
    char buff[1024];
    noInterrupts();
    spi_flash_read(_flashSector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(buff), 1024);
    interrupts();
    StaticJsonBuffer<1024> tmpBuff;
    JsonObject& newjson = tmpBuff.parseObject(buff);
    String cfg;
    newjson.printTo(cfg);
    return cfg;
}

JsonObject *IoTConfig::getCfg()
{
    return _cfg;
}

bool IoTConfig::isValid()
{
    const char* keys[] = { "dname", "wname", "wpass", "msrv", "mssl", "mport", "muser", "mpass" };

    for (int n = 0; n < sizeof(keys)/sizeof(char*); ++n) {
        if(!_cfg->containsKey(keys[n])){
            return false;
        }
    }
    return true;
}

const uint8_t IoTConfig::getBootFlag()
{
    return rtcData.bootFlag;
}

void IoTConfig::setBootFlag(uint8_t bootMode)
{
    rtcData.bootFlag = bootMode;
    rtcData.crc32 = calculateCRC32(((uint8_t*) &rtcData) + 4, sizeof(rtcData) - 4);
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
}

uint32_t IoTConfig::calculateCRC32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffff;
    while (length--) {
      uint8_t c = *data++;
      for (uint32_t i = 0x80; i > 0; i >>= 1) {
        bool bit = crc & 0x80000000;
        if (c & i) {
          bit = !bit;
        }
        crc <<= 1;
        if (bit) {
          crc ^= 0x04c11db7;
        }
      }
    }
    return crc;
}

