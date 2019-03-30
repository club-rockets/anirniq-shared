#include "mti.h"

typedef union {
    struct {
        uint8_t pipe;
        uint8_t fillwords[3];
        uint8_t message_id;
        uint8_t length;
        uint8_t data[];
    } fields;
    uint8_t data[256];
} message_t;

static message_t rx_buf;
static message_t tx_buf;


void mti_compute_crc();
void mti_send_message(mti_t *mti);
void mti_receive_message(mti_t *mti);

void mti_build_config_value(uint32_t value, uint16_t data_id, uint32_t* cursor);
void mti_build_config_message(mti_t *mti);

void mti_init(mti_t *mti, GPIO_TypeDef* cs_port, uint16_t cs_pin, GPIO_TypeDef* drdy_port, uint16_t drdy_pin, SPI_HandleTypeDef* spi, mti_config_t* config)
{
    mti->cs_port = cs_port;
    mti->cs_pin = cs_pin;

    mti->drdy_port = drdy_port;
    mti->drdy_pin = drdy_pin;

    mti->spi = spi;

    mti->config = config;
}

void mti_update(mti_t *mti)
{
    mti_receive_message(mti);

    switch (rx_buf.fields.message_id) {
        case MTCOMM_MID_WAKEUP:
            tx_buf.fields.pipe = MTCOMM_CONTROL_PIPE;
            tx_buf.fields.message_id = MTCOMM_MID_WAKEUP_ACK;
            tx_buf.fields.length = 0;
            mti_send_message(mti);

            mti_build_config_message(mti);
            mti_send_message(mti);
            break;
    };
}

void mti_compute_crc()
{
	uint8_t sum = 0xFF;

	for (uint8_t i = 0; i < tx_buf.fields.length + 2; i++) {
		sum += tx_buf.data[MTSSP_MESSAGE_OFFSET + i];
    }

    tx_buf.data[MTSSP_DATA_OFFSET + tx_buf.fields.length] = ((uint8_t)~sum) + 1;
}

void mti_send_message(mti_t *mti)
{
    mti_compute_crc();
	HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(mti->spi, tx_buf.data, MTSSP_DATA_OFFSET + tx_buf.fields.length + 1, 100);
    HAL_GPIO_WritePin(mti->cs_port, mti->cs_pin, GPIO_PIN_SET);
}

void mti_receive_message(mti_t *mti)
{
    rx_buf.fields.message_id = MTCOMM_MID_WAKEUP;
    rx_buf.fields.length = 0;
    return;
}

void mti_build_config_value(uint32_t value, uint16_t data_id, uint32_t* cursor)
{
    if (value) {
        tx_buf.data[(*cursor)++] = data_id >> 8;
        tx_buf.data[(*cursor)++] = data_id & 0xFF;
        tx_buf.data[(*cursor)++] = value >> 8;
        tx_buf.data[(*cursor)++] = value & 0xFF;
    }
}

void mti_build_config_message(mti_t *mti)
{
    uint32_t cursor = MTSSP_DATA_OFFSET;
    tx_buf.fields.pipe = MTCOMM_CONTROL_PIPE;
    tx_buf.fields.message_id = MTCOMM_MID_SET_OUTPUT_CONFIGURATION;

    mti_build_config_value(mti->config->acceleration_sampling_rate, 0x4020, &cursor);
    mti_build_config_value(mti->config->attitude_sampling_rate, 0x2030, &cursor);

    tx_buf.fields.length = cursor - MTSSP_DATA_OFFSET;
}