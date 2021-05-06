#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>

#define MODULE_TOPIC_IN "ESP60/btn_in"
#define MODULE_TOPIC_OUT "ESP07/btn_out"
#define WIFI_NAME "Siidisaba7"
#define WIFI_PASSWORD "Varst1onsuv1"

const byte buttonPin = D3; // TO which pin the button has been assigned
int i;

Switch button = Switch(buttonPin);
float value_out;
float value_in = 0;
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

  //iot.setConfig("wname", WIFI_NAME);
  //iot.setConfig("wpass", WIFI_PASSWORD);
  // Print json config to serial
  iot.printConfig();
  // Initialize IoT library
  iot.setup();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++)
  {
    if(data.charAt(i)==separator || i==maxIndex)
    {
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void iot_received(String topic, String msg)
{
  Serial.print("MSG FROM USER callback, topic: ");
  Serial.print(topic);
  Serial.print(" payload: ");
  Serial.println(msg);

  if(topic == MODULE_TOPIC_OUT)
  {
    // getting the value of out from topic
    value_out = 1;
  }
}

void loop()
{
  // IoT behind the plan work, it should be periodically called
  iot.handle();

  // Askes in which state the button is, pressed, long pressed, double click, or released.
  button.poll();

  char buf[10];
  // If the button is pushed down, it publishes message “ButtonPushed”
  if (value_in != 0){
    if (value_out >= 1){
      value_in = value_in - 1;
      value_out = 0;
    }
  }
  if (button.pushed()) {
    value_in = value_in + 1;
    iot.log("ButtonPushed");
  }else if (button.released()){
    iot.log("ButtonReleased");
  }
  String(value_in).toCharArray(buf,10);
  iot.publishMsg(MODULE_TOPIC_IN, buf);
  delay (500);
}
