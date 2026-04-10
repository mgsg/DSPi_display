// vu_meter.c
#include <pico/error.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// #include <time.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "config.h"
#include "api.h"

void i2c_api_init() 
{
  // I2C + GPIO config
  sleep_ms(500);
  i2c_init(i2c0, 400000);
  gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_I2C_SCK_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_I2C_SDA_PIN);
  gpio_pull_up(PICO_I2C_SCK_PIN);
  sleep_ms(500);

  // Scan the bus
//   api_send_string("I2C BUS SCAN");
//   char buff[64];
//   for (uint8_t addr = 0; addr < 128; addr++) {
//     if (i2c_write_blocking(i2c0, addr, NULL, 0, true) < 0) {
//         sprintf(buff, "I2C device 0x%2x", addr);
//         api_send_string(buff);
//         sleep_ms(100);
//     }
//   }
}

/**
 * @param data First byte is address
 */
void i2c_api_transmit(uint16_t length, uint8_t *data)
{
  switch(i2c_write_blocking(i2c0, data[0], &data[1], length, false)) {
  case PICO_ERROR_GENERIC:
      printf("addr %x not acknowledged!\n", data[0]);
      break;
  case PICO_ERROR_TIMEOUT:
      printf("timeout!\n");
      break;
  default:
      //printf("[%s] wrote successfully %lu bytes!\n", name, len);
      break;
  }
}

void i2c_api_send_bytes(uint8_t *data, uint16_t length) 
{
    // Send through I2C
    uint8_t i2cPacket[64];
    i2cPacket[0] = I2C_SLAVE_ADDRESS;
    memcpy(&i2cPacket[1], data, length);
    i2c_api_transmit(length+1, i2cPacket);
}
