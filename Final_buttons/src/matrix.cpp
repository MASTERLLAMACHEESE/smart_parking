//https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives

#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>
#include <Adafruit_GFX.h>
#include <WEMOS_Matrix_GFX.h>
#include <SPI.h>

#define MODULE_TOPIC_IN "ESP60/btn_in"
#define WIFI_NAME "Kohalik-WIFI"
#define WIFI_PASSWORD "PlayStation4"

MLED matrix(7); //set intensity=7 (maximum)

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

  int i;
  if (topic == MODULE_TOPIC_IN)
  {
    String in = getValue(msg,';',0);
    i = in.toInt();
    if(i == 1)//ehk parklas on üks auto
    {
      //value_out = 1;//ehk parklas on üks auto
      matrix.clear(); // Clear the matrix field
      matrix.drawLine(3, 4, 6, 1, LED_ON); // arv 1
      matrix.drawLine(6, 1, 6, 8, LED_ON); //arv 1
      matrix.writeDisplay();  // Write the changes we just made to the display
    }
    else if (i>=2) //ehk parklas on kaks autot
    {
      matrix.clear(); // Clear the matrix field
      matrix.drawRect(3, 1, 4, 8, LED_ON); // arv 0
      matrix.writeDisplay();  // Write the changes we just made to the display
    }
    else if (i<=0) {
      matrix.clear(); // Clear the matrix field
      matrix.drawLine(2, 3, 4, 1, LED_ON); //arv 2
      matrix.drawLine(4, 1, 5, 1, LED_ON); //arv 2
      matrix.drawLine(5, 1, 7, 3, LED_ON); //arv 2
      matrix.drawLine(7, 3, 2, 8, LED_ON); //arv 2
      matrix.drawLine(2, 8, 7, 8, LED_ON); //arv 2
      matrix.writeDisplay();  // Write the changes we just made to the display
    }
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  Serial.println("MQTT connected callback");
  iot.subscribe(MODULE_TOPIC_IN);
  iot.log("IoT MATRIX!");
}

void setup()
{
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  //iot.setConfig("wname", WIFI_NAME);
  //iot.setConfig("wpass", WIFI_PASSWORD);
  // Print json config to serial
  iot.printConfig();
  // Initialize IoT library
  iot.setup();

}

void loop(){
  iot.handle();
  matrix.clear(); // Clear the matrix field
  matrix.drawLine(2, 3, 4, 1, LED_ON); //arv 2
  matrix.drawLine(4, 1, 5, 1, LED_ON); //arv 2
  matrix.drawLine(5, 1, 7, 3, LED_ON); //arv 2
  matrix.drawLine(7, 3, 2, 8, LED_ON); //arv 2
  matrix.drawLine(2, 8, 7, 8, LED_ON); //arv 2
  matrix.writeDisplay();
  delay(200);
}
