#include <pico/error.h>
#include <pico/time.h>
#include <pico/types.h>
#include <hardware/gpio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USE_UART true
// #define USE_I2C true

#include "../config.h"
#include "api.h"

#ifdef USE_UART
#include "uart.h"
#endif
#ifdef USE_I2C
#include "i2c.h"
#endif

#define MAX_PAYLOAD_SIZE 128
#define LEVEL_EVENT_RATE_MS 200

absolute_time_t last_event_time;

static bool isUartInitialized = false;
static bool isI2cInitialized = false;

void api_init() {
    last_event_time = get_absolute_time();
    srand(time(NULL));

#ifdef USE_UART
    uart_api_init();
    isUartInitialized = true;
#endif

#ifdef USE_I2C
    i2c_api_init();
    isI2cInitialized = true;
#endif
    api_send_string("Display console initialized.");
}

/**
 * Request Handler
*/
void api_vendor_request_handler(uint8_t *data, uint8_t len) {
  uint8_t bRequest = data[0];

  switch(bRequest) {
  case REQ_GET_STATUS:
    {
      // uint16_t wValue = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
      uint16_t wValue = (uint16_t)data[1]; // For now send one byte only for wValue
      // Check if the specified time has elapsed
      if (absolute_time_diff_us(last_event_time, get_absolute_time()) >= LEVEL_EVENT_RATE_MS * 1000) {
          api_send_response(REQ_GET_STATUS, 9);
          last_event_time = get_absolute_time();
      }
    }
    break;
  case REQ_GET_LOG:
    {
      api_send_string( "Ack=%s", (char *)&data[1]);
    }
    break;
  default:
    api_send_string( "Wrong request %x", bRequest);
    break;
  }
}

void api_send_heartbeat(void) {
  static int watchdog = 0;
  if (absolute_time_diff_us(last_event_time, get_absolute_time()) >= LEVEL_EVENT_RATE_MS * 1000 * 5) {
    gpio_xor_mask(1u << 25);
    api_send_string("Watchdog %d", watchdog++);
    last_event_time = get_absolute_time();
  }
}

/** 
 * Check usb_audio vendor_setup_request_handler
 */
bool api_send_response(uint8_t bRequest, uint16_t wValue) {
  // Device -> Host (GET requests)
  static uint8_t resp_buf[64];
  // struct usb_setup_packet setup;
  uint8_t setup_bRequest = REQ_GET_STATUS;
  uint16_t setup_wValue = 9;     // Send all status data

  switch(setup_bRequest) {
    case REQ_GET_STATUS:
      if (setup_wValue == 9) {
        resp_buf[0] = REQ_GET_STATUS;
        resp_buf[1] = 0x09;

        // Combined status: all peaks + CPU in one 12-byte transfer
        resp_buf[2] = global_status.peaks[0] & 0xFF;
        resp_buf[3] = global_status.peaks[0] >> 8;
        resp_buf[4] = global_status.peaks[1] & 0xFF;
        resp_buf[5] = global_status.peaks[1] >> 8;
        resp_buf[6] = global_status.peaks[2] & 0xFF;
        resp_buf[7] = global_status.peaks[2] >> 8;
        resp_buf[8] = global_status.peaks[3] & 0xFF;
        resp_buf[9] = global_status.peaks[3] >> 8;
        resp_buf[10] = global_status.peaks[4] & 0xFF;
        resp_buf[11] = global_status.peaks[4] >> 8;
        resp_buf[12] = global_status.cpu0_load;
        resp_buf[13] = global_status.cpu1_load;
        // vendor_send_response(resp_buf, 12);

        #ifdef USE_UART
        uart_api_send_bytes(resp_buf, 12+2);
        #endif
        
        #ifdef USE_I2C
        i2c_api_send_bytes(resp_buf, 12+2);
        #endif
        return true;
      }
      break;
  }
  return false;
}

/** 
 * Log Function
 */
static uint8_t tx_payload[MAX_PAYLOAD_SIZE];
void api_send_string(const char *fmt, ...) {
  uint8_t len;
  va_list args;

  tx_payload[0] = REQ_GET_LOG;
  va_start(args, fmt);
  vsnprintf((char *)&tx_payload[1], MAX_PAYLOAD_SIZE-1, fmt, args);
  va_end(args);              

  len = strlen((char *)&tx_payload[1]);
  if (len == 0)
    return;
  len+=1;   // For final 0x00 

  // tx_payload[0] = REQ_GET_LOG;
  // strcpy(&tx_payload[1], "HOLA");
  // tx_payload[5] = 0x00;
  // len = 5;

  #ifdef USE_UART
  uart_api_send_bytes(tx_payload, len + 1);
  #endif

  #ifdef USE_I2C
  i2c_api_send_bytes(payload, len + 1);
  #endif
}
