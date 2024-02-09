#include "modbus_tcp.h"

esp_err_t init_nvs(struct modbus_tcp mbc) 
{
    mbc.esp_error_result = nvs_flash_init();

    if (mbc.esp_error_result == ESP_ERR_NVS_NO_FREE_PAGES || mbc.esp_error_result  == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        mbc.esp_error_result  = nvs_flash_init();
    }

    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "nvs_flash_init failed: returns(0x%x)", (int)mbc.esp_error_result);

    return ESP_OK;
}

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data, struct modbus_tcp mbc)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (mbc.number_of_retries < mbc.max_retries) 
        {
            esp_wifi_connect();
            mbc.number_of_retries++;
            ESP_LOGI(mbc.tag, "wifi connect failed: retrying connection to the network");
        } 

        else xEventGroupSetBits(mbc.wifi_event_group, WIFI_FAIL_BIT);

        ESP_LOGI(mbc.tag," max number of retries attempted: failed to connect to the network");
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(mbc.tag, "successfully connected to network, with ip:" IPSTR, IP2STR(&event -> ip_info.ip));
        mbc.number_of_retries = 0;
        xEventGroupSetBits(mbc.wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t init_netif(struct modbus_tcp mbc) 
{
    mbc.wifi_event_group = xEventGroupCreate();

    mbc.esp_error_result = esp_netif_init();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "esp_netif_init fail, returns(0x%x).", (int)mbc.esp_error_result);

    mbc.esp_error_result = esp_event_loop_create_default();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "esp_event_loop_create_default fail, returns(0x%x).", (int)mbc.esp_error_result);

    mbc.comm_info.ip_netif_ptr = esp_netif_create_default_wifi_sta();
    assert(mbc.comm_info.ip_netif_ptr);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {.sta = {.ssid = WIFI_SSID, .password = WIFI_PASS, .threshold.authmode = WIFI_AUTH_WPA2_PSK, .pmf_cfg = {.capable = true, .required = false},},};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(mbc.tag, "wifi initialization completed");

    EventBits_t bits = xEventGroupWaitBits(mbc.wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(mbc.tag, "successfully connected to wifi with SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    } 

    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(mbc.tag, "failed to connect to wifi with SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
    } 

    else 
    {
        ESP_LOGE(mbc.tag, "unexpected event");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(mbc.wifi_event_group);

    return ESP_OK;
}

esp_err_t init_slave(struct modbus_tcp mbc) 
{
    void *slave_handler = NULL;
    mbc.esp_error_result = mbc_slave_init_tcp(&slave_handler);
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK && slave_handler != NULL), ESP_ERR_INVALID_STATE, mbc.tag, "modbus controller initialization failed");
    mbc.comm_info.slave_uid = MODBUS_SLAVE_ADDRESS;
    
    mbc.esp_error_result = mbc_slave_setup((void*)&mbc.comm_info);
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "modbus controller slave setup failed, returns(0x%x).", (int)mbc.esp_error_result);

    mbc.register_area.type = MB_PARAM_HOLDING;
    mbc.register_area.start_offset = mbc.register_data[0];
    mbc.register_area.address = (void*)&mbc.register_data[0];
    mbc.register_area.size =  sizeof(mbc.register_data) << 1;
    mbc.esp_error_result = mbc_slave_set_descriptor(mbc.register_area);
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "modbus controller failed to set slave description, returns(0x%x).", (int)mbc.esp_error_result);

    mbc.esp_error_result = mbc_slave_start();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "mbc_slave_start fail, returns(0x%x).", (int)mbc.esp_error_result);
    vTaskDelay(5);

    return mbc.esp_error_result;
}

void slave_communication(void *arg, struct modbus_tcp mbc)
{
    ESP_LOGI(mbc.tag, "modbus slave stack initialized");
    ESP_LOGI(mbc.tag, "starting modbus test...");

    for(;mbc.register_data[0] < MODBUS_CHANNEL_DATA_MAX_VALUE;) 
    {
        (void)mbc_slave_check_event(MODBUS_READ_WRITE_MASK);
        ESP_ERROR_CHECK_WITHOUT_ABORT(mbc_slave_get_param_info(&mbc.register_info, MODBUS_PARAM_INFO_GET_TIMEOUT));
        const char* rw_str = (mbc.register_info.type & MODBUS_READ_MASK) ? "READ" : "WRITE";

        if(mbc.register_info.type & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) 
        {
            ESP_LOGI(mbc.tag, "HOLDING %s (%" PRIu32 " us), ADDR:%u, TYPE:%u, INST_ADDR:0x%" PRIx32 ", SIZE:%u",
                            rw_str,
                            mbc.register_info.time_stamp,
                            (unsigned)mbc.register_info.mb_offset,
                            (unsigned)mbc.register_info.type,
                            (uint32_t)mbc.register_info.address,
                            (unsigned)mbc.register_info.size);
            if (mbc.register_info.address == (uint8_t*)&mbc.register_data[0])
            {
                portENTER_CRITICAL(&param_lock);
                mbc.register_data[0] += MODBUS_CHANNEL_DATA_OFFSET;
                portEXIT_CRITICAL(&param_lock);
            }
        }
    }
    ESP_LOGI(mbc.tag,"modbus controller destroyed");
    vTaskDelay(100);
}

esp_err_t destroy_slave(struct modbus_tcp mbc)
{
    mbc.esp_error_result = mbc_slave_destroy();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "failed to destroy modbus controller slave, returns(0x%x).", (int)mbc.esp_error_result);
    
    return mbc.esp_error_result;
}

esp_err_t destroy_services(struct modbus_tcp mbc)
{
    mbc.esp_error_result = ESP_OK;

    mbc.esp_error_result = esp_event_loop_delete_default();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "failed to destroy default event loop, returns(0x%x).", (int)mbc.esp_error_result);
    
    mbc.esp_error_result = esp_netif_deinit();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK || mbc.esp_error_result == ESP_ERR_NOT_SUPPORTED), ESP_ERR_INVALID_STATE, mbc.tag, "failed to destroy network interface, returns(0x%x).", (int)mbc.esp_error_result);
    
    mbc.esp_error_result = nvs_flash_deinit();
    MB_RETURN_ON_FALSE((mbc.esp_error_result == ESP_OK), ESP_ERR_INVALID_STATE, mbc.tag, "failed to destroy nvs, returns(0x%x).", (int)mbc.esp_error_result);
    
    return mbc.esp_error_result;
}