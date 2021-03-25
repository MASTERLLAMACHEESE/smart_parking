#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>

#define MODULE_TOPIC "rgb"
#define WIFI_NAME "Kohalik-WIFI"
#define WIFI_PASSWORD "PlayStation4"

const byte buttonPin = D3; // TO which pin the button has been assigned
int i;

Switch button = Switch(buttonPin);

// Function started after the connection to the server is established.
void iot_connected()
{
  Serial.println("MQTT connected callback");
  iot.log("IoT Button example!");
}

void setup()
{
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  pinMode(buttonPin, INPUT);

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  // Print json config to serial
  iot.printConfig();
  // Initialize IoT library
  iot.setup();
}

void loop()
{
  // IoT behind the plan work, it should be periodically called
  iot.handle();

  // Askes in which state the button is, pressed, long pressed, double click, or released.
  button.poll();

  //the following was changed and code added to send messages to led 

  float value; //value to store for color
  char buf[10]; //buffer to hold data for messages
  // If the button is pushed down, it publishes message to the led to emite light
  if (button.pushed()) {
    iot.log("ButtonPushed"); //log that the button was pressed
    value = 255; //color value
    String(value).toCharArray(buf,10); //change float to string for message
    iot.publishMsg(MODULE_TOPIC, buf); //send data in buf to topic space
    Serial.println(buf); //serial check for buf data
    Serial.println("click"); //serial check that button was pressed

  // If the button is released, it publishes message to turn the led off
  }else if (button.released()){
    iot.log("ButtonReleased"); //log to show button was released
    value = 000; //value to turn led off
    String(value).toCharArray(buf,10); //change float to string for message
    iot.publishMsg(MODULE_TOPIC, buf); //send data in buf to topic space
  }
}
