#ifndef APP_SD_H_
#define APP_SD_H_

#include "app_sd_data.h"

#define CARD_CONNECTED    0x00
#define CARD_DISCONNECTED 0x01
#define DATA_READY        0x02

void app_sd_init();
sd_data_t* app_sd_prepare_data();
void app_sd_write_data(sd_data_t* data);
void app_sd_detect_handler();

#endif