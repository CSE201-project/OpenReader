#include "temp_sensor.h"
#include "../../tools/logging.h"

#include <cstdio>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

enum RegAddr
{
    E_REG_ADDR_TEMP         = 0x00,
    E_REG_ADDR_HUM          = 0x01,
    E_REG_ADDR_CONFIG       = 0x02,
    E_REG_ADDR_SER_ID_MSB   = 0xFB,
    E_REG_ADDR_SER_ID_MIB   = 0xFC,
    E_REG_ADDR_SER_ID_LSB   = 0xFD,
    E_REG_ADDR_MANUF_ID     = 0xFE,
    E_REG_ADDR_DEV_ID       = 0xFF,
};

#define REG_VAL_CONFIG_RST          (1 << 15)
#define REG_VAL_CONFIG_HEAT_EN      (1 << 13)
#define REG_VAL_CONFIG_MODE_SINGLE  (0 << 12)
#define REG_VAL_CONFIG_MODE_BOTH    (1 << 12)
#define REG_VAL_CONFIG_BTST_GT      (0 << 11)
#define REG_VAL_CONFIG_BTST_LT      (1 << 11)
#define REG_VAL_CONFIG_TRES_14BIT   (0 << 10)
#define REG_VAL_CONFIG_TRES_11BIT   (1 << 10)
#define REG_VAL_CONFIG_HRES_14BIT   (0 << 8)
#define REG_VAL_CONFIG_HRES_11BIT   (1 << 8)
#define REG_VAL_CONFIG_HRES_8BIT    (2 << 8)

static constexpr uint8_t k_i2cAddress[TempSensor::E_TEMP_SNS_TYPE_COUNT] = {
    [TempSensor::E_TEMP_SNS_TYPE_LOCAL]     = 0x40u,
    [TempSensor::E_TEMP_SNS_TYPE_AMBIENTAL] = 0x40u,
};

static constexpr I2CMuxChannel k_muxChIdx[TempSensor::E_TEMP_SNS_TYPE_COUNT] = {
    [TempSensor::E_TEMP_SNS_TYPE_LOCAL]     = I2CMuxChannel::E_HUMIDITY,
    [TempSensor::E_TEMP_SNS_TYPE_AMBIENTAL] = I2CMuxChannel::E_AMB_HUMIDITY
};

static constexpr uint16_t k_devId = 0x1050u;
static constexpr uint16_t k_manufId = 0x5449u;

TempSensor::TempSensor(Type type, std::shared_ptr<I2CMux> muxPtr) :
    m_mux(muxPtr),
    m_i2cFd(-1),
    m_type(type),
    m_isOpen(false)
{

}

int TempSensor::Open(int i2cFd)
{
    uint16_t devId = 0xFFFFu;
    uint16_t manufId = 0xFFFFu;

    if (i2cFd < 0)
    {
        return -4;
    }

    m_i2cFd = i2cFd;

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, i2cFd, k_muxChIdx[m_type]);
    if (!muxLock)
    {
        LogError << "temp_sensor : Could not acquire sensor MUX channel";
        return -1;
    }

    // Check I2C communication with potentiometers
    if ((ReadReg(E_REG_ADDR_DEV_ID, devId) < 0) ||
        (devId != k_devId))
    {
        LogError << "Error communicating with temp sensor: bad device ID";
        return -2;
    }

    if ((ReadReg(E_REG_ADDR_MANUF_ID, manufId) < 0) ||
        (manufId != k_manufId))
    {
        LogError << "Error communicating with temp sensor: bad manufacturer ID";
        return -2;
    }

    // Reset
    if (WriteReg(E_REG_ADDR_CONFIG, REG_VAL_CONFIG_RST) < 0)
    {
        LogError << "Could not reset temp sensor";
        return -3;
    }

    // Configure
    if (WriteReg(E_REG_ADDR_CONFIG, REG_VAL_CONFIG_MODE_BOTH | REG_VAL_CONFIG_HRES_14BIT | REG_VAL_CONFIG_TRES_14BIT) < 0)
    {
        LogError << "Could not configure temp sensor";
        return -4;
    }

    m_isOpen = true;
    return 0;
}

int TempSensor::MeasureSync(float &tempDegC, float &relHumPercent)
{
    if (!m_isOpen)
    {
        return -6;
    }

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, m_i2cFd, k_muxChIdx[m_type]);
    if (!muxLock)
    {
        LogError << "temp_sensor : Could not acquire sensor MUX channel";
        return -1;
    }

    // Measure
    uint16_t tempRaw, humRaw;
    if (TriggerAndMeasureBoth(tempRaw, humRaw) < 0)
    {
        LogError << "Could not read temp/hum measurement";
        return -2;
    }

    // Convert
    tempDegC        = ((float)tempRaw / (1 << 16)) * 165.f - 40.f;
    relHumPercent   = ((float)humRaw / (1 << 16)) * 100.f;

    return 0;
}

void TempSensor::Close()
{
    // Nothing to do
}

TempSensor::~TempSensor()
{
    Close();
}

int TempSensor::ReadReg(uint8_t address, uint16_t &value)
{
    uint8_t msgWr = address;
    uint8_t msgRd[2] = {0};

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress[m_type]) < 0)
    {
        LogError << "Error selecting temp sensor I2C address";
        return -2;
    }

    if (write(m_i2cFd, &msgWr, 1u) != 1u)
    {
        LogError << "Error writing to temp sensor I2C";
        return -2;
    }

    if (read(m_i2cFd, msgRd, 2u) != 2u)
    {
        LogError << "Error reading temp sensor I2C";
        return -2;
    }

    value = ((uint16_t)msgRd[0] << 8) | (uint16_t)msgRd[1];

    return 0;
}

int TempSensor::WriteReg(uint8_t address, uint16_t value)
{
    uint8_t msg[3] = {address, (uint8_t)(value >> 8), (uint8_t)(value & 0xFFu)};

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress[m_type]) < 0)
    {
        LogError << "Error selecting temp sensor I2C address";
        return -2;
    }

    if (write(m_i2cFd, msg, sizeof(msg)) != sizeof(msg))
    {
        LogError << "Error writing to temp sensor I2C";
        return -2;
    }

    return 0;
}

int TempSensor::TriggerAndMeasureBoth(uint16_t &tempRaw, uint16_t &humRaw)
{
    uint8_t msgWr = 0x00;
    uint8_t msgRd[4] = {0};

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress[m_type]) < 0)
    {
        LogError << "Error selecting temp sensor I2C address";
        return -2;
    }

    if (write(m_i2cFd, &msgWr, 1u) != 1u)
    {
        LogError << "Error writing to temp sensor I2C";
        return -2;
    }

    // Wait for measurements to complete
    usleep(14000);

    if (read(m_i2cFd, msgRd, 4u) != 4u)
    {
        LogError << "Error reading temp sensor I2C";
        return -2;
    }

    tempRaw = ((uint16_t)msgRd[0] << 8) | (uint16_t)msgRd[1];
    humRaw  = ((uint16_t)msgRd[2] << 8) | (uint16_t)msgRd[3];

    return 0;
}
