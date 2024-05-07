#include "ControllerTCP.hpp"

ControllerTCP::ControllerTCP(HardwareControl &hw) : hw(hw) {
    _tag = "TCP Controller";
    init_netif_helper();
    xTaskCreate(&run_tcp_console_trampoline, "tcp_console_task", 4096, this, 9, &_tcp_console);
    init_mb_master_helper();
    xTaskCreate(&run_send_data_trampoline, "mb_send_data", 4096, this, 9, &_mb_send_data);
}

void ControllerTCP::wifi_connect_hdl_en(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ControllerTCP* enable_event_handler = (ControllerTCP*) arg;
    enable_event_handler -> wifi_connect_hdl(arg, event_base, event_id, event_data);
}

void ControllerTCP::wifi_connect_hdl(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    static int s_retry_num = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_wifi_connect());
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num <= MAX_RETRIES) {
            ESP_ERROR_CHECK(esp_wifi_connect());  
            s_retry_num++;
            ESP_LOGE(_tag, "retrying connection to wifi");
        }
        else {
            xEventGroupSetBits(_s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* got_ip_event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(_tag, "successfully connected to wifi with ip:" IPSTR, IP2STR(&got_ip_event->ip_info.ip));
        s_retry_num = 0;
        _connected_to_wifi = true;
        xEventGroupSetBits(_s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void ControllerTCP::init_netif_helper() {
    ESP_ERROR_CHECK(nvs_flash_init());

    _s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    _mb_comm_info.ip_netif_ptr = (void*)esp_netif_create_default_wifi_sta();

    wifi_init_config_t _wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&_wifi_init_config));

    _connected_to_wifi = false;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_connect_hdl_en, this, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_connect_hdl_en, this, &instance_got_ip));

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = {'a','d','a','o','4'},
            .password = {'1','2','3','4','5','6','7','8'},
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(_s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(_tag, "connected to ap SSID:%s password:%s", "adao4", "12345678");
    } 
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(_tag, "Failed to connect to SSID:%s, password:%s", "adao4", "12345678");
    } 
    else {
        ESP_LOGE(_tag, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(_s_wifi_event_group);
}

void ControllerTCP::run_tcp_console_trampoline(void* arg) {
    ControllerTCP* local_tcp_console = (ControllerTCP*) arg;

    local_tcp_console->run_tcp_console();
}

void ControllerTCP::run_tcp_console() {
    while(1) {
        if (_connected_to_wifi) {
            if (open_tcp_socket()) {
                if (bind_tcp_socket()) {
                    while (_connected_to_wifi) {
                        if (listen_tcp_socket()) {
                            // receive messages from client
                            int len;
                            char rx_buffer[512];
                            memset(rx_buffer, ' ', sizeof(rx_buffer)); 
                            do {
                                len = recv(console_sock, rx_buffer, sizeof(rx_buffer), 0);
                                if (len < 0) {
                                    ESP_LOGE(_tag, "Error occurred during receiving: errno %d", errno);
                                }
                                else if (len == 0) {
                                    ESP_LOGW(_tag, "Connection closed");
                                }
                                else {
                                    ESP_LOGW(_tag, "Message from Client received: %s", rx_buffer);
                                    // do stuff based on message received
                                    char *token;
                                    token = strtok(rx_buffer, "_"); // gets device or pwm
                                    if(strcmp(token, "1") == 0) {
                                        digitalWrite(TURBINE_PIN, atol(token));
                                    }
                                    if(strcmp(token, "0") == 0) {
                                        digitalWrite(TURBINE_PIN, atol(token));
                                    }
                                    if (strcmp(token, "DEVICE") == 0) {
                                        int device_id = atoi(strtok(NULL, "_")); // gets device id
                                        bool en = atoi(strtok(NULL, "_")); // gets enable
                                        hw.setDeviceEN(device_id, en);
                                    }
                                    else if (strcmp(token, "PWM") == 0) {
                                        char *component = strtok(NULL, "_"); // gets rectifier, sepic, ps1, or ps2
                                        int pwm_id = 5;
                                        if (strcmp(component, "RECTIFIER") == 0) pwm_id = PWM_RECTIFIER;
                                        if (strcmp(component, "SEPIC") == 0) pwm_id = PWM_SEPIC;
                                        if (strcmp(component, "PS1") == 0) pwm_id = PWM_PS1;
                                        if (strcmp(component, "PS2") == 0) pwm_id = PWM_PS2;
                                        char *setting = strtok(NULL, "_"); // gets dutycycle or setpoint
                                        double val = atol(strtok(NULL, "_")); // gets value
                                        if (strcmp(setting, "DUTYCYCLE") == 0) hw.setPWM(pwm_id, val);
                                        if (strcmp(setting, "SETPOINT") == 0) hw.setSetPoint(pwm_id, val);
                                    }
                                    //vTaskDelay(10 / portTICK_PERIOD_MS);
                                }
                            } while (len > 0);
                        }
                        else break;
                        shutdown(console_sock, 0);
                        close(console_sock);
                    }
                }
            }
        }
        //vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

bool ControllerTCP::open_tcp_socket() {
    struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*) &dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(_tag, "Unable to create socket: errno %d", errno);
        return false;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(_tag, "Socket created");

    return true;
}

bool ControllerTCP::bind_tcp_socket() {
    if (bind(listen_sock, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) != 0) {
        ESP_LOGE(_tag, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(_tag, "IPPROTO: %d", AF_INET);
        close(listen_sock);
        return false;
    }
    ESP_LOGI(_tag, "Socket bound, port %d", PORT);
    if (listen(listen_sock, 1) != 0) {
        ESP_LOGE(_tag, "Error occurred during listen: errno %d", errno);
        close(listen_sock);
        return false;
    }
    ESP_LOGI(_tag, "Socket listening");
    return true;
}

bool ControllerTCP::listen_tcp_socket()
{
    static char addr_str[128];
    static int keepAlive = 1;
    static int keepIdle = SOCK_KEEPALIVE_IDLE;
    static int keepInterval = SOCK_KEEPALIVE_INTERVAL;
    static int keepCount = SOCK_KEEPALIVE_COUNT;

    socklen_t addr_len = sizeof(source_addr);
    console_sock = accept(listen_sock, (struct sockaddr*) &source_addr, &addr_len);
    if (console_sock < 0) {
        ESP_LOGE(_tag, "Unable to accept connection: errno %d", errno);
        close(listen_sock);
        shutdown(console_sock, 0);
        close(console_sock);
        return false;
    }

    // set tcp keepalive option
    setsockopt(console_sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
    setsockopt(console_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
    setsockopt(console_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
    setsockopt(console_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
    // convert ip address to string
    if (source_addr.ss_family == PF_INET) {
        inet_ntoa_r(((struct sockaddr_in*) &source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    }
    ESP_LOGI(_tag, "Socket accepted ip address: %s", addr_str);
    return true;
}

void ControllerTCP::init_mb_master_helper() {
    ESP_ERROR_CHECK(mbc_master_init_tcp(&_master_handler));
    ESP_LOGI(_tag, "HANDLER INITIALIZED");

    _mb_dev_ip_addrs[0] = RASPBERRY_IP;
    _mb_dev_ip_addrs[1] = RASPBERRY_IP;
    _mb_dev_ip_addrs[2] = RASPBERRY_IP;
    _mb_dev_ip_addrs[3] = RASPBERRY_IP;
    _mb_dev_ip_addrs[4] = RASPBERRY_IP;
    _mb_dev_ip_addrs[5] = RASPBERRY_IP;
    _mb_dev_ip_addrs[6] = RASPBERRY_IP;
    _mb_dev_ip_addrs[7] = NULL;

    _mb_comm_info.ip_mode = MB_MODE_TCP;
    _mb_comm_info.ip_port = 502; // sdkconfig default 502
    _mb_comm_info.ip_addr_type = MB_IPV4;
    _mb_comm_info.ip_addr = _mb_dev_ip_addrs;

    ESP_ERROR_CHECK(mbc_master_setup((void*)&_mb_comm_info));
    ESP_LOGI(_tag, "COMM INFO SET");

    mb_parameter_descriptor_t pd[CID_NUM] = {
        // SHUNT 1

        {
        .cid = SHUNT1_POWER_CID,
        .param_key = "Shunt1 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT1_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT1_VOLTAGE_CID,
        .param_key = "Shunt1 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT1_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT1_CURRENT_CID,
        .param_key = "Shunt1 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT1_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT1_TEMPERATURE_CID,
        .param_key = "Shunt1 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT1_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

        // SHUNT 2

        {
        .cid = SHUNT2_POWER_CID,
        .param_key = "Shunt2 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT2_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT2_VOLTAGE_CID,
        .param_key = "Shunt2 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT2_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT2_CURRENT_CID,
        .param_key = "Shunt2 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT2_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT2_TEMPERATURE_CID,
        .param_key = "Shunt2 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT2_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
    
        // SHUNT 3

        {
        .cid = SHUNT3_POWER_CID,
        .param_key = "Shunt3 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT3_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT3_VOLTAGE_CID,
        .param_key = "Shunt3 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT3_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT3_CURRENT_CID,
        .param_key = "Shunt3 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT3_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT3_TEMPERATURE_CID,
        .param_key = "Shunt3 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT3_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

        // SHUNT 4

        {
        .cid = SHUNT4_POWER_CID,
        .param_key = "Shunt4 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT4_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT4_VOLTAGE_CID,
        .param_key = "Shunt4 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT4_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT4_CURRENT_CID,
        .param_key = "Shunt4 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT4_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT4_TEMPERATURE_CID,
        .param_key = "Shunt4 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT4_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

        // SHUNT 5

        {
        .cid = SHUNT5_POWER_CID,
        .param_key = "Shunt5 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT5_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT5_VOLTAGE_CID,
        .param_key = "Shunt5 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT5_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT5_CURRENT_CID,
        .param_key = "Shunt5 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT5_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT5_TEMPERATURE_CID,
        .param_key = "Shunt5 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT5_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

        // SHUNT 6

        {
        .cid = SHUNT6_POWER_CID,
        .param_key = "Shunt6 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT6_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT6_VOLTAGE_CID,
        .param_key = "Shunt6 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT6_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT6_CURRENT_CID,
        .param_key = "Shunt6 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT6_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT6_TEMPERATURE_CID,
        .param_key = "Shunt6 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT6_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

        // SHUNT 7

        {
        .cid = SHUNT7_POWER_CID,
        .param_key = "Shunt7 Power",
        .param_units = "W",
        .mb_slave_addr = SHUNT7_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_POWER_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT7_VOLTAGE_CID,
        .param_key = "Shunt7 Voltage",
        .param_units = "V DC",
        .mb_slave_addr = SHUNT7_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_VOLTAGE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {0, 65535, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT7_CURRENT_CID,
        .param_key = "Shunt7 Current",
        .param_units = "A DC",
        .mb_slave_addr = SHUNT7_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_CURRENT_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },
        {
        .cid = SHUNT7_TEMPERATURE_CID,
        .param_key = "Shunt7 Temperature",
        .param_units = "Degrees Celcius",
        .mb_slave_addr = SHUNT7_UID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = BATTERY_TEMPERATURE_REG,
        .mb_size = 1,
        .param_offset = 0,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {-32768, 32767, 1},
        .access = PAR_PERMS_READ_WRITE
        },

    };
    memcpy(_param_descs, pd, sizeof(_param_descs));

    ESP_ERROR_CHECK(mbc_master_set_descriptor(_param_descs, CID_NUM));
    ESP_LOGI(_tag, "PD SET");
    ESP_ERROR_CHECK(mbc_master_start());
    ESP_LOGI(_tag, "MASTER STARTED");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}


void ControllerTCP::run_send_data_trampoline(void* arg) {
    ControllerTCP* local_send_data = (ControllerTCP*) arg;

    local_send_data->run_send_data();
}

void ControllerTCP::run_send_data() {
    const mb_parameter_descriptor_t* pd_rx = NULL;
    _device_values_t measurements;

    while(1) {
        for (int i = 0; i < TOTAL_MEASUREMENT_DEVICES; i++) {

            measurements = hw.getDeviceParams(i);

            uint16_t cid_power, cid_voltage, cid_current, cid_temperature;
            switch(i) {
                case 0:
                    cid_power = SHUNT1_POWER_CID;
                    cid_voltage = SHUNT1_VOLTAGE_CID;
                    cid_current = SHUNT1_CURRENT_CID;
                    cid_temperature = SHUNT1_TEMPERATURE_CID; 
                    break;
                case 1:
                    cid_power = SHUNT2_POWER_CID;
                    cid_voltage = SHUNT2_VOLTAGE_CID;
                    cid_current = SHUNT2_CURRENT_CID;
                    cid_temperature = SHUNT2_TEMPERATURE_CID;
                    break;
                case 2:
                    cid_power = SHUNT3_POWER_CID;
                    cid_voltage = SHUNT3_VOLTAGE_CID;
                    cid_current = SHUNT3_CURRENT_CID;
                    cid_temperature = SHUNT3_TEMPERATURE_CID;
                    break;
                case 3:
                    cid_power = SHUNT4_POWER_CID;
                    cid_voltage = SHUNT4_VOLTAGE_CID;
                    cid_current = SHUNT4_CURRENT_CID;
                    cid_temperature = SHUNT4_TEMPERATURE_CID;
                    break;
                case 4:
                    cid_power = SHUNT5_POWER_CID;
                    cid_voltage = SHUNT5_VOLTAGE_CID;
                    cid_current = SHUNT5_CURRENT_CID;
                    cid_temperature = SHUNT5_TEMPERATURE_CID;
                    break;
                case 5:
                    cid_power = SHUNT6_POWER_CID;
                    cid_voltage = SHUNT6_VOLTAGE_CID;
                    cid_current = SHUNT6_CURRENT_CID;
                    cid_temperature = SHUNT6_TEMPERATURE_CID;
                    break;
                case 6:
                    cid_power = SHUNT7_POWER_CID;
                    cid_voltage = SHUNT7_VOLTAGE_CID;
                    cid_current = SHUNT7_CURRENT_CID;
                    cid_temperature = SHUNT7_TEMPERATURE_CID;
                    break;
                default:
                    cid_power = SHUNT1_POWER_CID;
                    cid_voltage = SHUNT1_VOLTAGE_CID;
                    cid_current = SHUNT1_CURRENT_CID;
                    cid_temperature = SHUNT1_TEMPERATURE_CID;
                    break;
            }

            short ps = measurements.power * BATTERY_POWER_SF;
            short vs = measurements.vbus * BATTERY_VOLTAGE_SF;
            short cs = measurements.current * BATTERY_CURRENT_SF;
            short ts = measurements.dietemp * BATTERY_TEMPERATURE_SF;

            byte* pb = (byte*) &ps;
            byte* vb = (byte*) &vs;
            byte* cb = (byte*) &cs;
            byte* tb = (byte*) &ts;

            uint8_t pu[2] = {pb[0], pb[1]};
            uint8_t vu[2] = {vb[0], vb[1]};
            uint8_t cu[2] = {cb[0], cb[1]};
            uint8_t tu[2] = {tb[0], tb[1]};

            ESP_ERROR_CHECK(mbc_master_get_cid_info(cid_power, &pd_rx));
            uint8_t type = pd_rx->param_type;
            ESP_ERROR_CHECK(mbc_master_set_parameter(pd_rx->cid, (char*)pd_rx->param_key, (uint8_t*)pu, &type));

            ESP_ERROR_CHECK(mbc_master_get_cid_info(cid_voltage, &pd_rx));
            type = pd_rx->param_type;
            ESP_ERROR_CHECK(mbc_master_set_parameter(pd_rx->cid, (char*)pd_rx->param_key, (uint8_t*)vu, &type));

            ESP_ERROR_CHECK(mbc_master_get_cid_info(cid_current, &pd_rx));
            type = pd_rx->param_type;
            ESP_ERROR_CHECK(mbc_master_set_parameter(pd_rx->cid, (char*)pd_rx->param_key, (uint8_t*)cu, &type));

            ESP_ERROR_CHECK(mbc_master_get_cid_info(cid_temperature, &pd_rx));
            type = pd_rx->param_type;
            ESP_ERROR_CHECK(mbc_master_set_parameter(pd_rx->cid, (char*)pd_rx->param_key, (uint8_t*)tu, &type));
        };
    }
}

