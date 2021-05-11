#include "iotbootconfig.h"
extern "C" {
#include "user_interface.h"
}

using namespace ITT;
extern ITTIoT iot;

ESP8266WebServer server(80);

void _tmrCfgCb(){

}

IoTBootConfig::IoTBootConfig()
{
    _cfgTimeout = 120 * 1000; // 120 sec
}

void handleRoot() {
    server.send(200, "text/plain", "");
}

void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

String getInfo(){
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["id"] = String(ESP.getChipId() , 16);
    root["mac"] = WiFi.macAddress();
    root["cpu_freq"] = String(ESP.getCpuFreqMHz());
    root["f_size"] = String(ESP.getFlashChipSize());
    root["f_speed"] = String(ESP.getFlashChipSpeed());
    root["f_free"] = String(ESP.getFreeSketchSpace());
    root["fw_size"] = String(ESP.getSketchSize());
    root["fw_build"] = __DATE__ " " __TIME__;
    root["sdk_version"] = String(ESP.getSdkVersion());
    String result;
    root.printTo(result);
    return result;
}

String getNetworks(){

    DynamicJsonBuffer jsonBuffer;
    JsonArray& root = jsonBuffer.createArray();

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i)
    {
        JsonArray& network = root.createNestedArray();
        network.add(WiFi.SSID(i));
        network.add(String(WiFi.RSSI(i)));
        network.add((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
    }
    String result;
    root.printTo(result);
    return result;
}

void IoTBootConfig::setup()
{
    Serial.println("IoT boot config ...");

    uint8 mode = 0;
    wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);

    char buffName[13];
    sprintf(buffName, "IoT-%08x", ESP.getChipId());
    WiFi.softAP(buffName);

    server.on("/", handleRoot);

    server.on("/restart", HTTP_GET, [](){
        server.send ( 200, "text/json", "{\"success\":\"true\"}" );
        iot.restart();
    });

    server.on("/ping", HTTP_GET, [this](){
        _cfgTimeout += 60 * 1000; // add +1 min to config mode timeout
        server.send ( 200, "text/json", "{\"success\":\"true\"}" );
    });

    server.on("/get/config", HTTP_GET, [](){
        server.send ( 200, "text/json", ITT::IoTConfig::getInstance().getJsonConfig() );
    });

    server.on("/get/info", HTTP_GET, [](){
        server.send ( 200, "text/json", getInfo());
    });

    server.on("/get/networks", HTTP_GET, [](){
        server.send ( 200, "text/json", getNetworks());
    });

    server.on("/set/config", HTTP_POST, [](){
        if(ITT::IoTConfig::getInstance().setJsonConfig(server.arg("plain"))){
            server.send ( 200, "text/json", "{\"success\":\"true\"}" );
        }else {
            server.send ( 200, "text/json", "{\"success\":\"false\"}" );
        }
    });

    server.onNotFound(handleNotFound);
    server.begin();

    IoTBoot::setupOTA();

}

void IoTBootConfig::handle()
{
    ArduinoOTA.handle();
    server.handleClient();
    // restart to normal mode if config timeout reached
    if(millis() >= _cfgTimeout) iot.restart();

}



