#include <stdint.h>

typedef unsigned int GPIO_TypeDef;
typedef unsigned int GPIO_PinState;
typedef unsigned int SPI_HandleTypeDef;

#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1

void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
void HAL_SPI_Transmit(SPI_HandleTypeDef* spi, uint8_t* data, uint16_t length, uint32_t timeout);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);