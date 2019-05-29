#ifndef APP_SD_DATA_H
#define APP_SD_DATA_H

/* #define your datatypes IDs here */
#define SD_DATA_GENERIC 0x00

/* typedef/define your sd data structures here. 
 * The first field should always be int type.
 * e.g.: 
 * typedef struct {
 *      int type;
 *      float pressure, temperature;
 * } sd_baro_data_t;
 */

typedef struct {
    int type;
} sd_generic_data_t;

typedef union {
    sd_generic_data_t generic;
    /* add your sd data structures here */
} sd_data_t;

void app_sd_format_data(sd_data_t* data, char* line, int len);

#endif