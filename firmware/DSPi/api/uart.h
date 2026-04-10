#ifndef UART_API_H
#define UART_API_H

#include <stdint.h>

void uart_api_init(void);
void uart_api_send_bytes(uint8_t *data, uint8_t len);

#endif  // UART_API_H