/*
 * psy_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef PSY_DRV_H_
#define PSY_DRV_H_


#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <memory>

#include <libdigiapix/gpio.h>

#include "i2c_drv.h"


#define BATTERY_PACK_TEMPERATURE_CMD 		0x06
#define BATTERY_PACK_VOLTAGE_CMD 			0x08
#define BATTERY_INSTANT_CURRENT_CMD 		0x0C
#define BATTERY_REMAINING_CAPACITY_CMD 		0x10
#define BATTERY_FULL_CHARGE_CAPACITY_CMD 	0x12
#define BATTERY_RELATIVE_STATE_OF_CHG_CMD 	0x2C

#define MAC_00_ACCES 						0x00
#define MAC_3E_ACCES 						0x3E

#define BQ28Z610_SHUTDOWN 					0x0010
#define BQ28Z610_FET_EN 					0x0022
#define BQ28Z610_RESET 						0x0041
#define BQ28Z610_SAFETY_ALERT 				0x0051


#define BQ28Z610_UNSEAL_KEY_MSW 			0x0414
#define BQ28Z610_UNSEAL_KEY_LSW 			0x3672

#define BQ28Z610_FA_KEY_MSW 				0xFFFF
#define BQ28Z610_FA_KEY_LSW 				0xFFFF


#define MCA_MAIN_BUTTON_ALIAS			"MCA_MAIN_BTN"
#define MCA_ACCESSORY_BUTTON_ALIAS		"MCA_ACCESSORY_BTN"

#define MCA_3V3_EXT_EN_ALIAS			"MCA_3V3_EXT_EN"

#define MCA_AS_SINK_ALIAS				"MCA_AS_SINK"
#define MCA_PLUG_EVENT_ALIAS			"MCA_PLUG_EVENT"
#define MCA_CRTDG_DET_ALIAS				"MCA_CRTDG_DET"


class Bq28z610 : public I2cDev
{

};

class Sm24lc258 : public I2cDev
{

};


struct gpio_interrupt_cb_data {
	gpio_t				*gpio;
	gpio_value_t 		prev_gpio_value;
	struct timespec 	bebounce_time_spec;	// Debounce timer
};


// Power System controller class
class PsyControler
{
public:
	/** Name of the PCA9624 registers (for direct register access) */
	enum Sys_ctrl{
		SYS_PWR_CTRL,      /**< Master system MOSFET control      */
		EXT_3V3_CTRL,      /**< External peri register      */
	};

	enum UsbPlug_state{
		USB_DISCONNECTED,      	/**< USB is not connected  */
		PWR_PLUG_CONNECTED,     /**< A power plug is connected to the reader  */
		PERIPHERAL_CONNECTED, 	/**< A peripheral is connected and drawing current from the reader   */
		USB_PLUG_STATE_ERROR
	};

	enum CrtdgPlug_state{
		CRTDG_DRAWER_LOCKED,      	/**< Drawer is correctly inserted   */
		CRTDG_DRAWER_INVALID,      	/**< Drawer is not correctly inserted   */
		CRTDG_PLUG_STATE_ERROR
	};

	/** Name of the PCA9624 registers (for direct register access) */
	gpio_t *gpio_sys_pwr; 	// GPIO controlling system MOSFET
	gpio_t *gpio_ext_3v3;	// GPIO controlling supply of external 3V3

	gpio_t *gpio_pwr_main_button_det;		// Main button event detection gpio
	gpio_t *gpio_pwr_accessory_button_det;	// Accessory button event detection gpio

	gpio_t *gpio_pwr_crtdg_event_det;	// Cartridge inserted event detection gpio
	gpio_t *gpio_pwr_plug_evnt_det;		// Power plug event detection gpio
	gpio_t *gpio_pwr_isasink_evnt_det;	// Power is a sink event detection gpio

	struct gpio_interrupt_cb_data cb_data;

	Bq28z610 	bq28z610;	// I2C bus used for battery guage
	Sm24lc258 	sm24LC256;	// I2C bus used for power pcb memory

	int Init(const char *gpio_ctrl_alias, uint8_t i2c_bus_id);
	int PsySet(Sys_ctrl module, bool state);

	UsbPlug_state 	USBConnectionState(void);
	void 			USBControllerReset(void);
	CrtdgPlug_state CartridgeDetectionState(void);

	static void cleanup(void);

	int BatteryLevel(uint16_t *batteryLevel);
	int BatteryPackTemperature(uint16_t *batTemperature);
	int BatteryPackVoltage(uint16_t *packVoltage);
	int BatteryInstantCurrent(uint16_t *instantCurrent);
	int BatteryRemainingCapacity(uint16_t *remainingCapacity);
	int BatteryFullChargeCapacity(uint16_t *fullChgCapacity);
	int BatteryRelativeStateOfCharge(uint16_t *relativeStateOfCharge);
	int BatteryEraseFirwmare();
	int BatteryFirmwareUpdate(char* fwImage, uint32_t count);

	char *ChargeCtrlExecuteFs(char* goldenImg, int count);
	int AltManufacturerAccess(uint8_t cmd, int instCode);
	int FuelGaugeReset();
	int FuelGaugeFetEnable();
	char *FuelGaugeExecuteFs(char* goldenImg, int count);

	int FuelGaugeUnseal();
	int FuelGaugeFullAccess();
	int FuelGaugeShutdown();
};

#endif /* PSY_DRV_H_ */
