#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/irq.h"
#include "hardware/uart.h"

#include "config.h"
#include "api.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define START_BYTE 0xAA
#define MAX_PAYLOAD_SIZE 128

/* ================= Internal RX State Machine ================= */

typedef enum {
  WAIT_START = 0,
  WAIT_LEN,
  WAIT_PAYLOAD,
  WAIT_CHECKSUM
} rx_state_t;

static volatile rx_state_t rx_state = WAIT_START;
static volatile uint8_t rx_len = 0;
static volatile uint8_t rx_index = 0;
static uint8_t rx_payload[MAX_PAYLOAD_SIZE];
static uint8_t tx_payload[MAX_PAYLOAD_SIZE];

/** 
 * Checksum. XOR_NO_HEADER
 */
static uint8_t calculate_checksum(uint8_t len, const uint8_t *data) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < len; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

/**
 * Receive Callback
 * Weak attribute allows user override in another file
*/
__attribute__((weak)) void intercom_uart_receive_callback(uint8_t *data, uint8_t len) {
  api_vendor_request_handler(data, len);
}

/** 
 * UART IRQ Handler 
 */
static void on_uart_rx(void) {
  while (uart_is_readable(UART_ID)) {
    uint8_t byte = uart_getc(UART_ID);
    switch (rx_state) {
    // case WAIT_START:
    //   if (byte == START_BYTE) {
    //     rx_state = WAIT_LEN;
    //   }
    //   break;
    // case WAIT_LEN:
    //   rx_len = byte;
    //   if (rx_len == 0 || rx_len > MAX_PAYLOAD_SIZE) {
    //     rx_state = WAIT_START;
    //   } else {
    //     rx_index = 0;
    //     rx_state = WAIT_PAYLOAD;
    //   }
    //   break;
    // case WAIT_PAYLOAD:
    //   rx_payload[rx_index++] = byte;
    //   if (rx_index >= rx_len) {
    //     rx_state = WAIT_CHECKSUM;
    //   }
    //   break;
    // case WAIT_CHECKSUM: {
    //   uint8_t checksum = calculate_checksum(rx_len, rx_payload);
    //   if (checksum == byte) {
    //     intercom_uart_receive_callback(rx_payload, rx_len);
    //   }
    //   rx_state = WAIT_START;
    //   break;
    // }
    case WAIT_START:
      if (byte == START_BYTE) {
        rx_index = 0;
        rx_state = WAIT_PAYLOAD;
      }
      break;
    case WAIT_PAYLOAD:
      rx_payload[rx_index++] = byte;
      uint8_t checksum = calculate_checksum(rx_index, rx_payload);
      if (checksum == byte) {
        intercom_uart_receive_callback(rx_payload, rx_index);
        rx_state = WAIT_START;
      }
      break;
    default:
      rx_state = WAIT_START;
      break;
    }
  }
}

/** 
 * Initialization 
 */
void uart_api_init(void) {
  uart_init(UART_ID, BAUD_RATE);

  gpio_set_function(PICO_UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(PICO_UART_RX_PIN, GPIO_FUNC_UART);

  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
  uart_set_hw_flow(UART_ID, false, false);

  // Enable RX interrupt
  uart_set_irq_enables(UART_ID, true, false);

  irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
  irq_set_enabled(UART0_IRQ, true);
}

/** 
 * Send Function
 */
void uart_api_send_bytes(uint8_t *data, uint8_t len) {
  if (len == 0) 
    return;

  if (len > MAX_PAYLOAD_SIZE-2)
    len = MAX_PAYLOAD_SIZE-2;

  tx_payload[0] = START_BYTE;
  memcpy(&tx_payload[1], data, len);
  tx_payload[1 + len] = calculate_checksum(len, data);

  uart_write_blocking(UART_ID, tx_payload, len + 2);
}
