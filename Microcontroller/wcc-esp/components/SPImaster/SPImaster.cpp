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
    return false;
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
    return true;
}

bool SPImaster::read(int cs, int register_addr, long &data)
{
    byte buff =  (register_addr << READ_REGISTER_SHIFT) | (READ_REGISTER << READ_RW_SHIFT);
    this->spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    digitalWrite(cs, LOW);
    data = this->spi->transfer(buff);
    for(int i = 0; i < (register_size[register_addr] / 8) - 1; i++)
    {
        data = data << 8 | this->spi->transfer(0x00);
    }
    digitalWrite(cs, HIGH);
    this->spi->endTransaction();
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
