/*
 * IoT PIR example
 *
 * This example subscribe to the "pir" topic. When a message received, then it
 * will switch the pir module "ON" and "OFF"
 *
 * Created 21 Febuary 2018 by Heiko Pikner and Mallor Kingsepp
 */

// Includes global variables and librarys that the PIR shield uses
#include <Arduino.h>
#include <ittiot.h>

#define MODULE_TOPIC_PIR2 "pir"
#define WIFI_NAME "Siidisaba7"
#define WIFI_PASSWORD "Varst1onsuv1!"

//Pin definition for the PIR (GPIO14)
#define PIR_PIN D5
//Pin definition for the PIR LED (GPIO16)
#define PIR_LED_PIN D4

// PIR state for detection
bool pirState;
// State that switches PIR on and off
bool onState;
int i = 0;
int j = 0;
// If message received for PIR topic. For example:
// mosquitto_pub -u test -P test -t "ITT/IOT/3/pir" -m "1"
void iot_received(String topic, String msg)
{
  Serial.print("MSG FROM USER callback, topic: ");
  Serial.print(topic);
  Serial.print(" payload: ");
  Serial.println(msg);

  if(topic == MODULE_TOPIC_PIR2)
  {
    // Switching the PIR shield on or off, depending what message is received
    if(msg == "1")
    {
  	   onState = true;
    }
    if(msg == "0")
    {
  	   onState = false;
    }
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  Serial.println("MQTT connected callback");
  // Subscribe to the topic "pir"
  iot.subscribe(MODULE_TOPIC_PIR2);
  iot.log("IoT PIR example!");
}

void setup()
{
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.printConfig(); // Print json config to serial
  iot.setup(); // Initialize IoT library

  // Initialize PIR pin
  pinMode(PIR_PIN, INPUT);
  pinMode(PIR_LED_PIN, OUTPUT);
}

void loop()
{
  iot.handle(); // IoT behind the plan work, it should be periodically called
  delay(200); // Wait 0.2 seconds

  if(onState == true){
    // This part of the code is executed, when PIR shield is active
    if(digitalRead(PIR_PIN))
    {
      if(pirState == false)
      {
        // When PIR has detected motion, then the LED is switched on and text “Motion detected!” is published to the MQTT broker
        digitalWrite(PIR_LED_PIN, HIGH);
        // String msg = String("Liikumine");
        // iot.publishMsg("pir", msg.c_str());
        if (i == 0){
          String msg = String("Kinni");
          iot.publishMsg("pir", msg.c_str());
          i = 1;
          j = 0;
          delay (2000);
        }
        if (i == 1 && j == 1){
          String msg = String("Vaba");
          iot.publishMsg("pir", msg.c_str());
          i = 0;
          delay (2000);
        }
        //Serial.println(msg);
        pirState = true;
      }
    }
    else
    {
      if(pirState == true)
      {
        // PIR shields LED is switched off, when it is not detecting any motion
        digitalWrite(PIR_LED_PIN, LOW);
        pirState = false;
        j = 1;
        delay (1000);
      }
    }
  }
  else{
    // When the PIR shield has been switched off, then its offline state is sent to the MQTT broker
    iot.log("PIR offline");
    delay(2000); // Waiting 2 secondes
  }
  delay (500);
}
