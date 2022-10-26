#ifndef TEMPERATURESENSORWRAPPER_H
#define TEMPERATURESENSORWRAPPER_H

#include "../ITemperatureSensor.h"
#include "../IPeripheralStatus.h"
#include "../peripherals/temp_sensor.h"

class TemperatureSensorWrapper : public ITemperatureSensor, public TempSensor, public IPeripheralStatus
{
public:
    TemperatureSensorWrapper(TempSensor::Type type, std::shared_ptr<I2CMux> muxPtr);
    virtual ~TemperatureSensorWrapper();

    // ITemperatureSensor interface
public:
    TResult values();
};

#endif // TEMPERATURESENSORWRAPPER_H
