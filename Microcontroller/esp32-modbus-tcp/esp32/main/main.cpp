#include "esp32.hpp"

extern "C"
{
    #include <stdio.h>
}

extern "C"
{
    void app_main(void);
}

void app_main(void)
{ 
    ESP32 esp32;

    esp32.initialize_all();

    uint16_t set_value = 0;

    while (set_value < 1000)
    {
        esp32.testrw(CID_BATTERY_POWER, set_value);
        esp32.testrw(CID_BATTERY_VOLTAGE, set_value);
        esp32.testrw(CID_BATTERY_CURRENT, set_value);
        set_value++;
    };

    esp32.destroy_all();
}

