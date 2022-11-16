#ifndef CHARGER_H
#define CHARGER_H

#include <memory>
#include <cstdint>

#include "i2c_mux.h"

/* standard commands */
enum ChgRegAddr
{
    CHG_REG_ADDR_CTRL               = 0x00, /* control register */
    CHG_REG_ADDR_TEMPERATURE        = 0x06, /* temperature register */
    CHG_REG_ADDR_INT_TEMPERATURE    = 0x32, /* internal temperature register */
    CHG_REG_ADDR_VOLTAGE            = 0x08, /* voltage register */
    CHG_REG_ADDR_FLAGS              = 0x0A, /* flags register */
    CHG_REG_ADDR_AVG_I              = 0x14, /* average current register */
    CHG_REG_ADDR_SOC                = 0x2C, /* state of charge register */
    CHG_REG_ADDR_TRUE_SOC           = 0x2E, /* true state of charge register */
    CHG_REG_ADDR_INSTANTANEUS_I     = 0x30, /* instantaneus current register */
    // charger
    CHG_REG_ADDR_STATUS             = 0x75, /* status register */
    CHG_REG_ADDR_CHGR0              = 0x76, /* status/control register */

    CHG_REG_ADDR_FIRST_INVALID_ADDR = 0x80
};

/* subcommands for control command */
enum ChgCmdId
{
    CHG_CMD_ID_STA              = 0x0000, /* status */
    CHG_CMD_ID_DEV_ID           = 0x0001, /* device type */
    CHG_CMD_ID_SEAL             = 0x0020, /* seal the access */
    CHG_CMD_ID_BATTERY_INSERT   = 0x000D, /* battery insert */
    CHG_CMD_ID_BATTERY_REMOVE   = 0x000E, /* battery remove */
    CHG_CMD_ID_EN_CHARGER       = 0x001A, /* enable charger */
    CHG_CMD_ID_DIS_CHARGER      = 0x001B, /* disable charger */
    CHG_CMD_ID_HOST_MODE        = 0x001C, /* gg_chgrctl_enable = charger is host mode */
    CHG_CMD_ID_DEFAULT_MODE     = 0x001D, /* gg_chgrctl_disable = charger in default mode */
    CHG_CMD_ID_RST              = 0x0041, /* full reset */

    CHG_CMD_ID_UNSEAL_KEY1      = 0x0414, /* key part 1 */
    CHG_CMD_ID_UNSEAL_KEY2      = 0x3672, /* key part 2 */
};

/* extended commands */
enum ChgRegAddrExt
{
    CHG_REG_ADDR_EXT_DATA_FLASH_CLASS   = 0x3E, /* data flash class */
    CHG_REG_ADDR_EXT_DATA_FLASH_BLOCK   = 0x3F, /* data flash block */
    CHG_REG_ADDR_EXT_1ST_ADDR           = 0x40, /* address of the first byte in a block */
    CHG_REG_ADDR_EXT_OP_CONF_B          = 0x4B, /* address of the OP_CONF_B */
    CHG_REG_ADDR_EXT_DESIGN_CAP         = 0x48, /* address of the design capacity field */
    CHG_REG_ADDR_EXT_CHG_OPT            = 0x40, /* address of the charger options field */
    CHG_REG_ADDR_EXT_SOC_SET_THRES      = 0x40, /* address of the SOC set threshold field */
    CHG_REG_ADDR_EXT_SOC_CLEAR_THRES    = 0x41, /* address of the SOC clear threshold field */
    CHG_REG_ADDR_EXT_CRC                = 0x60, /* crc */
    CHG_REG_ADDR_EXT_BLOCK_DATA_CTRL    = 0x61, /* block data control */

    CHG_REG_ADDR_EXT_1ST_INVALID_ADDR   = 0x62
};

/* subclass of the data flash */
enum ChgDataFlashSubclass
{
    CHG_DF_SC_DATA          = 0x30, /* data subclass */
    CHG_DF_SC_DISCHARGE     = 0x31, /* discharge subclass */
    CHG_DF_SC_REGISTERS     = 0x40, /* registers subclass */
    CHG_DF_SC_POWER         = 0x44, /* power subclass */
    CHG_DF_SC_CHG_OPT       = 0x4E, /* charger options subclass */
};

/* block offset location in data flash */
enum ChgDataFlashBlock
{
    CHG_DF_BLOCK_OFFSET_0 = 0x0, /* block offset 0 */
    CHG_DF_BLOCK_OFFSET_1 = 0x1, /* block offset 1 */
};

struct ChargerStatus
{
    enum FaultType
    {
        E_FLT_NONE                  = 0,
        E_FLT_THERMAL_SHUTDOWN,
        E_FLT_BATTERY_TEMP_FAULT,
        E_FLT_WATCHDOG_EXPIRED,
        E_FLT_SAFETY_TIMER_EXPIRED,
        E_FLT_IN_SUPPLY_FAULT,
        E_FLT_USB_SUPPLY_FAULT,
        E_FLT_BATTERY_FAULT,
    };

    enum Status
    {
        E_CHG_STS_NO_VALID_SOURCE   = 0,
        E_CHG_STS_IN_READY          = 1,
        E_CHG_STS_USB_READY         = 2,
        E_CHG_STS_CHG_FROM_IN       = 3,
        E_CHG_STS_CHG_FROM_USB      = 4,
        E_CHG_STS_CHG_DONE          = 5,
        E_CHG_STS_FAULT             = 7,
    };

    bool isInitializing;
    bool hasError;
    bool isGaugeWaitingForCommand;
    FaultType faultType;
    Status chargerStatus;
};

class Charger
{
public:
    Charger(std::shared_ptr<I2CMux> muxPtr);

    int Open(int i2cFd);
    int MeasureSync(uint16_t &socPercent, uint16_t &voltage_mV, ChargerStatus &status);
    void Close();

    ~Charger();

protected:
    std::shared_ptr<I2CMux> m_mux;
    int m_i2cFd;
    bool m_isOpen;

    int ReadReg(ChgRegAddr address, uint16_t &value);
    int WriteReg(ChgRegAddr address, uint16_t value);
    int WriteCtrlCmd(ChgCmdId subcmd);
    int ReadCtrlCmd(ChgCmdId subcmd, uint16_t &value, uint32_t delayUs = 100u);
    int WriteExtendedCmd(ChgRegAddrExt extCmd, uint8_t *data, uint8_t len);
    int ReadExtendedCmd(ChgRegAddrExt extCmd, uint8_t *data, uint8_t len);
    int Unseal();
    int Seal();
    int ReadDataFlash(ChgDataFlashSubclass subcls, ChgDataFlashBlock block, ChgRegAddrExt off, uint8_t *readData, uint8_t len);
    int WriteDataFlash(ChgDataFlashSubclass subcls, ChgDataFlashBlock block, ChgRegAddrExt off, uint8_t *writeData, uint8_t len);
    int UpdateBattCapacity();
    int SetChgInHostMode();
    int SetSleepI();
    int SetHibernateI();
    int Reset();
    int VoltageRead(uint16_t &v_in_mv);
    int TrueSOCRead(uint16_t &true_soc);
    int StatusChargerRead(ChargerStatus &status);
};

#endif // CHARGER_H
