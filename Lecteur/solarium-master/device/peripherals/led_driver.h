#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <memory>
#include "gpio_instance.h"
#include "i2c_mux.h"

class LedDriver
{
public:
    enum Type
    {
        E_LED_TYPE_UV = 0,
        E_LED_TYPE_VIS,
        E_LED_TYPE_COUNT
    };

    LedDriver(Type type, std::shared_ptr<I2CMux> muxPtr);

    int Open(int i2cFd);

    // intensity range: [0..255]. See schematic for resulting LED currents.
    int Enable(uint8_t intensity1, uint8_t intensity2);
    int Enable();
    int Disable();
    // intensity range: [0..255]. See schematic for resulting LED currents.
    int SetIntensity(uint8_t intensity1, uint8_t intensity2);

    void Close();

    virtual ~LedDriver();

protected:
    Type m_type;
    uint8_t m_intensity[2];
    std::shared_ptr<I2CMux> m_mux;
    GpioInstance m_enGpio;
    int m_i2cFd;
    bool m_isOpen;

    int SetIntensityForDrv(uint8_t potIdx, uint8_t intensity);
    int ReadPotReg(uint8_t potIdx, uint8_t address, uint8_t &value);
    int WritePotReg(uint8_t potIdx, uint8_t address, uint8_t value);
};

#endif // LEDDRIVER_H
