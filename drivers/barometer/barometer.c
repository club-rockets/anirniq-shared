#include "barometer.h"

#include "cmsis_os.h"

static uint16_t barometer_read_prom(barometer_t *barometer, uint8_t coefficient);
static uint32_t barometer_read_data(barometer_t *barometer, uint8_t cmd);

/** Initialize the barometer struct
 *
 * @param[in]  barometer  A pointer to an allocated barometer struct
 */
void barometer_init(barometer_t *barometer, GPIO_TypeDef* port, uint16_t pin, SPI_HandleTypeDef* spi)
{
    uint32_t i = 0;

    //initialise the barometer parameters
    barometer->initialized = 0;
    barometer->port = port;
    barometer->pin = pin;
    barometer->spi = spi;
    barometer->pressure = 0;
    barometer->temperature = 0;

    // Reset the barometer
    barometer_reset(barometer);

    // Read the calibration coefficient
    for (i = 0; i < 6; i++) {
        barometer->calibration[i] = barometer_read_prom(barometer, i + 1);
    }

    //set the handle to initialised
    barometer->initialized = 1;
}

/** Send the RESET command to the barometer
 * 
 * @param[in]  barometer  A pointer to the barometer struct
 */
void barometer_reset(barometer_t *barometer)
{
    uint8_t tx[3] = { BAROMETER_RESET_COMMAND, 0x00, 0x00 };

    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(barometer->spi, tx, 1, 10);
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_SET);

    osDelay(5);
}

/** Reads the calibration coefficients from the barometer (MS5607) device.
 *
 * @param[in]  barometer  A pointer to the barometer struct
 * @param[in]  coef_num   Coefficient number index to be received.
 *
 * @return the coefficient number for the given index
 */
static uint16_t barometer_read_prom(barometer_t *barometer, uint8_t coefficient)
{
    uint8_t tx[3] = { ((uint8_t) (PROM_READ_COMMAND + coefficient * 2)), 0, 0 };
    uint8_t rx[3] = { 0 };

    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(barometer->spi, tx, rx, (uint16_t) sizeof(tx), 1000);
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_SET);

    return (rx[1] << 8) + rx[2];
}

/** Sends the ADC read command to the MS5607 device, which results in receiving
 * the raw pressure or raw temperature value.
 *
 * Pretty much copy pasted from Christian Roy's code from the BeagleBone Black
 * version.
 *
 * @param[in]  barometer  A pointer to the barometer struct
 * @param[in]  cmd        Determines the timeout to be applied
 * 
 * @return a 32-bit value
 */
static uint32_t barometer_read_data(barometer_t *barometer, uint8_t cmd)
{
    uint8_t rx[10] = { 0 };
    uint8_t tx = CMD_ADC_CONV + cmd;

    // Pull CS low to initiate communication
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_RESET);

    // Send conversion command
    HAL_SPI_Transmit(barometer->spi, &tx, 1, 10);

    // Wait necessary conversion time, which depends on the specific command
    switch (cmd & 0x0F) {
        case CMD_ADC_256:
            //delay = 900us
            osDelay(1);
            break;
        case CMD_ADC_512:
            //delay = 3ms;
            osDelay(3);
            break;
        case CMD_ADC_1024:
            //delay = 4ms;
            osDelay(4);
            break;
        case CMD_ADC_2048:
            //delay = 6ms;
            osDelay(6);
            break;
        case CMD_ADC_4096:
            //delay = 10ms;
            osDelay(10);
            break;
    }

    // Pull CS high to finish the conversion
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_SET);
    osDelay(10);
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_RESET);

    // Receive the data after the conversion
    HAL_SPI_Receive(barometer->spi, rx, 4, 10);

    // Pull CS high after read
    HAL_GPIO_WritePin(barometer->port, barometer->pin, GPIO_PIN_SET);

    // First byte is not data
    return (rx[1] << 16) | (rx[2] << 8) | rx[3];
}

/** Update the pressure and temperature value from the barometer
 * 
 * @param[in]  barometer  A pointer to the barometer struct
 */
void barometer_update(barometer_t *barometer)
{
    uint32_t raw_pressure, raw_temperature;
    int32_t delta_temperature;
    int64_t offset, sensitivity;

    // See the chip (MS5607) datasheet for more information on these conversions
    raw_pressure = barometer_read_data(barometer, CMD_ADC_D1 + CMD_ADC_1024);
    raw_temperature = barometer_read_data(barometer,  CMD_ADC_D2 + CMD_ADC_1024);

    // dT = D2 - C5 * 2⁸
    // TEMP = 2000 + dT * C6 / 2²³
    delta_temperature = raw_temperature - (barometer->calibration[4] << 8);
    barometer->temperature = (2000 + (delta_temperature * barometer->calibration[5] >> 23)) / 100.0f;

    // OFF = = C2 * 2¹⁷ + (C4 * dT ) / 2⁶
    // SENS = C1 * 2 + (C3 * dT ) / 2⁷
    // PRESSURE = (D1 * SENS / 2²¹ - OFF) / 2¹⁵
    offset = (((int64_t)barometer->calibration[1]) << 17) + (barometer->calibration[3] * delta_temperature >> 6);
    sensitivity = (((int64_t)barometer->calibration[0]) << 16) + (barometer->calibration[2] * delta_temperature >> 7);
    barometer->pressure = (((raw_pressure * sensitivity >> 21) - offset) >> 15);
}

/** Converts milliBars to meters
 *
 * @param[in]  pressure  Barometric air pressure, in Pascals
 *
 * @return the altitude in meters
 * 
 * @note uses the NOAA formula, adpated for meters and simplified with my TI
 */
float pressure_to_altitude(float pressure)
{
    return 44307.693f - 4942.781f * pow(pressure, 0.190284);
}

/** Computes ambient air density with the Ideal gas law
 *
 * @param[in]  pressure     Barometric air pressure, in Pascals
 * @param[in]  temperature  Air temperature, in Celsius
 * 
 * @return air density
 */
float compute_air_density(float pressure, float temperature)
{
    return (pressure * AIR_MOLAR_MASS) / (GAZ_CONSTANT * (CELSIUS_TO_KELVIN + temperature));
}

/** Returns speed of sound for a given temperature
 *
 * @param[in]  tempeature  Air temperature, in Celsius
 */
float compute_sound_speed(float temperature)
{
    return sqrtf(COEFF_LAPLACE_BIATOM * GAZ_CONSTANT * (CELSIUS_TO_KELVIN + temperature) / AIR_MOLAR_MASS);
}
