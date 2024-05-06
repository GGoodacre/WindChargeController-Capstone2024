idf_component_register(SRCS "ControllerTCP.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES "nvs_flash" "esp_event" "esp_wifi" "freertos" "lwip" "log" "freemodbus" "arduino" "Control" "PowerElectronics" "SPImaster" "QuickPID")
