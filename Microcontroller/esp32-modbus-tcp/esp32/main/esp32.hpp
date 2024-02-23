/* c libraries */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>

/* esp-idf libraries */

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

/* wifi network interface user definitions */

#define MB_TCP_PORT             502
#define WIFI_SSID               "adao4"
#define WIFI_PASS               "12345678"
#define ESP_MAX_RETRIES         100
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

/* modbus tcp controller user definitions*/

#define UID_SMART_SHUNT         245
#define REG_BATTERY_POWER       258
#define REG_BATTERY_VOLTAGE     259
#define REG_BATTERY_CURRENT     261
#define MB_DEV_CNT              1
#define CID_CNT                 3

/* parameter characteristic ids */

enum
{
    CID_BATTERY_POWER = 0,
    CID_BATTERY_VOLTAGE = 1,
    CID_BATTERY_CURRENT = 2,
};

/* esp32 class definition */

class ESP32
{
    public:

        ESP32();

        void initialize_all();

        void destroy_all();

        void testrw(uint16_t cid, uint16_t set_value);

    private:

        char* _mb_dev_ip_addrs[MB_DEV_CNT];

        mb_communication_info_t _comm_info;

        const char* _netif_tag;

        int _wifi_retry_cnt;

        static void netif_event_handler_jumper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        
        void netif_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        const char* _mbtcp_tag;
        
        mb_parameter_descriptor_t _parameter_descriptors[CID_CNT];
        
};