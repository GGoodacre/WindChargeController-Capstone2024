// ControllerTCP.hpp

// HardwareControl
#include "HardwareControl.hpp"
// c libraries
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>
// esp-idf libraries
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
// esp-modbus libraries
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

// defines
#define RASPBERRY_IP "192.168.229.169" 
#define MAX_RETRIES 10
#define WIFI_SSID "adao4"
#define WIFI_PASS "12345678"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define MB_DEV_NUM 7
#define CID_NUM 4*7

enum {
    SHUNT1_UID = 238, // 289
    SHUNT2_UID = 237, // 290
    SHUNT3_UID = 236, // 291
    SHUNT4_UID = 235, // 292
    SHUNT5_UID = 233, // 293
    SHUNT6_UID = 232, // 294
    SHUNT7_UID = 231, // 295
};

enum {
    BATTERY_POWER_REG = 258,
    BATTERY_VOLTAGE_REG = 259,
    BATTERY_CURRENT_REG = 261,
    BATTERY_TEMPERATURE_REG = 262
};

enum {
    BATTERY_POWER_SF = 1,
    BATTERY_VOLTAGE_SF = 100,
    BATTERY_CURRENT_SF = 10,
    BATTERY_TEMPERATURE_SF = 10
};

enum {
    SHUNT1_POWER_CID = 0,
    SHUNT1_VOLTAGE_CID,
    SHUNT1_CURRENT_CID,
    SHUNT1_TEMPERATURE_CID,

    SHUNT2_POWER_CID,
    SHUNT2_VOLTAGE_CID,
    SHUNT2_CURRENT_CID,
    SHUNT2_TEMPERATURE_CID,

    SHUNT3_POWER_CID,
    SHUNT3_VOLTAGE_CID,
    SHUNT3_CURRENT_CID,
    SHUNT3_TEMPERATURE_CID,

    SHUNT4_POWER_CID,
    SHUNT4_VOLTAGE_CID,
    SHUNT4_CURRENT_CID,
    SHUNT4_TEMPERATURE_CID,

    SHUNT5_POWER_CID,
    SHUNT5_VOLTAGE_CID,
    SHUNT5_CURRENT_CID,
    SHUNT5_TEMPERATURE_CID,

    SHUNT6_POWER_CID,
    SHUNT6_VOLTAGE_CID,
    SHUNT6_CURRENT_CID,
    SHUNT6_TEMPERATURE_CID,

    SHUNT7_POWER_CID,
    SHUNT7_VOLTAGE_CID,
    SHUNT7_CURRENT_CID,
    SHUNT7_TEMPERATURE_CID,
};

class ControllerTCP {
    public:
        ControllerTCP(HardwareControl &hw); // testing passing variable by reference
        HardwareControl &hw; // test variable in class
        void send_data();

    private:
        const char* _tag;

        bool _connected_to_wifi;
        void* _master_handler = NULL;
        const char* _mb_dev_ip_addrs[MB_DEV_NUM + 1];
        EventGroupHandle_t _s_wifi_event_group;
        
        mb_communication_info_t _mb_comm_info;
        mb_parameter_descriptor_t _param_descs[CID_NUM];

        TaskHandle_t _tcp_console;
        struct sockaddr_storage dest_addr;
        struct sockaddr_storage source_addr; 
        int listen_sock; 
        int console_sock; 
        static const constexpr uint16_t PORT = 49160; 
        static const constexpr uint16_t MAX_CONNECTION_ATTEMPTS = 2; 
        static const constexpr uint8_t SOCK_KEEPALIVE_IDLE = 5;
        static const constexpr uint8_t SOCK_KEEPALIVE_INTERVAL = 5;
        static const constexpr uint8_t SOCK_KEEPALIVE_COUNT = 3;

        void init_netif_helper();
        static void wifi_connect_hdl_en(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void wifi_connect_hdl(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        bool open_tcp_socket();
        bool bind_tcp_socket();
        bool listen_tcp_socket();
        static void run_tcp_console_trampoline(void *arg); 
        void run_tcp_console();

        void init_mb_master_helper();    
};