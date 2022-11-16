#include "gpio_instance.h"

#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include <cstdio>

#include "../../tools/logging.h"

GpioInstance::GpioInstance() : m_valFd(-1), m_gpioIdx(0), m_dir(E_DIR_READ), m_isOpen(false)
{

}

GpioInstance::~GpioInstance()
{
    Close();
}

int GpioInstance::Open(uint32_t gpioIdx, GpioInstance::Direction dir)
{
    // Close if already open
    Close();

    /// Export GPIO port

    // Check if already exported
    if (OpenValueFd(gpioIdx, dir, m_valFd) < 0)
    {
        if (Export(gpioIdx) < 0)
        {
            return -1;
        }

        if (OpenValueFd(gpioIdx, dir, m_valFd) < 0)
        {
            LogError << "Could not open value file";
            return -2;
        }
    }

    /// At this point, value fd is open

    // Configure direction
    if (ConfigureDir(gpioIdx, dir) < 0)
    {
        return -3;
    }

    m_gpioIdx = gpioIdx;
    m_dir = dir;
    m_isOpen = true;
    return 0;
}

int GpioInstance::Write(bool value)
{
    if (m_valFd < 0)
    {
        return -1;
    }

    int ret = write(m_valFd, value ? "1" : "0", 2);
    if (ret < 0)
    {
        LogError << "Could not write GPIO value";
        return -2;
    }

    return 0;
}

int GpioInstance::Read(bool &value)
{
    char in[2] = "";
    int ret = 0;

    if (m_valFd < 0)
    {
        return -1;
    }

    do
    {
        ret = read(m_valFd, in, 1);
    } while (ret == 0);

    if (ret < 0)
    {
        LogError << "Could not read GPIO value";
        return -1;
    }

    value = atoi(in);
    return 0;
}

void GpioInstance::Close()
{
    if (m_valFd >= 0)
    {
        close(m_valFd);
        m_valFd = -1;
    }
    m_isOpen = false;
}

bool GpioInstance::IsOpen()
{
    return m_isOpen;
}

int GpioInstance::Export(uint32_t gpioIdx)
{
    int ret = 0;
    int efd = open("/sys/class/gpio/export", O_WRONLY);

    if(efd >= 0)
    {
        char buf[5] = "";
        sprintf(buf, "%d", gpioIdx);
        if (write(efd, buf, strlen(buf)) < 0)
        {
            ret = -2;
        }
        close(efd);
    }
    else
    {
        LogError << "Couldn't open export file";
        ret = -1;
    }

    return ret;
}

int GpioInstance::OpenValueFd(uint32_t gpioIdx, GpioInstance::Direction dir, int &fd)
{
    const int oflag = (dir == E_DIR_READ) ? O_RDONLY : O_WRONLY;
    char buf[50] = "";

    sprintf(buf, "/sys/class/gpio/gpio%u/value", gpioIdx);
    fd = open(buf, oflag);
    if (fd < 0)
    {
        return -1;
    }

    return 0;
}

int GpioInstance::ConfigureDir(uint32_t gpioIdx, GpioInstance::Direction dir)
{
    int ret = 0;
    char buf[50];

    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpioIdx);

    int gpiofd = open(buf, O_WRONLY);
    if(gpiofd < 0)
    {
        LogError << "Couldn't open direction file";
        return -1;
    }

    if (dir == E_DIR_WRITE)
    {
        if (3 != write(gpiofd, "out", 3))
        {
            LogError << "Couldn't set GPIO direction to out";
            ret = -3;
        }
    }
    else
    {
        if (2 != write(gpiofd, "in", 2))
        {
            LogError << "Couldn't set GPIO direction to in";
            ret = -4;
        }
    }

    close(gpiofd);
    return ret;
}
