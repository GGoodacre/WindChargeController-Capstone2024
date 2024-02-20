#include "modbus_tcp_master.hpp"

ModbusController::ModbusController()
{

    /* device tag */

    _tag = "MB_CONTROLLER";

    /* ip addresses of all modbus capable devices in the network */

    char* ip_addresses[NUMBER_OF_MB_DEVICES + 1] = 
    {
        "192.168.81.169",
        NULL
    };
    memcpy(_mb_device_ip_addresses, ip_addresses, sizeof(_mb_device_ip_addresses));

    /* modbus parameters to read or write */

    mb_parameter_descriptor_t descriptors[NUMBER_OF_CIDS] = 
    {
        {
            .cid = CID_AC_L1_V,
            .param_key = "AC L1 Voltage",
            .param_units = "V",
            .mb_slave_addr = UID_MB_VE_PVI,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_reg_start = MB_AC_L1_V_REG_START,
            .mb_size = 1,
            .param_offset = 0,
            .param_type = PARAM_TYPE_U16,
            .param_size = PARAM_SIZE_U16,
            .param_opts = {0, 0, 0},
            .access = PAR_PERMS_READ_WRITE_TRIGGER
        },

        {
            .cid = CID_AC_L1_I,
            .param_key = "AC L1 Current",
            .param_units = "A",
            .mb_slave_addr = UID_MB_VE_PVI,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_reg_start = MB_AC_L1_V_REG_START,
            .mb_size = 1,
            .param_offset = 0,
            .param_type = PARAM_TYPE_U16,
            .param_size = PARAM_SIZE_U16,
            .param_opts = {0, 0, 0},
            .access = PAR_PERMS_READ_WRITE_TRIGGER
        }
    };
    memcpy(_parameter_descriptors, descriptors, sizeof(descriptors));

    /* modbus communication info */
    _communication_information.ip_mode = MB_MODE_TCP;
    _communication_information.ip_port = MB_TCP_PORT;
    _communication_information.ip_addr_type = MB_IPV4;
    _communication_information.ip_addr = (void*)_mb_device_ip_addresses;
    _communication_information.ip_netif_ptr = NULL; // to assign later while initializing wifi station  
}

void ModbusController::wifi_event_handler_jumper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ModbusController* local_wifi_event_handler = (ModbusController*) arg;
    local_wifi_event_handler -> wifi_event_handler(arg, event_base, event_id, event_data);
}

void ModbusController::wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    static int retry_wifi_connect_count = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        #ifdef ESP_DEBUG_MODE
            _esp_err = esp_wifi_connect();
            ESP_ERROR_CHECK_WITHOUT_ABORT(_esp_err);
            ESP_LOGI(_tag, "trying connection to wifi");
        #else
            esp_wifi_connect();
        #endif
    } 
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        #ifdef ESP_DEBUG_MODE
            _esp_err = esp_wifi_connect();
            ESP_ERROR_CHECK_WITHOUT_ABORT(_esp_err);
            ESP_LOGI(_tag, "retrying connection to wifi");
        #else
            esp_wifi_connect();
        #endif
        retry_wifi_connect_count++;
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        #ifdef ESP_DEBUG_MODE
            ip_event_got_ip_t* got_ip_event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(_tag, "got ip:" IPSTR, IP2STR(&got_ip_event->ip_info.ip));
        #endif
        retry_wifi_connect_count = 0;
    }
}

void ModbusController::initialize_esp32()
{
    /* initialize esp32 non-volatile storage */

    #ifdef ESP_DEBUG_MODE
        ESP_LOGI(_tag, "initializing esp32...");
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_init());
    #else
        nvs_flash_init();
    #endif

    /* initialize esp32 wifi station */

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    const wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_configuration = {.sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};

    #ifdef ESP_DEBUG_MODE
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        _communication_information.ip_netif_ptr = (void*)esp_netif_create_default_wifi_sta();
        assert(_communication_information.ip_netif_ptr);
        ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_jumper, this, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_jumper, this, &instance_got_ip));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration));
        ESP_ERROR_CHECK(esp_wifi_start());
    #else
        esp_netif_init();
        esp_event_loop_create_default();
        _communication_information.ip_netif_ptr = (void*)esp_netif_create_default_wifi_sta();
        esp_wifi_init(&wifi_init_config);
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_jumper, this, &instance_any_id);
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_jumper, this, &instance_got_ip);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);
        esp_wifi_start();
    #endif

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    /* initialize modbus tcp master  */

    static void* master_handler;

    #ifdef ESP_DEBUG_MODE
        ESP_ERROR_CHECK(mbc_master_init_tcp(&master_handler));
        ESP_ERROR_CHECK(mbc_master_setup((void*)&_communication_information));
        ESP_ERROR_CHECK(mbc_master_set_descriptor(_parameter_descriptors, sizeof(_parameter_descriptors)/sizeof(_parameter_descriptors[0])));
        ESP_ERROR_CHECK(mbc_master_start());
    #else
        mbc_master_init_tcp(&master_handler);
        mbc_master_setup((void*)&_communication_information);
        mbc_master_set_descriptor(_parameter_descriptors, sizeof(_parameter_descriptors));
        mbc_master_start();
    #endif

    vTaskDelay(50 / portTICK_PERIOD_MS); // optional delay? was included in example implementation
}

void ModbusController::destroy_esp32()
{
    #ifdef ESP_DEBUG_MODE
        ESP_LOGI(_tag, "destroying esp32...");
        ESP_ERROR_CHECK(mbc_master_destroy());
        ESP_ERROR_CHECK(esp_event_loop_delete_default());
        esp_netif_destroy((esp_netif_t*)_communication_information.ip_netif_ptr); // note: esp_netif_deinit() function not supported according to api documentation
        ESP_ERROR_CHECK(nvs_flash_deinit());
    #else
        mbc_master_destroy();
        esp_event_loop_delete_default();
        esp_netif_deinit(); // esp_netif_destroy() function not supported according to api documentation
        nvs_flash_deinit();
    #endif
}

void ModbusController::testrun_master(uint16_t new_value_to_set)
{
    const mb_parameter_descriptor_t* temp_pd = NULL;   
    uint8_t temp_value[2] = {(uint8_t)new_value_to_set, (uint8_t)(new_value_to_set >> 8)};
    uint8_t type;

    mbc_master_get_cid_info(CID_AC_L1_V, &temp_pd);
    type = (uint8_t)temp_pd->param_type;

    ESP_LOGI(_tag, "SETTING PARAMETER...");
    ESP_ERROR_CHECK(mbc_master_set_parameter(CID_AC_L1_V, (char*)temp_pd->param_key, (uint8_t*)&temp_value, &type));

    ESP_ERROR_CHECK(mbc_master_get_parameter(temp_pd->cid, (char*)temp_pd->param_key, temp_value, &type));
    ESP_LOGI(_tag, "GETTING PARAMETER: CID #%d %s (%s) VALUE = %u (0x%04x)", temp_pd->cid, (char*)temp_pd->param_key, (char*)temp_pd->param_units, *(uint16_t*)temp_value, *(uint16_t*)temp_value); 
}


