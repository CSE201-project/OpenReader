#include "charger.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "../../tools/logging.h"

static constexpr uint8_t k_i2cAddress = 0x55u;
static constexpr I2CMuxChannel k_muxChIdx = I2CMuxChannel::E_FUEL_GAUGE;

static constexpr uint16_t k_battCapacityMAh = 6600u;
static constexpr uint16_t k_battSleepCurrent_mA = 2u;
static constexpr uint16_t k_battHibernateCurrent_mA = 0u;
static constexpr uint16_t k_devID = 0x0530u;

#define CHG_CMD_NOT_REQ_MASK (0x20u) // CMD_NOT_REQ

/* STAT field */
enum ChargerStatusStat
{
  BATTD_NO_VALID_SOURCE       = 0x0,   /* no valid source detected */
  BATTD_IN_READY              = 0x1,   /* in ready */
  BATTD_USB_READY             = 0x2,   /* usb ready */
  BATTD_CHG_FROM_IN           = 0x3,   /* charging from in */
  BATTD_CHG_FROM_USB          = 0x4,   /* charging from usb */
  BATTD_CHG_DONE              = 0x5,   /* charging done */
  BATTD_FAULT                 = 0x7,   /* fault */
};

/* FAULT field */
enum ChargerStatusFault
{
  BATTD_NORMAL                = 0x0,   /* normal */
  BATTD_THERMAL_SHUTDOWN      = 0x1,   /* thermal shutdown */
  BATTD_BATTERY_TEMP_FAULT    = 0x2,   /* battery temperature fault */
  BATTD_WATCHDOG_EXPIRED      = 0x3,   /* watchdog timer expired */
  BATTD_SAFETY_TIMER_EXPIRED  = 0x4,   /* safety timer expired */
  BATTD_IN_SUPPLY_FAULT       = 0x5,   /* in supply fault */
  BATTD_USB_SUPPLY_FAULT      = 0x6,   /* usb supply fault */
  BATTD_BATTERY_FAULT         = 0x7,   /* battery fault */
};

struct ChargerStatusBits
{
  unsigned           rsvd            : 2;  /* reserved */
  unsigned           init            : 1;  /* indicates when the gauge is in the initializing process of the charge state machine. */
  unsigned           authfail        : 1;  /* indicates that the part number read from the charger in Chrgr_Rev_RegA() does not match the expected
                                              part number as programmed in the Product Number of the gauge data flash. */
  unsigned           wfail           : 1;  /* indicates that a write intended to the charger has failed. True when set. */
  unsigned           denied          : 1;  /* indicates host access to a charger register was denied. The fuel gauge allows access to all charger registers
                                              for which this bit would never be set. */
  unsigned           err             : 1;  /* indicates I2C communication error between gauge and charger. True when set. */
  unsigned           wait_cmd        : 1;  /* indicates that the gauge is waiting on GG_CHGCNTRL_ENABLE subcommand so that the gauge can
                                              control the charger in host mode. True when set. */

  ChargerStatusFault fault           : 3;  /* FAULT field from status/control reg ( memory location 00 ). */
  unsigned           supply_sel      : 1;
  ChargerStatusStat  stat            : 3;  /* STAT field from status/control reg ( memory location 00 ). */
  unsigned           tmr_rst         : 1;
};

union ChargerStatusRaw
{
  uint16_t all;
  ChargerStatusBits bit;
};

Charger::Charger(std::shared_ptr<I2CMux> muxPtr) : m_mux(muxPtr),
                                                   m_i2cFd(-1),
                                                   m_isOpen(false)
{
}

int Charger::Open(int i2cFd)
{
    uint16_t devID = 0;
    if (i2cFd < 0)
    {
        return -1;
    }

    m_i2cFd = i2cFd;

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, i2cFd, k_muxChIdx);
    if (!muxLock)
    {
        LogError << "Could not acquire charger MUX channel";
        return -2;
    }

    // Check I2C communication with potentiometers
    if ((ReadCtrlCmd(CHG_CMD_ID_DEV_ID, devID) < 0) ||
        (devID != k_devID))
    {
        LogError << "Error communicating with charger (bad device ID)";
        return -3;
    }

    // update gauge flash data if necessary: battery capacity, set charger in host mode by default.
    int ret = Unseal();
    if (ret < 0)
    {
        LogError << "Unseal() returned status " << ret;
        return -4;
    }

    usleep(1700000);

    ret = SetChgInHostMode();
    if (ret < 0)
    {
        LogError << "SetChgInHostMode() returned status " << ret;
        return -5;
    }

    ret = UpdateBattCapacity();
    if (ret < 0)
    {
        LogError << "UpdateBattCapacity() returned status " << ret;
        return -6;
    }

    ret = SetSleepI();
    if (ret < 0)
    {
        LogError << "SetSleepI() returned status " << ret;
        return -7;
    }

    ret = SetHibernateI();
    if (ret < 0)
    {
        LogError << "SetHibernateI() returned status " << ret;
        return -8;
    }

    ret = Reset(); // reset the gauge to force it to use the new parameters. The reset also seals the gauge.
    if (ret < 0)
    {
        LogError << "Reset() returned status " << ret;
        return -9;
    }

    usleep(100000);

    ret = WriteCtrlCmd(CHG_CMD_ID_EN_CHARGER);
    if (ret < 0)
    {
        LogError << "WriteCtrlCmd() returned status " << ret;
        return -10;
    }

    usleep(100000);

    ret = WriteCtrlCmd(CHG_CMD_ID_HOST_MODE);
    if (ret < 0)
    {
        LogError << "WriteCtrlCmd() returned status " << ret;
        return -11;
    }

    m_isOpen = true;
    return 0;
}

int Charger::MeasureSync(uint16_t &socPercent, uint16_t &voltage_mV, ChargerStatus &status)
{
    if (!m_isOpen)
    {
        return -1;
    }

    // Acquire I2C channel (auto-release)
    auto muxLock = I2CMuxLock::Lock(m_mux, m_i2cFd, k_muxChIdx);
    if (!muxLock)
    {
        LogError << "charger : Could not acquire sensor MUX channel";
        return -2;
    }

    int ret = VoltageRead(voltage_mV);
    if (ret < 0)
    {
        LogError << "Error: VoltageRead() returned status " << ret;
        return -3;
    }

    ret = TrueSOCRead(socPercent);
    if (ret < 0)
    {
        LogError << "Error: TrueSOCRead() returned status " << ret;
        return -4;
    }

    ret = StatusChargerRead(status);
    if (ret < 0)
    {
        LogError << "Error: StatusChargerRead() returned status " << ret;
        return -5;
    }

    return 0;
}

void Charger::Close()
{
    // Nothing to do
}

Charger::~Charger()
{
    Close();
}

int Charger::ReadReg(ChgRegAddr address, uint16_t &value)
{
    uint8_t msgWr = address;
    uint8_t msgRd[2] = {0};

    if (address >= CHG_REG_ADDR_FIRST_INVALID_ADDR)
    {
        return -1;
    }

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress) < 0)
    {
        LogError << "Error selecting charger I2C address";
        return -2;
    }

    if (write(m_i2cFd, &msgWr, 1u) != 1u)
    {
        LogError << "Error writing to charger I2C";
        return -3;
    }

    if (read(m_i2cFd, msgRd, 2u) != 2u)
    {
        LogError << "Error reading charger I2C";
        return -4;
    }

    value = ((uint16_t)msgRd[1] << 8) | (uint16_t)msgRd[0];

    return 0;
}

int Charger::WriteReg(ChgRegAddr address, uint16_t value)
{
    uint8_t msg[3] = {(uint8_t)address, (uint8_t)(value & 0xFFu), (uint8_t)(value >> 8)};

    if (address >= CHG_REG_ADDR_FIRST_INVALID_ADDR)
    {
        return -1;
    }

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress) < 0)
    {
        LogError << "Error selecting charger I2C address";
        return -2;
    }

    if (write(m_i2cFd, msg, sizeof(msg)) != sizeof(msg))
    {
        LogError << "Error writing to charger I2C";
        return -3;
    }

    return 0;
}

int Charger::WriteCtrlCmd(ChgCmdId subcmd)
{
    return WriteReg(ChgRegAddr::CHG_REG_ADDR_CTRL, (uint16_t)subcmd);
}

int Charger::ReadCtrlCmd(ChgCmdId subcmd, uint16_t &value, uint32_t delayUs)
{
    int ret = WriteCtrlCmd(subcmd);
    if (ret < 0)
    {
        LogError << "Error writing charger control command: " << ret;
        return -1;
    }

    usleep(delayUs);

    ret = ReadReg(ChgRegAddr::CHG_REG_ADDR_CTRL, value);
    if (ret < 0)
    {
        LogError << "Error reading charger control register: " << ret;
        return -2;
    }

    return ret;
}

int Charger::WriteExtendedCmd(ChgRegAddrExt extCmd, uint8_t *data, uint8_t len)
{
    uint8_t txData[32 + 1];

    if ((extCmd >= CHG_REG_ADDR_EXT_1ST_INVALID_ADDR) || (len > 32))
    {
        return -1;
    }

    if (data == NULL)
    {
        return -2;
    }

    txData[0] = extCmd;
    memcpy(&txData[1], data, len);

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress) < 0)
    {
        LogError << "Error selecting charger I2C address";
        return -3;
    }

    if (write(m_i2cFd, txData, len + 1) != len + 1)
    {
        LogError << "Error writing to charger I2C";
        return -4;
    }

    return 0;
}

int Charger::ReadExtendedCmd(ChgRegAddrExt extCmd, uint8_t *data, uint8_t len)
{
    uint8_t txData[1] = {(uint8_t)extCmd};

    if ((extCmd >= CHG_REG_ADDR_EXT_1ST_INVALID_ADDR) || (len > 32))
    {
        return -1;
    }

    if (data == NULL)
    {
        return -2;
    }

    memset(data, 0x0, len);

    if (ioctl(m_i2cFd, I2C_SLAVE, k_i2cAddress) < 0)
    {
        LogError << "Error selecting charger I2C address";
        return -2;
    }

    if (write(m_i2cFd, &txData, 1u) != 1u)
    {
        LogError << "Error writing to charger I2C";
        return -3;
    }

    if (read(m_i2cFd, data, len) != len)
    {
        LogError << "Error reading charger I2C";
        return -4;
    }

    return 0;
}

int Charger::Unseal()
{
    int ret = WriteCtrlCmd(CHG_CMD_ID_UNSEAL_KEY1);
    if (ret < 0)
    {
        LogError << "Error writing to control register: " << ret;
        return -1;
    }

    usleep(15000);

    ret = WriteCtrlCmd(CHG_CMD_ID_UNSEAL_KEY2);
    if (ret < 0)
    {
        LogError << "Error writing to control register: " << ret;
        return -1;
    }

    usleep(15000);

    return 0;
}

int Charger::Seal()
{
    int ret = WriteCtrlCmd(CHG_CMD_ID_SEAL);
    if (ret < 0)
    {
        LogError << "Error writing to control register: " << ret;
        return -1;
    }

    return 0;
}

int Charger::ReadDataFlash(ChgDataFlashSubclass subcls, ChgDataFlashBlock block, ChgRegAddrExt off, uint8_t *readData, uint8_t len)
{
    uint8_t param = 0;
    uint8_t subclsU8 = (uint8_t)subcls;
    uint8_t blockU8 = (uint8_t)block;

    if (readData == NULL)
    {
        return -1;
    }

    // enable block data flash control.
    int ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_BLOCK_DATA_CTRL, &param, sizeof(param));
    if (ret < 0)
    {
        LogError << "Error writing to charger BlockDataControl: " << ret;
        return -2;
    }

    // specify the subclass ID
    ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_DATA_FLASH_CLASS, &subclsU8, sizeof(subclsU8));
    if (ret < 0)
    {
        LogError << "Error writing to charger DataFlashClass: " << ret;
        return -3;
    }
    // specify the block offset
    ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_DATA_FLASH_BLOCK, &blockU8, sizeof(blockU8));
    if (ret < 0)
    {
        LogError << "Error writing to charger DataFlashBlock: " << ret;
        return -4;
    }
    usleep(10000);

    // read the data
    ret = ReadExtendedCmd(off, readData, len);
    if (ret < 0)
    {
        LogError << "Error reading charger extended command: " << ret;
        return -5;
    }
    usleep(10000);

    return 0;
}

int Charger::WriteDataFlash(ChgDataFlashSubclass subcls, ChgDataFlashBlock block, ChgRegAddrExt off, uint8_t *writeData, uint8_t len)
{
    uint8_t param = 0;
    uint8_t subclsU8 = (uint8_t)subcls;
    uint8_t blockU8 = (uint8_t)block;

    if (writeData == NULL)
    {
        return -1;
    }

    // enable block data flash control.
    int ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_BLOCK_DATA_CTRL, &param, sizeof(param));
    if (ret < 0)
    {
        LogError << "Error writing to charger BlockDataControl: " << ret;
        return -2;
    }

    // specify the subclass ID
    ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_DATA_FLASH_CLASS, &subclsU8, sizeof(subclsU8));
    if (ret < 0)
    {
        LogError << "Error writing to charger DataFlashClass: " << ret;
        return -3;
    }
    // specify the block offset
    ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_DATA_FLASH_BLOCK, &blockU8, sizeof(blockU8));
    if (ret < 0)
    {
        LogError << "Error writing to charger DataFlashBlock: " << ret;
        return -4;
    }
    usleep(10000);

    // read the data
    ret = WriteExtendedCmd(off, writeData, len);
    if (ret < 0)
    {
        LogError << "Error writing charger extended command: " << ret;
        return -5;
    }
    usleep(10000);

    return 0;
}

static uint8_t CalcCrc(uint8_t oldData, uint8_t newData, uint8_t oldCrc)
{
    uint8_t temp = 0;

    temp = (255 - oldCrc - oldData) % 256;

    return (255 - ((temp + newData) % 256));
}

int Charger::UpdateBattCapacity()
{
    uint8_t data_block[32] = {0};

    LogInfo << "Updating battery capacity";

    // read old battery capacity
    int ret = ReadDataFlash(CHG_DF_SC_DATA, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
    if (ret < 0)
    {
        LogError << "Could not read charger data flash: " << ret;
        return -1;
    }

    LogInfo << "Current battery capacity = " << ((data_block[8] << 8) | data_block[9]) << " mAh";

    if (((data_block[8] << 8) | data_block[9]) == k_battCapacityMAh)
    {
        // nothing to be done
    }
    else
    {
        data_block[8] = (uint8_t)(k_battCapacityMAh >> 8);
        data_block[9] = (uint8_t)(k_battCapacityMAh & 0xFF);

        uint16_t sum = 0;
        for (uint8_t i = 0; i < 32; i++)
        {
            sum += data_block[i];
        }

        uint8_t new_crc = 255 - (sum & 0xFF);

        // write new battery capacity
        ret = WriteDataFlash(CHG_DF_SC_DATA, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
        if (ret < 0)
        {
            LogError << "Could not write charger data flash: " << ret;
            return -2;
        }

        usleep(10000);

        // write new CRC
        // note: the actual flash writting is performed after a GOOD CRC is sent to the gauge.

        ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_CRC, &new_crc, sizeof(new_crc));
        if (ret < 0)
        {
            LogError << "Could not write charger extended cmd: " << ret;
            return -3;
        }

        // DO NOT read the CRC here. It can lead to flash corruption which renders the gauge IC useless. Information taken from the Linux driver.

        // wait for the flash write to complete.
        usleep(200000);
    }

    return 0;
}

int Charger::SetChgInHostMode()
{
    uint8_t data_read = 0;
    uint8_t data_write = 0;
    uint8_t current_crc = 0;
    uint8_t new_crc = 0;

    int ret = ReadDataFlash(CHG_DF_SC_CHG_OPT, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_CHG_OPT, &data_read, sizeof(data_read));
    if (ret < 0)
    {
        LogError << "Could not read charger data flash: " << ret;
        return -1;
    }

    LogInfo << "Current CHG OPT = 0x" << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)data_read;

    if (data_read & CHG_CMD_NOT_REQ_MASK)
    {
        // nothing to be done
    }
    else
    {
        // read block checksum
        ret = ReadExtendedCmd(CHG_REG_ADDR_EXT_CRC, &current_crc, 1);
        if (ret < 0)
        {
            LogError << "Could not read charger CRC: " << ret;
            return -2;
        }

        data_write = data_read | CHG_CMD_NOT_REQ_MASK;
        ret = WriteDataFlash(CHG_DF_SC_CHG_OPT, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_CHG_OPT, &data_write, sizeof(data_write));
        if (ret < 0)
        {
            LogError << "Could not write charger data flash: " << ret;
            return -3;
        }

        // compute new CRC
        new_crc = CalcCrc(data_read, data_write, current_crc);

        usleep(10000);

        // write new CRC
        // note: the actual flash writting is performed after a GOOD CRC is sent to the gauge.
        ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_CRC, &new_crc, sizeof(new_crc));
        if (ret < 0)
        {
            LogError << "Could not write charger CRC: " << ret;
            return -4;
        }

        // DO NOT read the CRC here. It can lead to flash corruption which renders the gauge IC useless. Information taken from the Linux driver.

        // wait for the flash write to complete.
        usleep(200000);
    }

    return 0;
}

int Charger::SetSleepI()
{
    uint8_t data_block[32];

    // read current value
    int ret = ReadDataFlash(CHG_DF_SC_POWER, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
    if (ret < 0)
    {
        LogError << "Could not read charger data flash: " << ret;
        return -1;
    }

    if (((data_block[4] << 8) | data_block[5]) == k_battSleepCurrent_mA)
    {
        // nothing to be done
    }
    else
    {
        data_block[4] = (uint8_t)(k_battSleepCurrent_mA >> 8);
        data_block[5] = (uint8_t)(k_battSleepCurrent_mA & 0xFF);

        uint16_t sum = 0;
        for (uint8_t i = 0; i < 32; i++)
        {
            sum += data_block[i];
        }
        uint8_t new_crc = 255 - (sum & 0xFF);

        // write new battery capacity
        ret = WriteDataFlash(CHG_DF_SC_POWER, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
        if (ret < 0)
        {
            LogError << "Could not write charger data flash: " << ret;
            return -3;
        }

        usleep(10000);

        // write new CRC
        // note: the actual flash writting is performed after a GOOD CRC is sent to the gauge.

        ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_CRC, &new_crc, sizeof(new_crc));
        if (ret < 0)
        {
            LogError << "Could not write charger CRC: " << ret;
            return -4;
        }

        // DO NOT read the CRC here. It can lead to flash corruption which renders the gauge IC useless. Information taken from the Linux driver.

        // wait for the flash write to complete.
        usleep(200000);
    }

    return 0;
}

int Charger::SetHibernateI()
{
    uint8_t data_block[32];

    // read current value
    int ret = ReadDataFlash(CHG_DF_SC_POWER, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
    if (ret < 0)
    {
        LogError << "Could not read charger data flash: " << ret;
        return -1;
    }

    if (((data_block[12] << 8) | data_block[13]) == k_battHibernateCurrent_mA)
    {
        // nothing to be done
    }
    else
    {
        data_block[12] = (uint8_t)(k_battHibernateCurrent_mA >> 8);
        data_block[13] = (uint8_t)(k_battHibernateCurrent_mA & 0xFF);

        uint16_t sum = 0;
        for (uint8_t i = 0; i < 32; i++)
        {
            sum += data_block[i];
        }
        uint8_t new_crc = 255 - (sum & 0xFF);

        // write new battery capacity
        ret = WriteDataFlash(CHG_DF_SC_POWER, CHG_DF_BLOCK_OFFSET_0, CHG_REG_ADDR_EXT_1ST_ADDR, data_block, sizeof(data_block));
        if (ret < 0)
        {
            LogError << "Could not write charger data flash: " << ret;
            return -3;
        }

        usleep(10000);

        // write new CRC
        // note: the actual flash writting is performed after a GOOD CRC is sent to the gauge.

        ret = WriteExtendedCmd(CHG_REG_ADDR_EXT_CRC, &new_crc, sizeof(new_crc));
        if (ret < 0)
        {
            LogError << "Could not write charger CRC: " << ret;
            return -4;
        }

        // DO NOT read the CRC here. It can lead to flash corruption which renders the gauge IC useless. Information taken from the Linux driver.

        // wait for the flash write to complete.
        usleep(200000);
    }

    return 0;
}

int Charger::Reset()
{
    int ret = Unseal();
    if (ret < 0)
    {
        LogError << "Could not unseal charger: " << ret;
        return -1;
    }

    ret = WriteCtrlCmd(CHG_CMD_ID_RST);
    if (ret < 0)
    {
        LogError << "Could not reset charger: " << ret;
        return -2;
    }

    usleep(10000);

    return 0;
}

int Charger::VoltageRead(uint16_t &v_in_mv)
{
    int ret = ReadReg(CHG_REG_ADDR_VOLTAGE, v_in_mv);
    if (ret < 0)
    {
        LogError << "Error reading charger register: " << ret;
        return -1;
    }

    return 0;
}

int Charger::TrueSOCRead(uint16_t &true_soc)
{
    int ret = ReadReg(CHG_REG_ADDR_TRUE_SOC, true_soc);
    if (ret < 0)
    {
        LogError << "Error reading charger register: " << ret;
        return -1;
    }

    return 0;
}

int Charger::StatusChargerRead(ChargerStatus &status)
{
    ChargerStatusRaw rawStatus = {};

    // read charger status ( "status/ctrl" reg )
    int ret = ReadReg(CHG_REG_ADDR_STATUS, rawStatus.all);
    if (ret < 0)
    {
        LogError << "Error reading charger register: " << ret;
        return -1;
    }

    status.isInitializing = rawStatus.bit.init;
    status.hasError = rawStatus.bit.authfail || rawStatus.bit.wfail || rawStatus.bit.denied || rawStatus.bit.err;
    status.isGaugeWaitingForCommand = rawStatus.bit.wait_cmd;
    status.faultType = (ChargerStatus::FaultType)rawStatus.bit.fault;
    status.chargerStatus = (ChargerStatus::Status)rawStatus.bit.stat;

    return 0;
}
