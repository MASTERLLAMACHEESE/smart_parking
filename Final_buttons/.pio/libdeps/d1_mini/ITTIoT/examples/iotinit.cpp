#include <Arduino.h>
#include <ittiot.h>

// use this
void iot_init_cb(){

    iot.cfg->set("custom", "123");
    iot.cfg->set("PPM", 665.33);

}
