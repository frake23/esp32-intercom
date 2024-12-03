#ifndef PCF8574_H
#define PCF8574_H

#include "driver/i2c_master.h"
#include "driver/i2c.h"

void i2c_master_init();
uint8_t read_pcf8574();
void write_pcf8574(uint8_t data);

#endif // PCF8574_H