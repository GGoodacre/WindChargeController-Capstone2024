#include <stdio.h>
#include <inttypes.h>

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "esp_modbus_slave.h"
#include "esp_modbus_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define WIFI_SSID "adao4"
#define WIFI_PASS "123545678"
#define WIFI_FAIL_BIT BIT0
#define WIFI_CONNECTED_BIT BIT1

#define MODBUS_SLAVE_ADDRESS 100

#define NUMBER_OF_DATA 8

#define MODBUS_PARAM_INFO_GET_TIMEOUT 10
#define MODBUS_CHANNEL_DATA_MAX_VALUE 10
#define MODBUS_CHANNEL_DATA_OFFSET 1.1f               

#define MODBUS_READ_MASK                        (MB_EVENT_INPUT_REG_RD \
                                                | MB_EVENT_HOLDING_REG_RD \
                                                | MB_EVENT_DISCRETE_RD \
                                                | MB_EVENT_COILS_RD)

#define MODBUS_WRITE_MASK                       (MB_EVENT_HOLDING_REG_WR \
                                                | MB_EVENT_COILS_WR)
#define MODBUS_READ_WRITE_MASK                  (MODBUS_READ_MASK | MODBUS_WRITE_MASK)

#define MODBUS_TCP_PORT 502

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;

struct modbus_tcp 
{
    const char* tag;
    esp_err_t esp_error_result;

    EventGroupHandle_t wifi_event_group;
    mb_communication_info_t comm_info;
    int number_of_retries;
    int max_retries;

    mb_register_area_descriptor_t register_area;
    float register_data[NUMBER_OF_DATA];

    mb_param_info_t register_info;
};

esp_err_t init_nvs(struct modbus_tcp mbc);

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data, struct modbus_tcp mbc);

esp_err_t init_netif(struct modbus_tcp mbc);

esp_err_t init_slave(struct modbus_tcp mbc);

void slave_communication(void *arg, struct modbus_tcp mbc);

esp_err_t destroy_slave(struct modbus_tcp mbc);

esp_err_t destroy_services(struct modbus_tcp mbc);