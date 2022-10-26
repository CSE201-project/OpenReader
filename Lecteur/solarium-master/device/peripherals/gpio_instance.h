#ifndef GPIOINSTANCE_H
#define GPIOINSTANCE_H

#include <cstdint>

class GpioInstance
{
public:
    enum Direction
    {
        E_DIR_READ = 0,
        E_DIR_WRITE
    };

    GpioInstance();
    ~GpioInstance();

    int Open(uint32_t gpioIdx, Direction dir);
    int Write(bool value);
    int Read(bool &value);
    void Close();

    bool IsOpen();

private:
    int m_valFd;
    uint32_t m_gpioIdx;
    Direction m_dir;
    bool m_isOpen;

    int Export(uint32_t gpioIdx);
    int OpenValueFd(uint32_t gpioIdx, Direction dir, int &fd);
    int ConfigureDir(uint32_t gpioIdx, Direction dir);
};

#endif // GPIOINSTANCE_H
