
/* INCLUDES */
#include "modbus_tcp.h"

/* APP_MAIN */
void app_main(void) 
{
    struct modbus_tcp mbc = {.tag = "modbus slave test", .comm_info = {.ip_port = MODBUS_TCP_PORT, .ip_addr_type = MB_IPV4, .ip_mode = MB_MODE_TCP, .ip_addr = NULL}, .number_of_retries = 0, .max_retries = 10};

    /* INITIALIZE ALL SERVICES */
    ESP_ERROR_CHECK(init_nvs(mbc));
    ESP_ERROR_CHECK(init_netif(mbc));

    /* INITIALIZE MODBUS SLAVE */
    ESP_ERROR_CHECK(init_slave(mbc));

    /* SLAVE COMMUNICATION */
    slave_communication(NULL, mbc);

    /* MODBUS TEARDOWN */
    ESP_ERROR_CHECK(destroy_slave(mbc));
    ESP_ERROR_CHECK(destroy_services(mbc));
}
