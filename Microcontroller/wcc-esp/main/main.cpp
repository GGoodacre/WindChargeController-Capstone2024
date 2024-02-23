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
    long data;
    spiMaster.config(R9_DC_DUMP);
    spiMaster.write(R9_DC_DUMP, CONFIG, 0x8000);
    while(1)
    {
        /*
        spiMaster.read(R9_DC_DUMP, CONFIG, data);
        spiMaster.read(R9_DC_DUMP, ADC_CONFIG, data);
        spiMaster.read(R9_DC_DUMP, SHUNT_CAL, data);
        spiMaster.read(R9_DC_DUMP, VBUS, data);
        */
        spiMaster.read(R9_DC_DUMP, MANUFACTURER_ID, data);
        delayMicroseconds(1000000); 
    }
/*
    powerElec.setDUTY(0, RECTIFIER_INDEX);
    delayMicroseconds(100);
    powerElec.setDUTY(30, SEPIC_INDEX);
    delayMicroseconds(100);
    powerElec.setDUTY(99, PS1_INDEX);
    delayMicroseconds(100);
    powerElec.setDUTY(99, PS2_INDEX);
*/
}
