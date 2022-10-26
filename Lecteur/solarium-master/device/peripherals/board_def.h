#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include <cstdint>

enum I2CMuxChannel
{
    E_HUMIDITY     = 0,
    E_AMB_HUMIDITY = 1,
    E_FUEL_GAUGE   = 2,
    E_UV_DRV1_POT  = 3,
    E_VIS_DRV1_POT = 4,
    E_VIS_DRV2_POT = 5,
    E_LIGHT_SNS1   = 6,
    E_LIGHT_SNS2   = 7,
    E_UV_DRV2_POT  = 8,
};

static inline constexpr uint32_t GetGpioIdx(uint32_t port, uint32_t pin)
{
    return (port - 1u) * 32u + pin;
}

#define GPIO_IDX_I2C_MUX_nRST   GetGpioIdx(6, 16)
#define GPIO_IDX_LED_UV_EN      GetGpioIdx(2, 6)
#define GPIO_IDX_LED_VIS_EN     GetGpioIdx(2, 7)


#endif // BOARD_DEF_H
