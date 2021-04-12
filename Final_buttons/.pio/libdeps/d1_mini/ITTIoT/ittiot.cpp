#include "ittiot.h"
extern void iot_init_cb()__attribute__((weak)) ;
extern void iot_received(String(topic), String msg)__attribute__((weak)) ;
extern void iot_connected()__attribute__((weak)) ;
using namespace ITT;

void statTickerCB();

ITTIoT::ITTIoT()
{
    cfg = IoTConfig::getInstance().getCfg();
    _printConfigFlag = false;
    _statFlag = false;
}

/*
 *  configurator functions
 */

void ITTIoT::config_get(String msg){

    // check if get request contains replay topic
    String retTopic = "log";
    int retTopicPos = msg.indexOf('!');
    if(retTopicPos != -1){
        retTopic = msg.substring(retTopicPos + 1, msg.length());
        msg = msg.substring(0, retTopicPos - 1);
    }

    if(msg.equals("get reset")){
        publishMsg(retTopic.c_str(), ESP.getResetReason().c_str());
        return;
    };
    if(msg.equals("get rssi")){
        publishMsg(retTopic.c_str(), String(WiFi.RSSI()).c_str());
        return;
    }
    if(msg.equals("get free heap")){
        publishMsg(retTopic.c_str(), String(ESP.getFreeHeap()).c_str());
        return;
    }
    if(msg.equals("get free flash")){
        publishMsg(retTopic.c_str(), String(ESP.getFreeSketchSpace()).c_str());
        return;
    }
    if(msg.equals("get flash size")){
        publishMsg(retTopic.c_str(), String(ESP.getFlashChipSize()).c_str());
        return;
    }
    if(msg.equals("get sketch size")){
        publishMsg(retTopic.c_str(), String(ESP.getSketchSize()).c_str());
        return;
    }
    if(msg.equals("get ip")){
        publishMsg(retTopic.c_str(), WiFi.localIP().toString().c_str());
        return;
    }
    if(msg.equals("get uptime")){
        publishMsg(retTopic.c_str(), String(millis()/1000).c_str());
        return;
    }
    if(msg.equals("get stat")){
        sendStat(retTopic);
        return;
    }


    // get config key
    if(cfg->containsKey(msg.substring(4))){
        publishMsg(retTopic.c_str(), cfg->get<String>(msg.substring(4).c_str()).c_str());
        return;
    }
    publishMsg(retTopic.c_str(), "?");
}

void ITTIoT::config_set(String msg){
    msg = msg.substring(4);
    int separator = msg.indexOf(' ');
    if(separator == -1){
        log("?");
        return;
    }
    const char *res;
    if(cfg->set(msg.substring(0, separator), msg.substring(separator + 1))){
        res = "Ok";
    }else{
        res = "Err";
    }
    log(res);
}

void ITTIoT::config_del(String msg){
    if(cfg->containsKey(msg.substring(4))){
        cfg->remove(msg.substring(4));
        log("Ok");
    }else{
        log("Err");
    }
}

void ITTIoT::updateFW(String url, imageType type) {

    String msg;
    t_httpUpdate_return ret;

    if(type == FIRMWARE){
        msg = "Starting FW update: ";
        ret = ESPhttpUpdate.update(url);
    }else if(type == SPIFFS){
        msg = "Starting SPIFFS update: ";
        ret = ESPhttpUpdate.updateSpiffs(url);
    }else{
        log("Error: unknown image type" );
        return;
    }
    msg += url;
    log(msg.c_str());

    switch(ret) {
    case HTTP_UPDATE_FAILED:
        msg = "HTTP_UPDATE_FAILD Error :" + String(ESPhttpUpdate.getLastError()) + " " +  ESPhttpUpdate.getLastErrorString();
        log(msg.c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        log("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        break;
    }

}

void ITTIoT::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if(String(topic).endsWith("/link")) return;

    char *buff = (char *)calloc(length + 1, 1);
    strncpy(buff,(const char*)payload, length);
    String msg = String(buff);

    if(String(topic).endsWith("/cfg")){
        if(msg.startsWith("get ")){
            config_get(msg);
        }else if (msg.startsWith("set ")) {
            config_set(msg);
        }else if(msg.startsWith("del ")){
            config_del(msg);
        }else if(msg.equals("save")){
            String j;
            cfg->printTo(j);
            if(IoTConfig::getInstance().setJsonConfig(j)){
                log("Ok");
            }else{
                log("Err");
            }
        }else if(msg.equals("list keys")){
            for(JsonObject::iterator it=cfg->begin(); it!=cfg->end(); ++it){
                log(it->key);
            }
        }else if(msg.equals("info")){
//            sendLocalInfo("info");
            sendLocalInfo();
        }else if(msg.equals("restart")){
            log("Restarting ...");
            delay(50);
            restart();
        }else if(msg.startsWith("fw ")){
            updateFW(msg.substring(3), FIRMWARE);
        }else if(msg.startsWith("fs ")){
            updateFW(msg.substring(3), SPIFFS);
        }else{
            log("?");
        }
    }else{
        if(iot_received){
            iot_received(topic, String(msg));
        };

    };
    free(buff);
}


void ITTIoT::setup()
{

    if(iot_init_cb) iot_init_cb();
    if(_printConfigFlag) _printConfig();


    if(_bootPin){
        pinMode(_bootPin, OUTPUT);
        digitalWrite(_bootPin, HIGH);
    }
    _iotboot = getBoot();
    _iotboot->setup();

    // setup mqtt
    // ssl ?
    if(cfg->get<bool>("mssl")){
        wifiClient = new WiFiClientSecure();
    }else{
        wifiClient = new WiFiClient();
    }

    mqttClient = new PubSubClient(cfg->get<const char*>("msrv"), cfg->get<unsigned int>("mport"), *wifiClient);
    mqttClient->setCallback([this] (char* topic, byte* payload, unsigned int length) { this->mqttCallback(topic, payload, length); });
    statTicker.attach(60, statTickerCB);

}

void statTickerCB(){
    iot.sendStat();
}

IoTBoot *ITTIoT::getBoot()
{
    _bootMode = normalMode;
    IoTConfig &iotcfg = IoTConfig::getInstance();

    // check config
    if(!IoTConfig::getInstance().isValid()){
        _bootMode = configMode;
    }

    // check boot pin
    if(_bootPin && !digitalRead(_bootPin)){
        _bootMode = configMode;
    }

    // check boot flag (from rtc)
    if(iotcfg.getBootFlag() == configMode){
        _bootMode = configMode;
    }

    if(_bootMode == normalMode){
        return new IoTBootNormal;
    }else{
        return new IoTBootConfig;
    }
}

void ITTIoT::handle()
{
    _iotboot->handle();

    // exit if
    if(_bootMode != normalMode) return;

    if (!mqttClient->connected()) {
        mqttReconnect();
    }

    if(_statFlag){
        _statFlag = false;
        sendStat("stat", true);
    }

    mqttClient->loop();

}

bool ITTIoT::setBootPin(uint8_t pin)
{
    if(pin > 0 && pin < 17){
        _bootPin = pin;
        return true;
    }
    return false;
}

const char* ITTIoT::getMainTopic(){
  return  cfg->get<const char *>("dname");
}

void ITTIoT::setBootFlag(uint8_t bootMode)
{
    IoTConfig::getInstance().setBootFlag(bootMode);
}

void ITTIoT::restart(uint8_t bootMode)
{
    setBootFlag(bootMode);
    delay(50);
    ESP.restart();
}

void ITTIoT::log(const char *msg)
{
    publishMsg("log", msg);
}

void ITTIoT::mqttReconnect()
{
    uint8_t attempts = 5;
    while (!mqttClient->connected()) {
        ArduinoOTA.handle();
        Serial.print("Connecting to MQTT server... ");

        char clientId[9], buffTopic[40];
        sprintf(clientId, "%X", ESP.getChipId());
        sprintf(buffTopic, "%s/%s", cfg->get<const char*>("dname"), "link");
        if (mqttClient->connect(clientId, cfg->get<const char*>("muser"), cfg->get<const char*>("mpass"), buffTopic, MQTTQOS1, true, "Offline")) {
            Serial.println("connected !");
            publishMsg("link", "Online", true);
            sendLocalInfo();

            subscribe("cfg");
            // exec iot_connected callback
            if(iot_received){
                iot_connected();
            };

        } else {
            delay(2500);
        }
        //        if(attempts-- == 0) break;
        if(attempts-- == 0) restart(configMode);
    }
}

void ITTIoT::sendLocalInfo(const char *to)
{
    const char *topic = "log";
    if(to != NULL) topic=to;

    // Saadame info seadme kohta
    String msg;
    this->publishMsg(topic, "Connected FW build date: " __DATE__ " " __TIME__);

    msg = "WIFI AP SSID: " + WiFi.SSID() + ", BSSID: " + WiFi.BSSIDstr() + ", SIGNAL: " + String(WiFi.RSSI());
    this->publishMsg(topic, msg.c_str());

    msg = "WIFI interface mode: ";
    switch (WiFi.getPhyMode()) {
    case WIFI_PHY_MODE_11B:
        msg += "11B";
        break;
    case WIFI_PHY_MODE_11G:
        msg += "11G";
        break;
    case WIFI_PHY_MODE_11N:
        msg += "11N";
        break;
    default:
        break;
        msg += "UNKNOWN";
    }
    msg += ", MAC: " + WiFi.macAddress();
    this->publishMsg(topic, msg.c_str());

    msg = "IP: " + WiFi.localIP().toString() + ", MASK: " + WiFi.subnetMask().toString() + ", GW: " +
            WiFi.gatewayIP().toString();
    publishMsg(topic, msg.c_str());

}

void ITTIoT::sendStat(String topic, bool retain)
{
    st.rssi = WiFi.RSSI();
    st.freeHeap = ESP.getFreeHeap();
    st.uptime = millis()/1000;
    char bufmsg[40];
    sprintf(bufmsg, "%u\t%d\t%u\t%u\t%u", st.uptime, st.rssi, st.msg, st.msgOk, st.freeHeap);
    Serial.println(bufmsg);
    publishMsg(topic.c_str() ,(const char *)bufmsg, retain);
}

void ITTIoT::_printConfig()
{
    cfg->prettyPrintTo(Serial);
    Serial.println();
}

void ITTIoT::publishMsg(const char* topic,const char *msg, bool retain){
    publishMsg(topic, (uint8_t *)msg, strlen(msg), retain);
}

void ITTIoT::publishMsgTo(const char* topic,const char *msg, bool retain){
    publishMsgTo(topic, (uint8_t *)msg, strlen(msg), retain);
}


void ITTIoT::publishMsg(const char* topic,const uint8_t *msg, uint32_t len, bool retain){
    st.msg++;
    if(!WiFi.isConnected()) return;
    if(!mqttClient->connected()) return;

    char buffTopic[40];
    sprintf(buffTopic, "%s/%s", cfg->get<const char *>("dname"), topic);
    if(mqttClient->publish(buffTopic , msg, len, retain)) st.msgOk++;
}

void ITTIoT::publishMsgTo(const char* topic,const uint8_t *msg, uint32_t len, bool retain){
    st.msg++;
    if(!WiFi.isConnected()) return;
    if(!mqttClient->connected()) return;

    char buffTopic[40];
    sprintf(buffTopic, "%s", topic);
    if(mqttClient->publish(buffTopic , msg, len, retain)) st.msgOk++;
}


void ITTIoT::subscribe(const char *topic)
{
    char buffTopic[40];
    sprintf(buffTopic, "%s", topic);
    mqttClient->subscribe(buffTopic);
}


ITT::ITTIoT iot;
