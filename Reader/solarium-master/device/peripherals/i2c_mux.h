#ifndef I2C_MUX_H
#define I2C_MUX_H

#include <mutex>
#include <memory>

#include "board_def.h"
#include "gpio_instance.h"

// Combined class for both I2C muxes on the board,
// since active slave devices need to have unique adresses
// across both.
// Usage: acquire channel, do stuff with slave device, release
class I2CMux
{
public:
    I2CMux();
    ~I2CMux();

    int Reset();
    int Acquire(int devFd, I2CMuxChannel channel, int timeoutMs);
    void Release();

    int GetFd() const;

private:
    // Semaphore for mutual exclusion
    std::timed_mutex m_mutex;
    std::unique_lock<std::timed_mutex> m_lock;

    // GPIO Reset pin
    GpioInstance m_rstGpio;

    int m_fileDescriptor;
    void OpenFd();

};

class I2CMuxLock
{
public:
    static std::shared_ptr<I2CMuxLock> Lock(std::shared_ptr<I2CMux> mux, int devFd, I2CMuxChannel channel, uint32_t timeoutMs = 100u);
    ~I2CMuxLock();
private:
    I2CMuxLock(std::shared_ptr<I2CMux> mux);

    std::shared_ptr<I2CMux> m_mux;
};

#endif // I2C_MUX_H
