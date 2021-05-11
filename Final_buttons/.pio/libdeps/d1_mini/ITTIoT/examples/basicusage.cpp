#include <Arduino.h>
#include <ittiot.h>

void iot_received(String topic, String msg){
    Serial.print("MSG FROM USER callback, topic: ");
    Serial.print(topic);
    Serial.print(" payload: ");
    Serial.println(msg);
}

void iot_connected(){
    Serial.println("MQTT connected callback");
    iot.subscribe("test");
    iot.log("Hello from ESP!");
}

void setup() {
    Serial.begin(115200);
    iot.printConfig(); // print json config to serial
    iot.setBootPin(5);
    iot.setup();
}

void loop() {
    iot.handle();
}
