#ifndef I2C_API_H
#define I2C_API_H

#include <stdint.h>

void i2c_api_init();
void i2c_api_transmit(uint16_t length, uint8_t *data);
void i2c_api_send_bytes(uint8_t *data, uint16_t length);
#endif // I2C_API_H