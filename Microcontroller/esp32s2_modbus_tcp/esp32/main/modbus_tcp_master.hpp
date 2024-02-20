/* libraries */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

/* user defined constants */

// comment whichever one you want, self-explanatory
#define ESP_DEBUG_MODE 1
//#undef ESP_DEBUG_MODE

// default modbus communication port
#define MB_TCP_PORT             502

// wifi station setup
#define WIFI_SSID               "adao4"
#define WIFI_PASS               "12345678"
#define ESP_MAX_RETRIES         100
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

// modbus pvinverter
#define UID_MB_VE_PVI 3
#define MB_AC_L1_V_REG_START 1027
#define MB_AC_L1_I_REG_START 1028
#define NUMBER_OF_MB_DEVICES 1

// user defined enum for parameters capable of being read / written
enum
{
    CID_AC_L1_V = 0,
    CID_AC_L1_I = 1
};

#define NUMBER_OF_CIDS 2

/* modbus controller class */

class ModbusController
{
    public:
        ModbusController();
        void initialize_esp32();
        void destroy_esp32();
        void testrun_master(uint16_t new_value_to_set);

    private:
        #ifdef ESP_DEBUG_MODE
            char* _tag; 
            esp_err_t _esp_err; 
        #endif

        char* _mb_device_ip_addresses[NUMBER_OF_MB_DEVICES + 1];
        mb_communication_info_t _communication_information;
        void* _master_handler;
        mb_parameter_descriptor_t _parameter_descriptors[NUMBER_OF_CIDS];

        static void wifi_event_handler_jumper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
};