#include "Arduino.h"
#include "SPImaster.hpp"

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
    SPImaster spiMaster;
    while(1)
    {
        spiMaster.write(R8_DC_BATTERY, CONFIG, 0x2f);
    }
}