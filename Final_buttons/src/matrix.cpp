#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>
#include <Adafruit_GFX.h>
#include <WEMOS_Matrix_GFX.h>
#include <SPI.h>

#define MODULE_TOPIC_IN "ESP35/btn_in"
#define MODULE_TOPIC_PIR1 "ESP31/pir1" //pir1
#define MODULE_TOPIC_PIR2 "ESP08/pir2" //pir2
#define WIFI_NAME "Kohalik-WIFI"
#define WIFI_PASSWORD "PlayStation4"

MLED matrix(7); //set intensity=7 (maximum)

String in_data;
String str_pir1;
String str_pir2;
int pir1 = 2;
int pir2 = 2;
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

  //get vehicle in out status
  if (topic == MODULE_TOPIC_IN)
  {
    in_data = getValue(msg,';',0);
  }
  //get data from pir 1
  if(topic == MODULE_TOPIC_PIR1)
  {
    str_pir1 = getValue(msg,';',0);
    if (str_pir1 == "Kinni"){
      iot.log("pir1 kinni");
      pir1 = 1;
    }else if(str_pir1 == "Vaba"){
      pir1 = 0;
    }
  }
  //get data from pir 2
  if(topic == MODULE_TOPIC_PIR2)
  {
    str_pir2 = getValue(msg,';',0);
    if (str_pir2 == "Kinni"){
      iot.log("pir2 kinni");
      pir2 = 1;
    }else if(str_pir2 == "Vaba"){
      pir2 = 0;
    }
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  Serial.println("MQTT connected callback");
  iot.subscribe(MODULE_TOPIC_IN);
  iot.subscribe(MODULE_TOPIC_PIR1);
  iot.subscribe(MODULE_TOPIC_PIR2);
  iot.log("IoT MATRIX!");
}

void setup()
{
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  // Print json config to serial
  iot.printConfig();
  // Initialize IoT library
  iot.setup();

}

void loop(){
  iot.handle();
  int i;
  if (in_data != "" && pir1 != 2 && pir2 != 2){
    i = in_data.toInt();
    if(i == 1)//ehk parklas on üks auto
    {
      if (pir1 == 1 || pir2 == 1)
      {
        matrix.clear(); // Clear the matrix field
        matrix.drawLine(2, 3, 5, 0, LED_ON); // arv 1
        matrix.drawLine(5, 0, 5, 7, LED_ON); //arv 1
        matrix.writeDisplay();  // Write the changes we just made to the display
      }
    }
    else if (i>=2) //ehk parklas on kaks autot
    {
      if (pir1 == 1 && pir2 == 1)
      {
      matrix.clear(); // Clear the matrix field
      matrix.drawRect(2, 0, 4, 8, LED_ON); // arv 0
      matrix.writeDisplay();  // Write the changes we just made to the display
      }
    }
    else if (i<=0) {
      if (pir1 == 0 && pir2 == 0)
      {
        matrix.clear(); // Clear the matrix field
        matrix.drawLine(1, 2, 3, 0, LED_ON); //arv 2
        matrix.drawLine(3, 0, 4, 0, LED_ON); //arv 2
        matrix.drawLine(4, 0, 6, 2, LED_ON); //arv 2
        matrix.drawLine(6, 2, 1, 7, LED_ON); //arv 2
        matrix.drawLine(1, 7, 6, 7, LED_ON); //arv 2
        matrix.writeDisplay();  // Write the changes we just made to the display
      }
    }
  }
  delay(50);
}
