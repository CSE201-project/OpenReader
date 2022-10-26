#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include <memory>
#include <cstdint>

#include "i2c_mux.h"

#define LIGHT_SENSOR_COUNT 2u

class LightSensor
{
public:
    LightSensor(uint8_t index, std::shared_ptr<I2CMux> muxPtr);

    int Open(int i2cFd);
    int MeasureSync(int32_t &valUV, int32_t &valVis);
    void Close();

    ~LightSensor();

protected:
    std::shared_ptr<I2CMux> m_mux;
    int m_i2cFd;
    uint8_t m_i2cAddr;
    uint8_t m_index;
    bool m_isOpen;

    int ReadReg(uint8_t address, uint8_t &value);
    int WriteReg(uint8_t address, uint8_t value);

    int ReadParam(uint8_t address, uint8_t &value);
    int WriteParam(uint8_t address, uint8_t value);

    int WriteCommand(uint8_t command);

    int ConfigureChannel(uint8_t chIdx, uint8_t adcMuxVal, uint8_t hwGain);
};

#endif // LIGHTSENSOR_H
