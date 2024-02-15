#include "Arduino.h"
#include "SPImaster.hpp"
#include "PowerElectronics.hpp"

extern "C"
{
    #include <stdio.h>
    #include "esp_task_wdt.h"
}

extern "C"
{
    void app_main(void);
}



void app_main(void)
{


    SPImaster spiMaster;
    PowerElectronics powerElec;

    powerElec.setDUTY(30, RECTIFIER_INDEX);
    delayMicroseconds(100);
    powerElec.setDUTY(30, SEPIC_INDEX);
    delayMicroseconds(100);
    powerElec.setDUTY(30, LOAD_INDEX);
    while(1)
    {
        /*
            powerElec.setDUTY(30, RECTIFIER_INDEX);
            delayMicroseconds(100);
            powerElec.setDUTY(50, SEPIC_INDEX);
            delayMicroseconds(100);
            powerElec.setDUTY(70, LOAD_INDEX);
            delayMicroseconds(1000000);
            powerElec.setDUTY(10, RECTIFIER_INDEX);
            delayMicroseconds(100);
            powerElec.setDUTY(80, SEPIC_INDEX);
            delayMicroseconds(100);
            powerElec.setDUTY(40, LOAD_INDEX);
            delayMicroseconds(1000000);
        */
    }
}