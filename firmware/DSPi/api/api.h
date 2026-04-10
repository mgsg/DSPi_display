#ifndef API_H
#define API_H

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_OK              0
#define DISPLAY_ERROR           1

#define DISPLAY_SKIN_ANALOG     0

#define I2C_CMD_DATA_SIZE       32
#define I2C_SLAVE_ADDRESS       0x1b        // 0x78 = 0x3c with R/W bit 0X7A=0X3D  // 0x1b        // 0x28

// Commands received (INPUT)
#define CMD_DISPLAY_OFF         0
#define CMD_DISPLAY_ON          1
#define CMD_UPDATE_STATUS       3
#define CMD_LOG                 4

#define CMD_SCREEN_SPLASH       10
#define CMD_SCREEN_LOG          11
#define CMD_SCREEN_LEVELS       12
#define CMD_SCREEN_STATUS       13
#define CMD_SCREEN_ONBOARD      14

#define CMD_DATA_SIZE           32

// Function prototypes for instantiation and manipulation
void api_init();
bool api_send_response(uint8_t bRequest, uint16_t wValue);
void api_send_string(const char *fmt, ...);
void api_send_heartbeat(void);
void api_vendor_request_handler(uint8_t *data, uint8_t len);

#endif // API_H