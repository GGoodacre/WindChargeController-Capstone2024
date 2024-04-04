#include "Arduino.h"
#include "HardwareControl.hpp"
#include "PowerSupplyControl.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

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

#define TXD_PIN (GPIO_NUM_43) // GPIO pin for TX
#define RXD_PIN (GPIO_NUM_44) // GPIO pin for RX

/*
void app_main(void)
{
    //HardwareControl hw;
    PowerSupplyControl ps;

    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);
    esp_adc_cal_characteristics_t adc1_chars;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 0, &adc1_chars);

    float test_point;
    float Kp;
    float duty;

    _device_values_t measurements;
    ps.setSetPoint(0, 3);
    ps.setAlgorithm(PID);
    while(1)
    {
        ps.testDevice(0);
        test_point = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc1_chars) * 0.006f;
        Kp = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_1), &adc1_chars) * 0.04;
        ESP_LOGI(MAIN_TAG, "SetPoint:%f,Kp:%f", test_point, Kp);
        measurements = ps.getDeviceParams(0);
        ESP_LOGI(MAIN_TAG, "Current:%f,Voltage_Shunt:%f,Voltage_Bus:%f", measurements.current, 1000*measurements.vshunt, measurements.vbus);
        ps.setSetPoint(0, test_point);
        ps.setPWM_PID_Kp(0, (int)Kp);
        ps.update();
        duty = ps.getPWM(0);
        ESP_LOGI(CSV_TAG,"%f,%f,%f,%f,%f,%f", measurements.current, measurements.vshunt, measurements.vbus, test_point, Kp, duty);
        delayMicroseconds(50000);
    }
}
*/
HardwareControl hw;
esp_adc_cal_characteristics_t adc1_chars;
void idleTsk(void * parameter);


void app_main(void)
{

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
    for(;;)
    {
        hw.update();
        _device_values_t measurements;


        for(int i = 0; i < TOTAL_MEASUREMENT_DEVICES; i++)
        {
            if(hw.getDeviceEN(i))
            {
                measurements = hw.getDeviceParams(i);
                ESP_LOGI(CSV_TAG,"%lld,%d,%f,%f,%f,%f", esp_timer_get_time(), i, measurements.current, measurements.vbus, measurements.power, measurements.dietemp);
            }
        }
    }
}


