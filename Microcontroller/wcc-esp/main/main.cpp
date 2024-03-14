#include "Arduino.h"
#include "HardwareControl.hpp"
#include "PowerSupplyControl.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"

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

void app_main(void)
{
    HardwareControl hw;

    while(1)
    {
        hw.testDevice(0);
        hw.testDevice(1);
        hw.testDevice(2);
        hw.testDevice(3);
        hw.testDevice(4);
        hw.testDevice(5);
        hw.testDevice(6);

        delayMicroseconds(50000);
    }
}
