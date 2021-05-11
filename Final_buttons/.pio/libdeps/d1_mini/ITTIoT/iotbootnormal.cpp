#include "iotbootnormal.h"
using namespace ITT;
extern ITTIoT iot;

IoTBootNormal::IoTBootNormal()
{
    cfg = IoTConfig::getInstance().getCfg();
}

void IoTBootNormal::setup()
{
    Serial.println("IoT boot normal ...");
    WiFi.mode(WIFI_STA);

    WiFi.begin(cfg->get<const char*>("wname") ,cfg->get<const char*>("wpass"));
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        iot.restart(configMode);
    }

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    IoTBoot::setupOTA();



}

void IoTBootNormal::handle()
{
    ArduinoOTA.handle();

}



