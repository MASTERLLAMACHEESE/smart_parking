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


  // If the button is pushed down, it publishes message “ButtonPushed”
  float value;
    char buf[10];
    // If the button is pushed down, it publishes message “ButtonPushed”
    if (button.pushed()) {
      value = 255;
      String(value).toCharArray(buf,10);
      iot.log("ButtonPushed");
      iot.publishMsg(MODULE_TOPIC, buf);
      Serial.println(buf);
      Serial.println("click");

    }else if (button.released()){
      value = 000;
      String(value).toCharArray(buf,10);
      iot.log("ButtonReleased");
      iot.publishMsg(MODULE_TOPIC, buf);
    }
}
