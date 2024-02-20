#include "modbus_tcp_master.hpp"

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
    ModbusController mbc;
    mbc.initialize_esp32();

    /* garnett place i guess */
    uint16_t new_value_to_set = 0;
    while (new_value_to_set < 1000)
    {
        mbc.testrun_master(new_value_to_set);
        new_value_to_set++;
        vTaskDelay(60);
    };

    mbc.destroy_esp32();
}

