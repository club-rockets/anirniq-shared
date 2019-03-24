#ifndef BAROMETER_H_
#define BAROMETER_H_

#include "stm32f4xx_hal.h"
#include "spi.h"
#include "main.h"

#include <math.h>

/******************************************************************************/
/*                                Define                                      */
/******************************************************************************/
#define PROM_READ_COMMAND           0xA0 // Command to read the PROM
#define BAROMETER_RESET_COMMAND     0x1E // Command to reset the barometer on the SPI interface

#define CMD_ADC_256                 0x00 // ADC OSR=256
#define CMD_ADC_512                 0x02 // ADC OSR=512
#define CMD_ADC_1024                0x04 // ADC OSR=1024
#define CMD_ADC_2048                0x06 // ADC OSR=2056
#define CMD_ADC_4096                0x08 // ADC OSR=4096
#define CMD_ADC_CONV                0x40 // ADC conversion command
#define CMD_ADC_READ                0x00 // ADC read command
#define CMD_ADC_D1                  0x00 // ADC D1 conversion
#define CMD_ADC_D2                  0x10 // ADC D2 conversion

#define AIR_MOLAR_MASS            0.0289644 // kg par mol
#define GAZ_CONSTANT              8.3144621
#define COEFF_LAPLACE_BIATOM      1.4
#define CELSIUS_TO_KELVIN         273.15

/******************************************************************************/
/*                             Type  Prototype                                */
/******************************************************************************/
typedef struct {
  GPIO_TypeDef *port;     /** GPIO port of chip select pin */
  uint16_t pin;           /** GPIO pin for chip select */
  SPI_HandleTypeDef *spi; /** SPI handle to use for communciation */

  uint32_t calibration[6]; /** Calibration registers */

  float pressure;    /** Air pressure, in Pascals */
  float temperature; /** Air temperature, in Celsius */

  uint32_t initialized;
} barometer_t;

/******************************************************************************/
/*                             Global variable                                */
/******************************************************************************/

/******************************************************************************/
/*                             Function prototype                             */
/******************************************************************************/
float mbar_to_altitude(float pressure);
float compute_air_density(float pressure, float temperature);
float compute_sound_speed(float temperature);

void barometer_init(barometer_t *barometer, GPIO_TypeDef* port, uint16_t pin, SPI_HandleTypeDef* spi);
void barometer_reset(barometer_t *barometer);
void barometer_update(barometer_t *barometer);

#endif /* BAROMETER_H_ */
