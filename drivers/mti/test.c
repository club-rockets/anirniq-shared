#include "mti.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state)
{
    printf("[\033[34;1mGPIO\033[0m PORT %u PIN %u = %u]\n", *port, pin, state);
}

void HAL_SPI_Transmit(SPI_HandleTypeDef* spi, uint8_t* data, uint16_t length, uint32_t timeout)
{
    printf("[\033[35;1mSPI\033[0m %u LENGTH %u [ ", *spi, length);
    for (int i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("] TIMEOUT %u]\n", timeout);
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin)
{
    printf("[\033[34;1mGPIO\033[0m PORT %u PIN %u = ?\033[1D", *port, pin);

    char c = getchar();
    if (c == 'y' || c == 'Y' || c == 'o' || c == 'O') {
        printf("\033[1D1]\n");
        return 1;
    }

    printf("\033[1D0]\n");
    return 1;
}

int main(int argc, char const *argv[])
{
    static struct termios oldt, newt;

    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /* Test code */
    GPIO_TypeDef cs_port = 1;
    GPIO_TypeDef drdy_port = 2;
    SPI_HandleTypeDef spi = 1;

    mti_t mti;
    mti_config_t config = {
        .delta_v = 50,
        .acceleration = 50,
        .attitude = 100,
        .rate_of_turn = 100,
        .magnetic_field = 10,
        .temperature = 1
    };

    mti_init(&mti, &cs_port, 1, &drdy_port, 1, &spi, &config);
    mti_update(&mti);
    /* End test code */

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
