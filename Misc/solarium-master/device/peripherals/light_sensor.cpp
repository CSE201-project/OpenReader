#include "light_sensor.h"
#include "../../tools/logging.h"

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

static const uint8_t k_snsGainUV = 11;
static const uint8_t k_snsGainVis = 4;

enum RegAddr
{
    E_REG_ADDR_PART_ID      = 0x00,
    // ... (Unused)
    E_REG_ADDR_HOSTIN0      = 0x0A,
    E_REG_ADDR_COMMAND      = 0x0B,
    E_REG_ADDR_IRQENABLE    = 0x0F,
    E_REG_ADDR_RESPONSE1    = 0x10,
    E_REG_ADDR_RESPONSE0    = 0x11,
    E_REG_ADDR_IRQ_STATUS   = 0x12,
    E_REG_ADDR_HOSTOUT0     = 0x13,
    // ... (More HOSTOUTn)
};

#define REG_ADDR_HOSTOUT(x)                 (E_REG_ADDR_HOSTOUT0 + (x))

#define REG_VAL_COMMAND_RESET_CMD_CTR       0x00
#define REG_VAL_COMMAND_RESET_SW            0x01
#define REG_VAL_COMMAND_FORCE               0x11
#define REG_VAL_COMMAND_PARAM_QUERY(prm)    (0x40 | (prm))
#define REG_VAL_COMMAND_PARAM_SET(prm)      (0x80 | (prm))

enum ParamAddr
{
    // ... (Unused)
    E_PRM_ADDR_CHAN_LIST    = 0x01,
    E_PRM_ADDR_ADCCONFIG0   = 0x02,
    E_PRM_ADDR_ADCSENS0     = 0x03,
    E_PRM_ADDR_ADCPOST0     = 0x04,
    E_PRM_ADDR_MEASCONFIG0  = 0x05,
    E_PRM_ADDR_ADCCONFIG1   = 0x06,
    E_PRM_ADDR_ADCSENS1     = 0x07,
    E_PRM_ADDR_ADCPOST1     = 0x08,
    E_PRM_ADDR_MEASCONFIG1  = 0x09,
    // ... (Other channels)
};

#define PRM_VAL_CHAN_LIST               0x03u

#define PRM_VAL_ADC_CONFIG_UV_DEEP      0x19u
#define PRM_VAL_ADC_CONFIG_UV           0x18u
#define PRM_VAL_ADC_CONFIG_VIS_LARGE    0x0Du
#define PRM_VAL_ADC_CONFIG_VIS          0x0Bu

#define PRM_VAL_ADC_POST_24BIT          0x40u

#define PRM_VAL_ADC_SENS_HW_GAIN(x)     ((x) << 0)

static constexpr uint8_t k_i2cAddress[LIGHT_SENSOR_COUNT] = {0x55u, 0x55u};
static constexpr uint8_t k_i2cAddressAlt[LIGHT_SENSOR_COUNT] = {0x52u, 0x52u};

static constexpr I2CMuxChannel k_muxChIdx[LIGHT_SENSOR_COUNT] = {
    I2CMuxChannel::E_LIGHT_SNS1, I2CMuxChannel::E_LIGHT_SNS2
};

static constexpr uint8_t k_devId = 0x33u;

LightSensor::LightSensor(uint8_t index, std::shared_ptr<I2CMux> muxPtr) :
    m_mux(muxPtr),
    m_i2cFd(-1),
    m_index(index > (LIGHT_SENSOR_COUNT - 1) ? (LIGHT_SENSOR_COUNT - 1) : index),
    m_isOpen(false)
{
    m_i2cAddr = k_i2cAddress[m_index];
}

int LightSensor::Open(int i2cFd)
{
    uint8_t devId = 0xFFu;

    if (i2cFd < 0)
    {
        return -4;
    }

    m_i2cFd = i2cFd;

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, i2cFd, k_muxChIdx[m_index]);
    if (!muxLock)
    {
        LogError << "Could not acquire light sensor " << (int)m_index << " MUX channel";
        return -1;
    }

    // Check I2C communication
    if ((ReadReg(E_REG_ADDR_PART_ID, devId) < 0) ||
        (devId != k_devId))
    {
        LogError << "Error communicating with light sensor " << (int)m_index << " at 0x" << std::hex << std::setw(2) << std::setfill('0') << m_i2cAddr;

        // Try alternative I2C address
        m_i2cAddr = k_i2cAddressAlt[m_index];

        if ((ReadReg(E_REG_ADDR_PART_ID, devId) < 0) ||
            (devId != k_devId))
        {
            LogError << "Error communicating with light sensor " << (int)m_index << " at 0x" << std::hex << std::setw(2) << std::setfill('0') << m_i2cAddr << ". Aborting.";
            return -2;
        }
    }

    // Reset
    if (WriteCommand(REG_VAL_COMMAND_RESET_SW) < 0)
    {
        LogError << "Could not reset light sensor " << (int)m_index;
        return -3;
    }

    // Configure channels
    if (WriteParam(E_PRM_ADDR_CHAN_LIST, PRM_VAL_CHAN_LIST) < 0)
    {
        LogError << "Could not write channel list for light sensor " << (int)m_index;
        return -3;
    }

    if (ConfigureChannel(0u, PRM_VAL_ADC_CONFIG_UV, k_snsGainUV) < 0)
    {
        LogError << "Could not configure channel 0 (UV) for light sensor " << (int)m_index;
        return -3;
    }

    if (ConfigureChannel(1u, PRM_VAL_ADC_CONFIG_VIS, k_snsGainVis) < 0)
    {
        LogError << "Could not configure channel 1 (VIS) for light sensor " << (int)m_index;
        return -3;
    }

    m_isOpen = true;
    return 0;
}

int LightSensor::MeasureSync(int32_t &valUV, int32_t &valVis)
{
    if (!m_isOpen)
    {
        return -6;
    }

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, m_i2cFd, k_muxChIdx[m_index]);
    if (!muxLock)
    {
        LogError << "Could not acquire light sensor " << (int)m_index << " MUX channel";
        return -1;
    }

    if (WriteCommand(REG_VAL_COMMAND_FORCE) < 0)
    {
        LogError << "Could not force light sensor " << (int)m_index << " read";
        return -2;
    }

    uint8_t tries = 3;
    uint8_t chStat = 0x03;
    do
    {
        uint8_t irqStat = 0x00;

        usleep(100000);

        if (ReadReg(E_REG_ADDR_IRQ_STATUS, irqStat) < 0)
        {
            LogError << "Error reading IRQ status of light sensor " << (int)m_index;
            return -3;
        }

        chStat &= ~irqStat;
        tries--;
    } while ((chStat != 0) && (tries > 0));

    if (chStat != 0)
    {
        LogError << "Measurement timeout for light sensor " << (int)m_index;
        return -4;
    }

    uint8_t valCh[2][3] = {{0}};
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            if (ReadReg(REG_ADDR_HOSTOUT(i * 3 + j), valCh[i][j]) < 0)
            {
                LogError << "Error reading HOSTOUTx of light sensor " << (int)m_index;
                return -5;
            }
        }
    }

    // Convert
    valUV  = ((int32_t)valCh[0][0] << 16) | ((int32_t)valCh[0][1] << 8) | ((int32_t)valCh[0][2]);
    valVis = ((int32_t)valCh[1][0] << 16) | ((int32_t)valCh[1][1] << 8) | ((int32_t)valCh[1][2]);

    // Sign-extend
    valUV  = (valUV << 8) >> 8;
    valVis = (valVis << 8) >> 8;

    return 0;
}

void LightSensor::Close()
{
    // Nothing to do
}

LightSensor::~LightSensor()
{
    Close();
}

int LightSensor::ReadReg(uint8_t address, uint8_t &value)
{
    uint8_t msg = address;

    if (ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddr) < 0)
    {
        LogError << "Error selecting light sensor " << (int)m_index << " I2C address";
        return -2;
    }

    if (write(m_i2cFd, &msg, 1u) != 1u)
    {
        LogError << "Error writing to light sensor " << (int)m_index << " I2C";
        return -2;
    }

    if (read(m_i2cFd, &value, 1u) != 1u)
    {
        LogError << "Error reading light sensor " << (int)m_index << " I2C";
        return -2;
    }

    return 0;
}

int LightSensor::WriteReg(uint8_t address, uint8_t value)
{
    uint8_t msg[2] = {address, value};

    if (ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddr) < 0)
    {
        LogError << "Error selecting light sensor " << (int)m_index << " I2C address";
        return -2;
    }

    if (write(m_i2cFd, msg, sizeof(msg)) != sizeof(msg))
    {
        LogError << "Error writing to light sensor " << (int)m_index << " I2C\n";
        return -2;
    }

    return 0;
}

int LightSensor::ReadParam(uint8_t address, uint8_t &value)
{
    if (WriteCommand(REG_VAL_COMMAND_PARAM_QUERY(address)) < 0)
    {
        LogError << "Error querying light sensor " << (int)m_index << " parameter";
        return -1;
    }

    if (ReadReg(E_REG_ADDR_RESPONSE1, value) < 0)
    {
        LogError << "Error reading light sensor " << (int)m_index << " RESPONSE1";
        return -2;
    }

    return 0;
}

int LightSensor::WriteParam(uint8_t address, uint8_t value)
{
    if (WriteReg(E_REG_ADDR_HOSTIN0, value) < 0)
    {
        LogError << "Error writing light sensor " << (int)m_index << " HOSTIN0";
        return -1;
    }

    if (WriteCommand(REG_VAL_COMMAND_PARAM_SET(address)) < 0)
    {
        LogError << "Error setting light sensor " << (int)m_index << " parameter";
        return -2;
    }

    return 0;
}

int LightSensor::WriteCommand(uint8_t command)
{
    uint8_t resp = 0xFFu;

    if ((command != REG_VAL_COMMAND_RESET_CMD_CTR) && (command != REG_VAL_COMMAND_RESET_SW))
    {
        // Reset command counter register
        if (WriteCommand(REG_VAL_COMMAND_RESET_CMD_CTR) < 0)
        {
            LogError << "Could not reset light sensor " << (int)m_index << " command counter";
            return -1;
        }
    }

    if (WriteReg(E_REG_ADDR_COMMAND, command) < 0)
    {
        LogError << "Error writing light sensor " << (int)m_index << " command register";
        return -2;
    }

    const uint8_t expected_resp_lsb = (command == REG_VAL_COMMAND_RESET_CMD_CTR) ? 0x00u :
                                      (command == REG_VAL_COMMAND_RESET_SW)      ? 0x0Fu :
                                                                                   0x01u;

    do
    {
        if (ReadReg(E_REG_ADDR_RESPONSE0, resp) < 0)
        {
            LogError << "Could not read RESPONSE0 register of light sensor " << (int)m_index;
            return -3;
        }

    } while (((resp & 0x0F) != expected_resp_lsb) && ((resp & 0x10) == 0));

    if (resp & 0x10)
    {
        LogError << "Light sensor " << (int)m_index << " command returned error (0x" << std::hex << std::setw(2) << std::setfill('0') << (resp & 0x1F) << ")";
        return -4;
    }

    return 0;
}

int LightSensor::ConfigureChannel(uint8_t chIdx, uint8_t adcMuxVal, uint8_t hwGain)
{
    // Most parameters default
    // Select photodiodes
    if (WriteParam(chIdx == 0 ? E_PRM_ADDR_ADCCONFIG0 : E_PRM_ADDR_ADCCONFIG1, adcMuxVal) < 0)
    {
        return -1;
    }

    // 24-bit output
    if (WriteParam(chIdx == 0 ? E_PRM_ADDR_ADCPOST0 : E_PRM_ADDR_ADCPOST1, PRM_VAL_ADC_POST_24BIT) < 0)
    {
        return -1;
    }

    // HW gain
    if (WriteParam(chIdx == 0 ? E_PRM_ADDR_ADCSENS0 : E_PRM_ADDR_ADCSENS1, PRM_VAL_ADC_SENS_HW_GAIN(hwGain)) < 0)
    {
        return -1;
    }

    return 0;
}
