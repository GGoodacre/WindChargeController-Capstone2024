#include "Arduino.h"
#include <SPI.h>
#include "SPImaster.hpp"
#include <bit>
#include <algorithm>
#include <array>

SPImaster::SPImaster()
{
    spi = new SPIClass(VSPI);
    spi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
}

bool SPImaster::config(int cs)
{
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
/*
    //CONFIG REGISTER SETUP
    int config_word = ( CONFIG_RST | CONFIG_RSTACC | CONFIG_CONVDLY | CONFIG_TEMPCOMP | CONFIG_ADCRANGE );
    write(cs, CONFIG, config_word);

    //ADC_CONFIG REGISTER SETUP
    config_word = ( ADC_CONFIG_MODE | ADC_CONFIG_VBUSCT | ADC_CONFIG_VSHCT | ADC_CONFIG_VTCT | ADC_CONFIG_AVG );             
    write(cs, ADC_CONFIG, config_word);

    //SHUNT_CAL REGISTER SETUP
    float shunt_cal = (13107.2*pow(10,6))*(CURRENT_MAX/pow(2,19))*R_SHUNT;
    if ((CONFIG_ADCRANGE) > 0) shunt_cal = shunt_cal*4;
    config_word = (int)shunt_cal;
    write(cs, SHUNT_CAL, config_word);

    //SHUNT_TEMPCO REGISTER SETUP
    config_word = SHUNT_TEMPCO_TEMPCO;
    write(cs, SHUNT_TEMPCO, config_word);
*/
    return true;
}

bool SPImaster::write(int cs, int register_addr, int data)
{
    int buff =  (register_addr << WRITE_REGISTER_SHIFT) | (WRITE_REGISTER << WRITE_RW_SHIFT) | data;
    flipBytes(buff);
    //ESP_LOGI(SPI_TAG, "Data to transfer: %d", buff);
    this->spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(cs, LOW);
    this->spi->transfer(&buff, WRITE_SIZE);
    digitalWrite(cs, HIGH);
    this->spi->endTransaction();
    ESP_LOGI(SPI_TAG, "Register: %d  |  Data Written: %d", register_addr, (int)data);
    return true;
}

bool SPImaster::read(int cs, int register_addr, long &data)
{
    data = 0;
    int size;
    if (register_addr == MANUFACTURER_ID || register_addr == DEVICE_ID)
    {
        size = 3;
    }
    else
    {
        size = (register_size[register_addr])/8 + 1;
    }
    uint8_t tx[size];
    uint8_t rx[size];

    for(int i = 0; i < size; i++)
    {
        rx[i] = 0;
        tx[i] = 0;
    }
    tx[0] = (register_addr << READ_REGISTER_SHIFT) | (READ_REGISTER << READ_RW_SHIFT);

    this->spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(cs, LOW);

    this->spi->transferBytes(tx, rx, size);

    digitalWrite(cs, HIGH);
    this->spi->endTransaction();

    for(int i = 0; i < size; i++)
    {
        data = data | rx[i] << (i*8);
    }

    ESP_LOGI(SPI_TAG, "Register: %d  |  Data Read: %d", register_addr, (int)data);
    return true;
}

void SPImaster::flipBytes(int &data)
{
    std::array<std::byte, WRITE_SIZE> value;
    int temp = data;
    for(int i = 0; i < WRITE_SIZE; i++)
    {
        value[i] = (std::byte)temp;
        temp = temp >> 8;
    }
    std::reverse(value.begin(), value.end());
    temp = 0;
    for(int i = 0; i < WRITE_SIZE; i++)
    {
        temp = temp + int((unsigned char)value[i] << (8*i));
    }
    data = temp;
}
