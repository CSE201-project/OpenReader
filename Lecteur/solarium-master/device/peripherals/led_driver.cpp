#include "led_driver.h"
#include "../../tools/logging.h"

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>

enum RegAddr
{
    E_REG_ADDR_IVR      = 0,
    E_REG_ADDR_DEV_ID   = 1,
    // [2..7] not used by driver.
    E_REG_ADDR_ACR      = 8,
};

static constexpr uint8_t k_potAddresses[LedDriver::E_LED_TYPE_COUNT][2] = {
    { 0x28u, 0x28u },
    { 0x28u, 0x28u },
};

static constexpr I2CMuxChannel k_potMuxChIdx[LedDriver::E_LED_TYPE_COUNT][2] = {
    { I2CMuxChannel::E_UV_DRV1_POT, I2CMuxChannel::E_UV_DRV2_POT },
    { I2CMuxChannel::E_VIS_DRV1_POT, I2CMuxChannel::E_VIS_DRV2_POT },
};

static constexpr uint32_t k_ledEnGpioIdx[LedDriver::E_LED_TYPE_COUNT] = {
    GPIO_IDX_LED_UV_EN, GPIO_IDX_LED_VIS_EN
};

static constexpr uint8_t k_potDevId = 0xD0u;

LedDriver::LedDriver(LedDriver::Type type, std::shared_ptr<I2CMux> muxPtr) :
    m_type(type),
    m_intensity{0, 0},
    m_mux(muxPtr),
    m_i2cFd(-1),
    m_isOpen(false)
{

}

int LedDriver::Open(int i2cFd)
{
    uint8_t devId[2] = {0xFFu, 0xFFu};

    if (i2cFd < 0)
    {
        return -4;
    }

    if (m_isOpen)
    {
        return -5;
    }

    // Open GPIO port and disable LEDs on startup
    if (m_enGpio.Open(k_ledEnGpioIdx[m_type], GpioInstance::E_DIR_WRITE) < 0)
    {
        LogError << "Failed to open EN GPIO";
        return -1;
    }

    if (Disable() < 0)
    {
        return -2;
    }

    m_i2cFd = i2cFd;

    // Check I2C communication with potentiometers
    if ((ReadPotReg(0u, E_REG_ADDR_DEV_ID, devId[0]) < 0) ||
        (ReadPotReg(1u, E_REG_ADDR_DEV_ID, devId[1]) < 0) ||
        (devId[0] != k_potDevId) ||
        (devId[1] != k_potDevId))
    {
        LogError << "led_driver : Error communicating with potentiometer(s)";
        return -3;
    }

    m_isOpen = true;
    return 0;
}

int LedDriver::Enable(uint8_t intensity1, uint8_t intensity2)
{
    if (!m_isOpen)
    {
        return -3;
    }

    if (SetIntensity(intensity1, intensity2) < 0)
    {
        return -1;
    }

    if (Enable() < 0)
    {
        return -2;
    }

    return 0;
}

int LedDriver::Enable()
{
    if (!m_isOpen)
    {
        return -2;
    }

    if (m_enGpio.Write(true) < 0)
    {
        LogError << "Could not enable potentiometer";
        return -1;
    }

    return 0;
}

int LedDriver::Disable()
{
    if (m_enGpio.Write(false) < 0)
    {
        LogError << "Could not disable potentiometer";
        return -1;
    }

    return 0;
}

int LedDriver::SetIntensity(uint8_t intensity1, uint8_t intensity2)
{
    if (!m_isOpen)
    {
        return -2;
    }

    if ((SetIntensityForDrv(0u, intensity1) < 0) ||
        (SetIntensityForDrv(1u, intensity2) < 0))
    {
        LogError << "Could not set LED intensity";
        return -1;
    }

    return 0;
}

int LedDriver::SetIntensityForDrv(uint8_t potIdx, uint8_t intensity)
{
    // Select non-volatile register (wiper register)
    if (WritePotReg(potIdx, E_REG_ADDR_ACR, 0x80u) < 0)
    {
        return -1;
    }

    if (WritePotReg(potIdx, E_REG_ADDR_IVR, ~intensity) < 0)
    {
        return -2;
    }

    return 0;
}

void LedDriver::Close()
{
    m_enGpio.Close();
}

LedDriver::~LedDriver()
{
    Close();
}

int LedDriver::ReadPotReg(uint8_t potIdx, uint8_t address, uint8_t &value)
{
    int ret = 0;
    uint8_t msg = address;

    if (m_mux->Acquire(m_i2cFd, k_potMuxChIdx[m_type][potIdx], 100) < 0)
    {
        return -1;
    }

    if (ioctl(m_i2cFd, I2C_SLAVE, k_potAddresses[m_type][potIdx]) < 0)
    {
        LogError << "Error selecting potentiometer I2C address\n";
        ret = -2;
    }

    if (write(m_i2cFd, &msg, 1u) != 1u)
    {
        LogError << "Error writing to potentiometer I2C\n";
        ret = -3;
    }

    if (read(m_i2cFd, &value, 1u) != 1u)
    {
        LogError << "Error reading potentiometer I2C\n";
        ret = -4;
    }

    m_mux->Release();
    return ret;
}

int LedDriver::WritePotReg(uint8_t potIdx, uint8_t address, uint8_t value)
{
    int ret = 0;
    uint8_t msg[2] = {address, value};

    if (m_mux->Acquire(m_i2cFd, k_potMuxChIdx[m_type][potIdx], 100) < 0)
    {
        return -1;
    }

    if (ioctl(m_i2cFd, I2C_SLAVE, k_potAddresses[m_type][potIdx]) < 0)
    {
        LogError << "Error selecting potentiometer I2C address";
        ret = -2;
    }

    if (write(m_i2cFd, msg, sizeof(msg)) != sizeof(msg))
    {
        LogError << "Error writing to potentiometer I2C";
        ret = -3;
    }

    m_mux->Release();
    return ret;
}
