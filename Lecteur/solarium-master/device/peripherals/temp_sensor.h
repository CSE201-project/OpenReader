#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <memory>
#include <cstdint>

#include "i2c_mux.h"

class TempSensor
{
public:
    enum Type
    {
        E_TEMP_SNS_TYPE_LOCAL = 0,
        E_TEMP_SNS_TYPE_AMBIENTAL,
        // Used internally
        E_TEMP_SNS_TYPE_COUNT
    };

    TempSensor(Type type, std::shared_ptr<I2CMux> muxPtr);

    int Open(int i2cFd);
    int MeasureSync(float &tempDegC, float &relHumPercent);
    void Close();

    ~TempSensor();

protected:
    std::shared_ptr<I2CMux> m_mux;
    int m_i2cFd;
    Type m_type;
    bool m_isOpen;

    int ReadReg(uint8_t address, uint16_t &value);
    int WriteReg(uint8_t address, uint16_t value);
    int TriggerAndMeasureBoth(uint16_t &tempRaw, uint16_t &humRaw);
};

#endif // TEMPSENSOR_H
