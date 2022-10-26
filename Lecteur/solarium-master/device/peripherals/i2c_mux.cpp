#include "i2c_mux.h"
#include "../../tools/logging.h"

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <exception>

#include "../../tools/simple_logging.h"

static const char *k_i2cDevicePath = "/dev/i2c-2";
// Channels [0..7] use first mux, channel 8 uses second mux
static constexpr uint8_t k_slaveAddresses[2] = {0x70u, 0x71u};

I2CMux::I2CMux() :
    m_mutex(),
    m_lock(m_mutex, std::defer_lock)
{
    OpenFd();
}

I2CMux::~I2CMux()
{
    // Nothing to do
}

int I2CMux::Reset()
{
    int ret = 0;

    if (!m_lock.try_lock())
    {
        return -1;
    }

    if (!m_rstGpio.IsOpen())
    {
        ret = m_rstGpio.Open(GPIO_IDX_I2C_MUX_nRST, GpioInstance::E_DIR_WRITE);
        if (ret < 0)
        {
            LogError << "Could not open I2CMux nRST GPIO";
            ret = -2;
        }
    }

    if (ret == 0)
    {
        if (m_rstGpio.Write(false) < 0)
        {
            LogError << "Could not reset I2CMux";
            ret = -3;
        }
    }

    if (ret == 0)
    {
        usleep(1); //500 ns according to datasheet
        if (m_rstGpio.Write(true) < 0)
        {
            LogError << "Could not take I2CMux out of reset";
            ret = -4;
        }
    }

    m_lock.unlock();

    return ret;
}

// channel range is [0..8]
int I2CMux::Acquire(int devFd, I2CMuxChannel channel, int timeoutMs)
{
    const uint8_t chIdx = static_cast<int>(channel);
    // Write to both muxes
    const uint16_t chMask = 1 << chIdx;
    const uint8_t msg[2] = {(uint8_t)(chMask & 0xFFu), (uint8_t)((chMask >> 8) & 0xFFu)};

    if (devFd < 0)
    {
        return -4;
    }

    if (!m_lock.try_lock_for(std::chrono::milliseconds(timeoutMs)))
    {
        return -1;
    }

    if (ioctl(devFd, I2C_SLAVE, k_slaveAddresses[0]) < 0)
    {
        Release();
        return -2;
    }

    if (write(devFd, &msg[0], 1u) != 1u)
    {
        Release();
        return -3;
    }

    if (ioctl(devFd, I2C_SLAVE, k_slaveAddresses[1]) < 0)
    {
        Release();
        return -2;
    }

    if (write(devFd, &msg[1], 1u) != 1u)
    {
        Release();
        return -3;
    }

    return 0;
}

void I2CMux::Release()
{
    m_lock.unlock();
}

void I2CMux::OpenFd() {
    m_fileDescriptor = ::open(k_i2cDevicePath, O_RDWR);
    if (m_fileDescriptor < 0)
    {
        std::ostringstream oss;
        oss << "Failed to open i2c device file " << k_i2cDevicePath;
        throw std::runtime_error(oss.str());
    }
}

int I2CMux::GetFd() const {
    return m_fileDescriptor;
}

std::shared_ptr<I2CMuxLock> I2CMuxLock::Lock(std::shared_ptr<I2CMux> mux, int devFd, I2CMuxChannel channel, uint32_t timeoutMs)
{
    if (mux->Acquire(devFd, channel, timeoutMs) < 0)
    {
        return nullptr;
    }

    return std::shared_ptr<I2CMuxLock>(new I2CMuxLock(mux));
}

I2CMuxLock::~I2CMuxLock()
{
    m_mux->Release();
}

I2CMuxLock::I2CMuxLock(std::shared_ptr<I2CMux> mux) : m_mux(mux)
{
}
