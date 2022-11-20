#include "TemperatureSensorWrapper.h"

#include "../../tools/Exception.h"
#include "../../tools/DebugInfo.h"

TemperatureSensorWrapper::TemperatureSensorWrapper(Type type, std::shared_ptr<I2CMux> muxPtr)
    : TempSensor(type, muxPtr)
{
    auto ret = Open(m_mux->GetFd());
    if (ret < 0)
    {
        std::ostringstream oss;
        oss << "Failed to open Temperature Sensor " << m_type << " ret: " << ret;
        setErrorStatus(oss.str());
    }
    else
        setOnlineStatus();
}

TemperatureSensorWrapper::~TemperatureSensorWrapper()
{

}

ITemperatureSensor::TResult TemperatureSensorWrapper::values()
{
    if (isOnline())
    {
        float temperatureInCelsius, humidityPercent;

        if (auto ret = MeasureSync(temperatureInCelsius, humidityPercent))
        {
            THROWEX("Error reading TemperatureSensor " << m_type << " ret:" << ret);
        }
        return {
            .temperature = temperatureInCelsius,
            .humidity = humidityPercent
        };
    }
    else
        return {
            .temperature = -1,
            .humidity = -1
        };
}
