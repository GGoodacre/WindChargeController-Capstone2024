#include "Arduino.h"
#include "HardwareControl.hpp"

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


    HardwareControl hw;

    while(1)
    {

    }
}
