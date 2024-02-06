
/* INCLUDED HEADERS */

// included by default upon project creation
#include <stdio.h>

// includes from modbus tcp api reference
#include "esp_modbus_slave.h"
#include "esp_modbus_common.h"

// includes for esp networking functionality
#include "esp_netif.h"

// includes to fix dumb logging type issue
#include <inttypes.h>

/* DEFINED CONSTANTS */

#define MB_REG_HOLDING_START_AREA0  (0)
#define MB_REG_HOLD_CNT             (100)                          
#define MB_TCP_PORT                 (502)    

#define MB_READ_MASK                (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD)
#define MB_WRITE_MASK               (MB_EVENT_HOLDING_REG_WR)
#define MB_READ_WRITE_MASK          (MB_READ_MASK | MB_WRITE_MASK)
#define portTICK_RATE_MS            (0.001)                                                                                 // arbitrarily declared
#define MB_PAR_INFO_GET_TOUT        (10 / portTICK_RATE_MS)

void app_main(void)
{

    static const char* TAG = "SLAVE_TEST"; 

    /* MODBUS PORT INITIALIZATION */
    void* slave_handler = NULL;                                                                                             // pointer to allocate interface structure
    esp_err_t slave_err = mbc_slave_init_tcp(&slave_handler);                                                               // initialization of Modbus slave for TCP
    if (slave_handler == NULL || slave_err != ESP_OK) {                                                                     // error handling for slave initialization
        ESP_LOGE(TAG, "mb controller initialization fail.");        
    }

    /* CONFIGURING SLAVE DATA ACCESS */
    mb_register_area_descriptor_t reg_area;                                                                                 // modbus register area descriptor structure
    uint16_t holding_reg_area[MB_REG_HOLD_CNT] = {0};                                                                       // storage area for holding registers

    reg_area.type = MB_PARAM_HOLDING;                                                                                       // set type of register area
    reg_area.start_offset = MB_REG_HOLDING_START_AREA0;                                                                     // offset of register area in Modbus protocol
    reg_area.address = (void*)&holding_reg_area[0];                                                                         // set pointer to storage instance
    reg_area.size = sizeof(holding_reg_area) << 1;                                                                          // set the size of register storage area in bytes
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));                                                                    // error handling for register initialization

    /* SLAVE COMMUNICATION OPTIONS */
    esp_netif_init();

    void* esp_netif_ptr = NULL; 
    mb_communication_info_t comm_info = {
        .ip_port = MB_TCP_PORT,                                                                                             // modbus tcp port number
        .ip_addr_type = MB_IPV4,                                                                                            // version of ip protocol
        .ip_mode = MB_MODE_TCP,                                                                                             // port communication mode
        .ip_addr = NULL,                                                                                                    // this field keeps the client ip address to bind, NULL - bind to any client
        .ip_netif_ptr = esp_netif_ptr                                                                                       // esp_netif_ptr - pointer to the corresponding network interface
    };

    ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));                                                                    // setup communication parameters and start stack

    /* SET REGISTER VALUES? */

    /* SLAVE COMMUNICATION */
    ESP_ERROR_CHECK(mbc_slave_start());

    mb_event_group_t event = mbc_slave_check_event(MB_READ_WRITE_MASK);                                                     // the function blocks while waiting for register access

    mb_param_info_t reg_info;                                                                                               // declare the register parameter info structure
    ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));                                             // get information about data accessed from master
    const char* rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";
                                                                                 // declare the tag variable in each c file that uses logging functionality
    if (event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {                                                      // filter events and process them accordingly
        ESP_LOGI(TAG, "HOLDING %s (%"PRIu32" us), ADDR:%"PRIu32", TYPE:%"PRIu32", INST_ADDR:0x%"PRIu32", SIZE:%"PRIu32"",   // %u does not work since IDF v5.1 onwards, referring to equivalent real pri macro
                    rw_str,
                    (uint32_t)reg_info.time_stamp,
                    (uint32_t)reg_info.mb_offset,
                    (uint32_t)reg_info.type,
                    (uint32_t)reg_info.address,
                    (uint32_t)reg_info.size);
    }

    /* MODBUS SLAVE TEARDOWN */
    ESP_ERROR_CHECK(mbc_slave_destroy());                                                                                   // stops the modbus communication stack, destroys the controller interface, and frees all used active objects allocated for the slave
}
