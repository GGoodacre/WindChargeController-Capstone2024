/*
#include <stdio.h>
#include "Arduino.h"
#include "ControllerTCP.hpp"

extern "C" void app_main(void) {
    double a; // test variable
    a = 0; // some initial state
    ControllerTCP controller(a);
    while(1) {
        a = a + 1; // some ongoing operation performed
        ESP_LOGI("TEST", "a = %f", a);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/

#include "Arduino.h"
#include "HardwareControl.hpp"
#include "PowerSupplyControl.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
<<<<<<< Updated upstream
=======

#include "ControllerTCP.hpp"
>>>>>>> Stashed changes

extern "C"
{
    #include <stdio.h>
    #include "esp_task_wdt.h"
}

extern "C"
{
    void app_main(void);
}

static const char* MAIN_TAG = "MAIN";
static const char* CSV_TAG = "CSV";
<<<<<<< Updated upstream
=======
static const char* PWM_TAG = "PWM";
>>>>>>> Stashed changes

#define TXD_PIN (GPIO_NUM_43) // GPIO pin for TX
#define RXD_PIN (GPIO_NUM_44) // GPIO pin for RX

<<<<<<< Updated upstream
/*
=======
HardwareControl hw;
esp_adc_cal_characteristics_t adc1_chars;
void idleTsk(void * parameter);


>>>>>>> Stashed changes
void app_main(void)
{

    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 0, &adc1_chars);

    float test_point;
    float Kp;
    float duty;
    int *adc_out;

    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, LOW);


    hw.setSetPoint(0, 3);
    //hw.setAlgorithm(PID);
    hw.setPWM(PWM_SEPIC, 40);

    esp_log_level_set(CONTROL_TAG, ESP_LOG_NONE);
    esp_log_level_set(SPI_TAG, ESP_LOG_INFO);
    esp_log_level_set(MAIN_TAG, ESP_LOG_NONE);
    esp_log_level_set(CSV_TAG, ESP_LOG_NONE);

    xTaskCreate(
        idleTsk,    // Function that should be called
        "Idle Task",   // Name of the task (for debugging)
        10000,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
    );
}
<<<<<<< Updated upstream
*/
HardwareControl hw;
esp_adc_cal_characteristics_t adc1_chars;
void idleTsk(void * parameter);

=======
>>>>>>> Stashed changes

void idleTsk(void * parameter)
{

<<<<<<< Updated upstream
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 0, &adc1_chars);

    float test_point;
     bv
    float Kp;
    float duty;
    int *adc_out;

    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, LOW);


    hw.setSetPoint(0, 3);
    //hw.setAlgorithm(PID);
    hw.setPWM(PWM_SEPIC, 40);

    esp_log_level_set(CONTROL_TAG, ESP_LOG_NONE);
    esp_log_level_set(SPI_TAG, ESP_LOG_NONE);
    esp_log_level_set(MAIN_TAG, ESP_LOG_NONE);
    esp_log_level_set(CSV_TAG, ESP_LOG_INFO);

    xTaskCreate(
        idleTsk,    // Function that should be called
        "Idle Task",   // Name of the task (for debugging)
        10000,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
    );
}

void idleTsk(void * parameter)
{
=======
    // passing hw into the controller class to do stuff
    ControllerTCP controller(hw);
>>>>>>> Stashed changes
    for(;;)
    {
        hw.update();
        _device_values_t measurements;
<<<<<<< Updated upstream


=======
        controller.send_data();

>>>>>>> Stashed changes
        for(int i = 0; i < TOTAL_MEASUREMENT_DEVICES; i++)
        {
            if(hw.getDeviceEN(i))
            {
<<<<<<< Updated upstream
                measurements = hw.getDeviceParams(i);
                ESP_LOGI(CSV_TAG,"%lld,%d,%f,%f,%f,%f", esp_timer_get_time(), i, measurements.current, measurements.vbus, measurements.power, measurements.dietemp);
            }
=======
                //hw.testDevice(i);
                measurements = hw.getDeviceParams(i);
                ESP_LOGI(CSV_TAG,"%lld,%d,%f,%f,%f,%f", esp_timer_get_time(), i, measurements.current, measurements.vbus, measurements.power, measurements.dietemp);
            }
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        for(int i = 0; i < TOTAL_PWM_VALUES; i++) {
            ESP_LOGI(PWM_TAG, "%lld,%d,%f,%d", esp_timer_get_time(), i, hw.getPWM(i), hw.getSetPoint(i));
            vTaskDelay(250 / portTICK_PERIOD_MS);
>>>>>>> Stashed changes
        }
    }
}


