#include "mti.h"

typedef union {
    struct {
        uint8_t pipe;
        uint8_t fillwords[3];
        uint8_t message_id;
        uint8_t length;
    } fields;
    uint8_t data[256];
} message_t;

static message_t rx_buf;
static message_t tx_buf;

void mti_compute_crc();
void mti_send_message(mti_t *mti);
void mti_receive_message(mti_t *mti);

void mti_handle_mtdata2(mti_t *mti);

void mti_build_config_value(uint32_t value, uint16_t data_id, uint32_t *cursor);
void mti_build_config_message(mti_t *mti);

void read_float(int cursor, float *dest);
void read_vec3f(int cursor, vec3f_t *dest);


/** Initialize an MTi structure
 * 
 * @param  mti        Empty but allocated mti_t struct
 * @param  cs_port    GPIO port for the chip select pin
 * @param  cs_pin     GPIO pin for chip select
 * @param  drdy_port  GPIO port for the data ready pin
 * @param  drdy_pin   GPIO pin for data ready signal
 * @param  spi  SPI   handle connected to the MTi device
 * @param  config     allocated and configured mti_config_t structure
 */
void mti_init(mti_t *mti, GPIO_TypeDef* cs_port, uint16_t cs_pin, GPIO_TypeDef* drdy_port, uint16_t drdy_pin, SPI_HandleTypeDef* spi, mti_config_t* config)
{
    mti->cs_port = cs_port;
    mti->cs_pin = cs_pin;
    mti->drdy_port = drdy_port;
    mti->drdy_pin = drdy_pin;

    mti->spi = spi;

    mti->config = config;
    mti->initialized = 1;
    mti->measuring = 0;
}

/** Update the state of the MTi (receive message, or continuing configuration).
 * This function should be called everytime (but only when) the DRDY pin is
 * active
 * 
 * @param  mti  MTi device to process
 */
void mti_update(mti_t *mti)
{
    mti_receive_message(mti);

    switch (rx_buf.fields.message_id) {
        /* Replying WAKEUP_ACK within 500ms of power-on will make the MTi enter
           its configuration state. */
        case MTCOMM_MID_WAKEUP:
            tx_buf.fields.pipe = MTCOMM_CONTROL_PIPE;
            tx_buf.fields.message_id = MTCOMM_MID_WAKEUP_ACK;
            tx_buf.fields.length = 0;
            mti_send_message(mti);
            mti->measuring = 0;

            mti_build_config_message(mti);
            mti_send_message(mti);
            break;

        /* After configuring the output, go to measurements */
        case MTCOMM_MID_SET_OUTPUT_CONFIGURATION_ACK:
            tx_buf.fields.pipe = MTCOMM_CONFIGURE_PIPE;
            tx_buf.fields.message_id = MTCOMM_MID_GOTO_MEASUREMENT;
            tx_buf.fields.length = 0;
            mti_send_message(mti);
            break;

        /* Nothing to do */
        case MTCOMM_MID_GOTO_MEASUREMENT_ACK:
            mti->measuring = 1;
            break;

        case MTCOMM_MID_MTDATA2:
            mti_handle_mtdata2(mti);
            break;
    };
}


/** Compute the CRC for rx_buf */
void mti_compute_crc()
{
	uint8_t sum = 0xFF;

	for (uint8_t i = 0; i < tx_buf.fields.length + 2; i++) {
		sum += tx_buf.data[MTSSP_MESSAGE_OFFSET + i];
    }

    tx_buf.data[MTSSP_DATA_OFFSET + tx_buf.fields.length] = ((uint8_t)~sum) + 1;
}

/** Send the message in tx_buf to an MTi device
 * 
 * @param  mti  MTi device to send the message to
 */
void mti_send_message(mti_t *mti)
{
    mti_compute_crc();
	HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(mti->spi, tx_buf.data, MTSSP_DATA_OFFSET + tx_buf.fields.length + 1, 100);
    HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_SET);
}

/** Receive a message from the MTi and store it in the tx_buf
 * 
 * @param  mti  MTi device to receive message from
 */
void mti_receive_message(mti_t *mti)
{
	uint16_t size;

    tx_buf.fields.pipe = MTCOMM_STATUS_PIPE;

	// Read message size
	HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(mti->spi, &tx_buf, &rx_buf, 8, 100);
	HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_SET);

	// Figure out whether message is notification or measurement based
	// on size information.
	uint16_t size = (rx_buf.data[4] | rx_buf.data[5] << 8);
    if (size) {
        tx_buf.fields.pipe = MTCOMM_NOTIFICATION_PIPE;
    }
    else {
        tx_buf.fields.pipe = MTCOMM_MEASUREMENTS_PIPE;
        size = (rx_buf.data[6] | rx_buf.data[7] << 8);
    }
	// Read message
	HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(mti->spi, &tx_buf, &rx_buf, 3 + size, 100);
    HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_SET);
}

/** Handle a MTDATA2 message and fill the approriate fields of the MTI struct 
 * 
 * @param  mti  MTi device that sent the MTDATA2 message
*/
void mti_handle_mtdata2(mti_t *mti)
{
    uint16_t data_id;
    uint16_t length = 0;

    for (int i = MTSSP_DATA_OFFSET; i < MTSSP_DATA_OFFSET + rx_buf.fields.length; i += length) {
        data_id = (rx_buf.data[i] << 8 + rx_buf.data[i + 1]) & 0xFFF0;
        switch (data_id)
        {
            case MTCOMM_DATA_TEMPERATURE:
                read_float(i, &mti->temperature);
                break;
            case MTCOMM_DATA_DELTA_V:
                read_vec3f(i, &mti->delta_v);
                break;
            case MTCOMM_DATA_ACCELERATION:
                read_vec3f(i, &mti->acceleration);
                break;
            case MTCOMM_DATA_ORIENTATION:
                read_vec3f(i, &mti->attitude);
                break;
            case MTCOMM_DATA_RATE_OF_TURN:
                read_vec3f(i, &mti->rate_of_turn);
                break;
            case MTCOMM_DATA_MAGNETIC_FIELD:
                read_vec3f(i, &mti->magnetic_field);
                break;

            default:
                break;
        }
    }
}

/** Build the SET_OUTPUT_CONFIGURATION packet for a given data_id and frequency
 * 
 * @param  value  the frequency (0 for disabled)
 * @param  data_id  the hex value of the XDA type (see MTLLCP datasheet)
 * @param  cursor  pointer to the current position in the buffer
 * 
 * @note  This will increment cursor if needed
 */
void mti_build_config_value(uint32_t value, uint16_t data_id, uint32_t* cursor)
{
    if (value) {
        tx_buf.data[(*cursor)++] = data_id >> 8;
        tx_buf.data[(*cursor)++] = data_id & 0xFF;
        tx_buf.data[(*cursor)++] = value >> 8;
        tx_buf.data[(*cursor)++] = value & 0xFF;
    }
}

/** Build the SET OUTPUT CONFIGURATION message for a given mti->config
 * 
 * @param  mti  pointer to the mti structure with the configuration to use
 * 
 * @note You still need to call mti_send_message() after this
 */
void mti_build_config_message(mti_t *mti)
{
    uint32_t cursor = MTSSP_DATA_OFFSET;
    tx_buf.fields.pipe = MTCOMM_CONTROL_PIPE;
    tx_buf.fields.message_id = MTCOMM_MID_SET_OUTPUT_CONFIGURATION;

    mti_build_config_value(mti->config->temperature, MTCOMM_DATA_TEMPERATURE, &cursor);
    mti_build_config_value(mti->config->delta_v, MTCOMM_DATA_DELTA_V | MTCOMM_FORMAT_FLOAT32, &cursor);
    mti_build_config_value(mti->config->acceleration, MTCOMM_DATA_ACCELERATION | MTCOMM_FORMAT_FLOAT32, &cursor);
    mti_build_config_value(mti->config->attitude, MTCOMM_DATA_ORIENTATION | MTCOMM_FORMAT_FLOAT32, &cursor);
    mti_build_config_value(mti->config->rate_of_turn, MTCOMM_DATA_RATE_OF_TURN | MTCOMM_FORMAT_FLOAT32, &cursor);
    mti_build_config_value(mti->config->magnetic_field, MTCOMM_DATA_MAGNETIC_FIELD | MTCOMM_FORMAT_FLOAT32, &cursor);

    tx_buf.fields.length = cursor - MTSSP_DATA_OFFSET;
}

/** Read a float value from rx_buf, at given position.
 * 
 * @param  cursor  position in rx_buf.data
 * @param  dest    destination float
*/
void read_float(int cursor, float *dest)
{
    union {
        uint32_t u;
        float f;
    } f32;
    f32.u = (rx_buf.data[cursor] << 24) | (rx_buf.data[cursor + 1] << 16) | (rx_buf.data[cursor + 2] << 8) | (rx_buf.data[cursor + 3] << 0);
    *dest = f32.f;
}

/** Read a vec3f value from rx_buf, at given position.
 * 
 * @param  cursor  position in rx_buf.data
 * @param  dest    destination vec3f
*/
void read_vec3f(int cursor, vec3f_t *dest)
{
    read_float(cursor, &dest->x);
    read_float(cursor + sizeof(float), &dest->y);
    read_float(cursor + 2 * sizeof(float), &dest->z);
}