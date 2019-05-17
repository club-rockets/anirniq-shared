#ifndef MTI_H_
#define MTI_H_

#include "stm32f4xx_hal.h"

/******************************************************************************/
/*                                Define                                      */
/******************************************************************************/
#define MTCOMM_PROTOCOL_INFO_PIPE 0x01
#define MTCOMM_CONFIGURE_PIPE     0x02
#define MTCOMM_CONTROL_PIPE       0x03
#define MTCOMM_STATUS_PIPE        0x04
#define MTCOMM_NOTIFICATION_PIPE  0x05
#define MTCOMM_MEASUREMENTS_PIPE  0x06

#define MTCOMM_MID_WAKEUP                       0x3E
#define MTCOMM_MID_WAKEUP_ACK                   0x3F
#define MTCOMM_MID_SET_OUTPUT_CONFIGURATION     0xC0
#define MTCOMM_MID_SET_OUTPUT_CONFIGURATION_ACK 0xC1
#define MTCOMM_MID_SET_SYNC_SETTINGS            0x2C
#define MTCOMM_MID_SET_SYNC_SETTINGS_ACK        0x2D
#define MTCOMM_MID_GOTO_MEASUREMENT             0x10
#define MTCOMM_MID_GOTO_MEASUREMENT_ACK         0x11
#define MTCOMM_MID_MTDATA2                      0x36

#define MTCOMM_FORMAT_FLOAT32 0x00

#define MTCOMM_DATA_TEMPERATURE    0x0810
#define MTCOMM_DATA_PACKET_COUNTER 0x1020
#define MTCOMM_DATA_ORIENTATION    0x2030
#define MTCOMM_DATA_DELTA_V        0x4010
#define MTCOMM_DATA_ACCELERATION   0x4020
#define MTCOMM_DATA_RATE_OF_TURN   0x8020
#define MTCOMM_DATA_MAGNETIC_FIELD 0xC020

#define MTSSP_MESSAGE_OFFSET 4
#define MTSSP_DATA_OFFSET    6

/******************************************************************************/
/*                             Type  Prototype                                */
/******************************************************************************/
/** 3-float tuple */
typedef struct {
  float x, y, z;
} vec3f_t;

/** MTi configuration structure. To enable a measurement, give it a non-null
 * value, to be used as it's sampling rate.
 * 
 * Refer to the value in the mti_t struct for units and descriptions of fields
 */
typedef struct {
  uint16_t acceleration;
  uint16_t attitude;
  uint16_t delta_v;
  uint16_t temperature;
  uint16_t rate_of_turn;
  uint16_t magnetic_field;
} mti_config_t;

/** Structure representing a MTi-3-AHRS IMU, connected over SPI */
typedef struct {
  GPIO_TypeDef *cs_port;                /** GPIO port of chip select pin */
  uint16_t cs_pin;                      /** GPIO pin for chip select */

  GPIO_TypeDef *drdy_port;              /** GPIO port of data ready pin */
  uint16_t drdy_pin;                    /** GPIO pin for data ready */

  SPI_HandleTypeDef *spi;               /** SPI handle to use for communciation */

  mti_config_t* config;                 /** Configuration structure, allocated by user*/

  float temperature;                    /** Internal temperature, in Celsius */

  vec3f_t delta_v;                      /** Delta V (delta m/s) */
  vec3f_t acceleration;                 /** 3D acceleration vector. In m/s² */
  vec3f_t attitude;                     /** Attitude, in euler angles */
  vec3f_t rate_of_turn;
  vec3f_t magnetic_field;

  uint32_t initialized;
  uint32_t measuring;
} mti_t;

/******************************************************************************/
/*                             Global variable                                */
/******************************************************************************/

/******************************************************************************/
/*                             Function prototype                             */
/******************************************************************************/
void mti_init(mti_t *mti, GPIO_TypeDef* cs_port, uint16_t cs_pin, GPIO_TypeDef* drdy_port, uint16_t drdy_pin, SPI_HandleTypeDef* spi, mti_config_t* config);

/** Function to be called everytime the DRDY signal is up */
void mti_update(mti_t *mti);

#endif
