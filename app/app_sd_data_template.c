#include "app_sd_data.h"

#include <stdio.h>

/** app_sd_format_data
 * Format a sd_datat_t structure into a human-readable string.
 * 
 * @param  data  pointer to the sd_data_t structure (union)
 * @param  line  the output string buffer
 * @param  len   length of the given string buffer
 */
void app_sd_format_data(sd_data_t* data, char* line, int len)
{
    switch (data->generic.type) {
        /* For each SD data type your defined in app_sd_data.h, create a case
         * with a sntprinf()
         * e.g.:
         * case SD_DATA_BARO:
         *     snprintf(line, len, "baro,%f,%f\n", data->baro.pressure, data->baro.temperature);
         *     break;
        */
        default:
            snprintf(line, len, "unhandled data type in app_sd_format_data()\n");
            break;
    }
}