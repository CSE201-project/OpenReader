#include "LightSensorWrapper.h"


#include <memory>
#include <thread>
#include <chrono>

#include "../../tools/Exception.h"
#include "../../tools/DebugInfo.h"

LightSensorWrapper::LightSensorWrapper(uint8_t index, const std::shared_ptr<I2CMux> &muxPtr)
    : LightSensor(index, muxPtr)
{
    auto ret = Open(m_mux->GetFd());
    if (ret < 0)
    {
        std::ostringstream oss;
        oss << "Failed to open Light Sensor " << m_index << " ret: " << ret;
        setErrorStatus(oss.str());
    }
    else
        setOnlineStatus();
}

LightSensorWrapper::~LightSensorWrapper()
{

}

ILightSensor::TResult LightSensorWrapper::values()
{
    if (isOnline())
    {
        int32_t valUV, valVis;
        if (auto ret = MeasureSync(valUV, valVis))
        {
            THROWEX("Error reading Light Sensor values, ret: " << ret);
        }
        debugLog("LightSensor " << unsigned(m_index) << " uv:" << valUV << " visible:" << valVis);
        return {
                .uv = valUV,
                .visible = valVis
        };
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return {
            .uv = -255,
            .visible = -255
        };
    }
}
