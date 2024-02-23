#include "esp32.hpp"

/* ESP32 */

ESP32::ESP32()
{
    /* common */
    char* ip_addrs[MB_DEV_CNT + 1] = 
    {
        "192.168.81.169",
        NULL
    };

    memcpy(_mb_dev_ip_addrs, ip_addrs, sizeof(_mb_dev_ip_addrs));

    _comm_info.ip_mode = MB_MODE_TCP;
    _comm_info.ip_port = MB_TCP_PORT;
    _comm_info.ip_addr_type = MB_IPV4;
    _comm_info.ip_addr = (void*)_mb_dev_ip_addrs;
    _comm_info.ip_netif_ptr = NULL; 

    _netif_tag = "Wifi Network Interface";

    _wifi_retry_cnt = 0;

    _mbtcp_tag = "Modbus TCP Controller";

    mb_parameter_descriptor_t descriptors[CID_CNT] = 
    {
        {
            .cid = CID_BATTERY_POWER,
            .param_key = "Battery Power",
            .param_units = "W",
            .mb_slave_addr = UID_SMART_SHUNT,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_reg_start = REG_BATTERY_POWER,
            .mb_size = 1,
            .param_offset = 0,
            .param_type = PARAM_TYPE_U16,
            .param_size = PARAM_SIZE_U16,
            .param_opts = {0, 0, 0},
            .access = PAR_PERMS_READ_WRITE_TRIGGER
        },

        {
            .cid = CID_BATTERY_VOLTAGE,
            .param_key = "Battery Voltage",
            .param_units = "V",
            .mb_slave_addr = UID_SMART_SHUNT,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_reg_start = REG_BATTERY_VOLTAGE,
            .mb_size = 1,
            .param_offset = 0,
            .param_type = PARAM_TYPE_U16,
            .param_size = PARAM_SIZE_U16,
            .param_opts = {0, 0, 0},
            .access = PAR_PERMS_READ_WRITE_TRIGGER
        },

        {
            .cid = CID_BATTERY_CURRENT,
            .param_key = "Battery Current",
            .param_units = "A",
            .mb_slave_addr = UID_SMART_SHUNT,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_reg_start = REG_BATTERY_CURRENT,
            .mb_size = 1,
            .param_offset = 0,
            .param_type = PARAM_TYPE_U16,
            .param_size = PARAM_SIZE_U16,
            .param_opts = {0, 0, 0},
            .access = PAR_PERMS_READ_WRITE_TRIGGER
        },
    };

    memcpy(_parameter_descriptors, descriptors, sizeof(descriptors));
}

void ESP32::netif_event_handler_jumper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP32* local_event_handler = (ESP32*) arg;
    local_event_handler -> netif_event_handler(arg, event_base, event_id, event_data);
}

void ESP32::netif_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_ERROR_CHECK(esp_wifi_connect());
        ESP_LOGI(_netif_tag, "trying connection to wifi");
    } 
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_ERROR_CHECK(esp_wifi_connect());
        ESP_LOGI(_netif_tag, "retrying connection to wifi");
        _wifi_retry_cnt++;
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* got_ip_event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(_netif_tag, "got ip:" IPSTR, IP2STR(&got_ip_event->ip_info.ip));
        _wifi_retry_cnt = 0;
    }
}

void ESP32::initialize_all()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    vTaskDelay(50 / portTICK_PERIOD_MS);

    esp_event_handler_instance_t instance_any_id;

    esp_event_handler_instance_t instance_got_ip;
    
    const wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    wifi_config_t wifi_cfg = {.sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};
    
    ESP_ERROR_CHECK(esp_netif_init());
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    _comm_info.ip_netif_ptr = (void*)esp_netif_create_default_wifi_sta();
    
    assert(_comm_info.ip_netif_ptr);
    
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler_jumper, this, &instance_any_id));
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &netif_event_handler_jumper, this, &instance_got_ip));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    
    ESP_ERROR_CHECK(esp_wifi_start());
    
    vTaskDelay(10000 / portTICK_PERIOD_MS); // just give it some time to connect to wifi, will improve later

    static void* master_handler;
    
    ESP_ERROR_CHECK(mbc_master_init_tcp(&master_handler));
    
    ESP_ERROR_CHECK(mbc_master_setup((void*)&_comm_info));
    
    ESP_ERROR_CHECK(mbc_master_set_descriptor(_parameter_descriptors, sizeof(_parameter_descriptors)/sizeof(_parameter_descriptors[0])));
    
    ESP_ERROR_CHECK(mbc_master_start());
    
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void ESP32::destroy_all()
{
    ESP_ERROR_CHECK(mbc_master_destroy());

    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    
    esp_netif_destroy((esp_netif_t*)_comm_info.ip_netif_ptr); // note: esp_netif_deinit() function not supported according to api documentation
    
    ESP_ERROR_CHECK(nvs_flash_deinit());
}

void ESP32::testrw(uint16_t cid, uint16_t set_value)
{
    const mb_parameter_descriptor_t* temp_pd = NULL;   
    
    uint8_t temp_value[2] = {(uint8_t)set_value, (uint8_t)(set_value >> 8)};
    
    uint8_t type;

    ESP_ERROR_CHECK(mbc_master_get_cid_info(cid, &temp_pd));
    
    type = (uint8_t)temp_pd->param_type;
    
    ESP_ERROR_CHECK(mbc_master_set_parameter(cid, (char*)temp_pd->param_key, (uint8_t*)&temp_value, &type));

    ESP_LOGI(_mbtcp_tag, "SET PARAMETER");
    
    ESP_ERROR_CHECK(mbc_master_get_parameter(temp_pd->cid, (char*)temp_pd->param_key, temp_value, &type));
    
    ESP_LOGI(_mbtcp_tag, "GOT PARAMETER: CID #%d %s (%s) VALUE = %u (0x%04x)", temp_pd->cid, (char*)temp_pd->param_key, (char*)temp_pd->param_units, *(uint16_t*)temp_value, *(uint16_t*)temp_value); 

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

