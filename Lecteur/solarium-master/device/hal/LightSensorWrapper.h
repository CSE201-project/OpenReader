#ifndef LIGHTSENSORWRAPPER_H
#define LIGHTSENSORWRAPPER_H

#include "../ILightSensor.h"
#include "../IPeripheralStatus.h"
#include "../peripherals/light_sensor.h"

class LightSensorWrapper : public ILightSensor, public LightSensor, public IPeripheralStatus {
public:
    LightSensorWrapper(uint8_t index, const std::shared_ptr<I2CMux> &muxPtr);

    virtual ~LightSensorWrapper();

    TResult values() override;

};

#endif // LIGHTSENSORWRAPPER_H
