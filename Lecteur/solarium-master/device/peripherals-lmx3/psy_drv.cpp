/*
 * udc_drv.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "i2c_drv.h"
#include "inc/psy_drv.h"


/*
 * gpio_interrupt_cb() - GPIO callback for interrupts
 *
 * @arg:	GPIO interrupt data (struct gpio_interrupt_cb_data).
 */
static int gpio_interrupt_cb(void *arg)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;
	struct gpio_interrupt_cb_data *data = (struct gpio_interrupt_cb_data *)arg;
	gpio_value_t 	current_gpio_value;

	printf("Input GPIO interrupt detected; processing debounce \n");

    clock_gettime(CLOCK_REALTIME, &spec);
    current_gpio_value = ldx_gpio_get_value(data->gpio);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

	/* If first hit, initialize time spec for debounce.  */
    if(0 == data->bebounce_time_spec.tv_sec){
    	data->bebounce_time_spec = spec;
    }else if(current_gpio_value == GPIO_HIGH){
    	// Process only if a valid debounce timer is running
    	if(0 != data->bebounce_time_spec.tv_sec){
        	/* Determine the type of button press */
        	if(spec.tv_sec-data->bebounce_time_spec.tv_sec >= 3){
        		printf("Detected long press -> going to deep sleep");
        		usleep(10000);
        		// Send shutdown signal to device
        		system("gpiomon --num-events 1 --falling-edge mca-gpio 1 &");
        		// system("gpiomon --num-events 1 --falling-edge mca-gpio 2 &");
        		system("poweroff -f");

        	}else if(spec.tv_sec-data->bebounce_time_spec.tv_sec >= 1){
        		printf("Detected mid press -> signal action \n");

        		// So something here
        	}
        	else
        		printf("Press is degliched, no action will be taken \n");
    	}
    }else if(current_gpio_value == GPIO_LOW){
    	/* Determine the type of button press */
    	data->bebounce_time_spec = spec;
    	printf("A new detection started \n");
    }

    // Store new state of button
    data->prev_gpio_value = current_gpio_value;

	return 0;
}

/*
 * sigaction_handler() - Handler to execute after receiving a signal
 *
 * @signum:	Received signal.
 */
static void sigaction_handler(int signum)
{
	/* 'atexit' executes the cleanup function */
	exit(EXIT_FAILURE);
}

/*
 * register_signals() - Registers program signals
 */
static void register_signals(void)
{
	struct sigaction action;

	action.sa_handler = sigaction_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);

	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}


int PsyControler::Init(const char *gpio_ctrl_alias, uint8_t i2c_bus_id)
{
	static char main_btn_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	static char accessory_btn_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	static char ext_3V3_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	static char plug_evt_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	static char as_sink_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	static char crtdg_ctrl[MAX_CONTROLLER_LEN] = { 0 };
	int main_btn_line, accessory_btn_line, ext_3v3_line, plug_evt_line, as_sink_line, crtdg_line;


	// Retrieve control over main sys power MOSFET
	ldx_gpio_get_controller(MCA_3V3_EXT_EN_ALIAS, ext_3V3_ctrl);
	ext_3v3_line = ldx_gpio_get_line(MCA_3V3_EXT_EN_ALIAS);

	gpio_ext_3v3 = ldx_gpio_request_by_controller(ext_3V3_ctrl, ext_3v3_line, GPIO_OUTPUT_LOW);

	ldx_gpio_set_value(gpio_ext_3v3, GPIO_LOW);

	printf("External 3V3 enabled \n");

	// Retrieve control over main button GPIO
	ldx_gpio_get_controller(MCA_MAIN_BUTTON_ALIAS, main_btn_ctrl);
	main_btn_line = ldx_gpio_get_line(MCA_MAIN_BUTTON_ALIAS);

	// Retrieve control over accessory button
	ldx_gpio_get_controller(MCA_ACCESSORY_BUTTON_ALIAS, accessory_btn_ctrl);
	accessory_btn_line = ldx_gpio_get_line(MCA_ACCESSORY_BUTTON_ALIAS);

	// Retrieve control over main sys power MOSFET
	ldx_gpio_get_controller(MCA_CRTDG_DET_ALIAS, crtdg_ctrl);
	crtdg_line = ldx_gpio_get_line(MCA_CRTDG_DET_ALIAS);

	// Retrieve control over MCA plug event try _gpio_ctrl_alias
	ldx_gpio_get_controller(MCA_PLUG_EVENT_ALIAS, plug_evt_ctrl);
	plug_evt_line = ldx_gpio_get_line(MCA_PLUG_EVENT_ALIAS);

	// Retrieve control over main sys power MOSFET
	ldx_gpio_get_controller(MCA_AS_SINK_ALIAS, as_sink_ctrl);
	as_sink_line = ldx_gpio_get_line(MCA_AS_SINK_ALIAS);

	/* Request access to main and accessory buttons */
	gpio_pwr_main_button_det = ldx_gpio_request_by_controller(main_btn_ctrl, main_btn_line,
			GPIO_IRQ_EDGE_BOTH);

	/* Initialize data to be passed to the interrupt handler */
	cb_data.gpio = gpio_pwr_main_button_det;
	cb_data.prev_gpio_value = ldx_gpio_get_value(gpio_pwr_main_button_det);
	cb_data.bebounce_time_spec.tv_sec = 0; // Initialize to 0 (no current detection), as we shall have no debouncer at startup

	register_signals();

	if (ldx_gpio_start_wait_interrupt(gpio_pwr_main_button_det, &gpio_interrupt_cb, &cb_data)
	    != EXIT_SUCCESS) {
		printf("Failed to start interrupt handler thread\n");
		return EXIT_FAILURE;
	}

	gpio_pwr_accessory_button_det = ldx_gpio_request_by_controller(accessory_btn_ctrl, accessory_btn_line, GPIO_INPUT);

	/* Request input GPIO */
	gpio_pwr_crtdg_event_det = ldx_gpio_request_by_controller(crtdg_ctrl, crtdg_line, GPIO_INPUT);

	gpio_pwr_plug_evnt_det = ldx_gpio_request_by_controller(plug_evt_ctrl, plug_evt_line, GPIO_INPUT);

	gpio_pwr_isasink_evnt_det = ldx_gpio_request_by_controller(as_sink_ctrl, as_sink_line, GPIO_INPUT);

	// Set addresses for peripherals
	bq28z610.Init(i2c_bus_id, 0x55);

	// Configure memory
	sm24LC256.Init(i2c_bus_id, 0x50);

	return EXIT_SUCCESS;
}


PsyControler::UsbPlug_state PsyControler::USBConnectionState(void){
	gpio_value_t plug_evt_line_value, isa_sink_line_value;

	plug_evt_line_value = ldx_gpio_get_value(gpio_pwr_plug_evnt_det);
	isa_sink_line_value = ldx_gpio_get_value(gpio_pwr_isasink_evnt_det);

	if((GPIO_VALUE_ERROR == plug_evt_line_value)||(GPIO_VALUE_ERROR == isa_sink_line_value)){
		printf("Please verify device tree as it appear to be a conflict \n");
		return PsyControler::USB_PLUG_STATE_ERROR;
	}

	if((plug_evt_line_value == GPIO_HIGH) && (isa_sink_line_value == GPIO_HIGH)){
		printf("USB is connected to a power source \n");
		return PsyControler::PWR_PLUG_CONNECTED;
	}else if((plug_evt_line_value == GPIO_HIGH) && (isa_sink_line_value == GPIO_LOW)){
		printf("USB is connected to a USB drive or another peripheral acting as a sink \n");
		return PsyControler::PERIPHERAL_CONNECTED;
	}else{
		printf("USB is disconnected \n");
		return PsyControler::USB_DISCONNECTED;
	}
}


void PsyControler::USBControllerReset(void){
	PsySet(EXT_3V3_CTRL, false);

	usleep(250000);

	PsySet(EXT_3V3_CTRL, true);

	printf("External 3V3 enabled \n");
}

PsyControler::CrtdgPlug_state PsyControler::CartridgeDetectionState(void){
	gpio_value_t cartridge_line_value;

	if(gpio_pwr_crtdg_event_det==NULL){
		printf("Please verify device tree as it appear to be a conflict \n");
		return PsyControler::CRTDG_PLUG_STATE_ERROR;
	}

	cartridge_line_value = ldx_gpio_get_value(gpio_pwr_crtdg_event_det);

	if(GPIO_VALUE_ERROR == cartridge_line_value)
		return PsyControler::CRTDG_PLUG_STATE_ERROR;

	if(cartridge_line_value == GPIO_HIGH){
		printf("Cartridge drawer is absent or incorrectly inserted \n");
		return PsyControler::CRTDG_DRAWER_INVALID;
	}else{
		printf("Cartridge drawer is locked and ready for reading \n");
		return PsyControler::CRTDG_DRAWER_LOCKED;
	}
}

int PsyControler::PsySet(Sys_ctrl module, bool state)
{
	if(SYS_PWR_CTRL==module){
		ldx_gpio_set_value(gpio_sys_pwr, state ? GPIO_HIGH : GPIO_LOW);
		return EXIT_SUCCESS;
	}else if(EXT_3V3_CTRL==module){
		ldx_gpio_set_value(gpio_ext_3v3, state ? GPIO_LOW : GPIO_HIGH);
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}


int PsyControler::BatteryPackTemperature(uint16_t *batteryTemperature)
{
	uint8_t regValue[2];
	float tempInC;

	if(bq28z610.ReadRegisters(BATTERY_PACK_TEMPERATURE_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*batteryTemperature = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*batteryTemperature = (regValue[1] << 8) | regValue[0];
	tempInC = *batteryTemperature*0.1-273.15;
	printf("Battery pack temperature = %f deg C \n", tempInC);

	return EXIT_SUCCESS;
}


int PsyControler::BatteryPackVoltage(uint16_t *packVoltage)
{
	uint8_t regValue[2];

	if(bq28z610.ReadRegisters(BATTERY_PACK_VOLTAGE_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*packVoltage = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*packVoltage = (regValue[1] << 8) | regValue[0];
	printf("Battery pack voltage = %i mV \n", *packVoltage);

	return EXIT_SUCCESS;
}


int PsyControler::BatteryInstantCurrent(uint16_t *instantCurrent)
{
	uint8_t regValue[2];

	if (bq28z610.ReadRegisters(BATTERY_INSTANT_CURRENT_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*instantCurrent = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*instantCurrent = (regValue[1] << 8) | regValue[0];
	if(*instantCurrent&0x8000)
	{
		*instantCurrent = ~*instantCurrent + 1;
		printf("Battery is discharging, instant current = %i mA \n", *instantCurrent);
	}else{
		printf("Battery is charging, instant current = %i mA \n", *instantCurrent);
	}

	return EXIT_SUCCESS;
}


int PsyControler::BatteryRemainingCapacity(uint16_t *remainingCapacity)
{
	uint8_t regValue[2];

	if (bq28z610.ReadRegisters(BATTERY_REMAINING_CAPACITY_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*remainingCapacity = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*remainingCapacity = (regValue[1] << 8) | regValue[0];
	printf("Battery pack remaining capacity = %i mAH \n", *remainingCapacity);

	return EXIT_SUCCESS;
}


int PsyControler::BatteryFullChargeCapacity(uint16_t *fullChgCapacity)
{
	uint8_t regValue[2];

	if (bq28z610.ReadRegisters(BATTERY_FULL_CHARGE_CAPACITY_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*fullChgCapacity = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*fullChgCapacity = (regValue[1] << 8) | regValue[0];
	printf("Battery pack full charge capacity = %i mA \n", *fullChgCapacity);

	return EXIT_SUCCESS;
}

int PsyControler::BatteryRelativeStateOfCharge(uint16_t *relativeStateOfCharge)
{
	uint8_t regValue[2];

	if (bq28z610.ReadRegisters(BATTERY_RELATIVE_STATE_OF_CHG_CMD, regValue, 2) != EXIT_SUCCESS) {
		printf("Error: Register access failed.\n");
		*relativeStateOfCharge = 0xFFFF;
		return EXIT_FAILURE;
	}

	// Retrieve battery level
	*relativeStateOfCharge = (regValue[1] << 8) | regValue[0];
	printf("Battery pack relative state of charge = %i %% \n", *relativeStateOfCharge);

	return EXIT_SUCCESS;
}

int PsyControler::AltManufacturerAccess(uint8_t cmd, int instCode){
	uint8_t data[3];

	data[0] = cmd;
	data[1] = instCode & 0xFF;
	data[2] = (instCode>>8) & 0xFF;

	if(bq28z610.DirectWrite(data, (uint16_t)(3)) != EXIT_SUCCESS) {
		printf("Error: AltManufacturerAccess instruction failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int PsyControler::FuelGaugeReset()
{

	if(AltManufacturerAccess(MAC_3E_ACCES, BQ28Z610_RESET) != EXIT_SUCCESS) {
		printf("Error: Failed to reset bq28z610 failed.\n");
		return EXIT_FAILURE;
	}

	printf("Fuel gauge reset successful.\n");
	return EXIT_SUCCESS;
}

int PsyControler::FuelGaugeUnseal()
{
	if(AltManufacturerAccess(MAC_00_ACCES, BQ28Z610_UNSEAL_KEY_MSW) != EXIT_SUCCESS) {
		printf("Error: Failed to unseal bq28z610 failed.\n");
		return EXIT_FAILURE;
	}

	if(AltManufacturerAccess(MAC_00_ACCES, BQ28Z610_UNSEAL_KEY_LSW) != EXIT_SUCCESS) {
		printf("Error: Failed to unseal bq28z610 failed.\n");
		return EXIT_FAILURE;
	}

	printf("Fuel gauge successfully UNSEALED.\n");
	return EXIT_SUCCESS;
}

int PsyControler::FuelGaugeFullAccess()
{
	if(AltManufacturerAccess(MAC_00_ACCES, BQ28Z610_FA_KEY_MSW) != EXIT_SUCCESS) {
		printf("Error: Failed to gain full access bq28z610 failed.\n");
		return EXIT_FAILURE;
	}

	if(AltManufacturerAccess(MAC_00_ACCES, BQ28Z610_FA_KEY_LSW) != EXIT_SUCCESS) {
		printf("Error: Failed to gain full access bq28z610 failed.\n");
		return EXIT_FAILURE;
	}

	printf("Fuel gauge full access successful.\n");
	return EXIT_SUCCESS;
}


int PsyControler::FuelGaugeShutdown(){

	if(AltManufacturerAccess(MAC_3E_ACCES, BQ28Z610_SHUTDOWN) != EXIT_SUCCESS) {
		printf("Error: Failed to shutdown Gauge.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int PsyControler::FuelGaugeFetEnable(){

	if(AltManufacturerAccess(MAC_3E_ACCES, BQ28Z610_FET_EN) != EXIT_SUCCESS) {
		printf("Error: Failed to enable FET failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

char *PsyControler::FuelGaugeExecuteFs(char* goldenImg, int count){
	int nDataLength;
	char pBuf[16];
	char pData[32];
	int n, m;
	char *pEnd = NULL;
	char *pErr;
	bool bWriteCmd = false;
	unsigned char nRegister;

	m = 0;
	for (n = 0; n < count; n++){
		if (goldenImg[n] != ' ') goldenImg[m++] = goldenImg[n];
	}
	pEnd = goldenImg + m;
	pEnd[0] = 0;

	do
	{
		switch (*goldenImg)
		{
		case ';':
			break;
		case 'W':
		case 'C':
			bWriteCmd = *goldenImg == 'W';
			goldenImg++;
			if ((*goldenImg) != ':')
				return goldenImg;
			goldenImg++;
			n = 0;
			while ((pEnd - goldenImg > 2) && (n < sizeof(pData) + 2) &&(*goldenImg != '\n'))
			{
				pBuf[0] = *(goldenImg++);
				pBuf[1] = *(goldenImg++);
				pBuf[2] = 0;
				m = strtoul(pBuf, &pErr, 16);
				if (*pErr) return (goldenImg);
				if (n == 0)
					bq28z610.PointRegister(m);
				if (n == 1)
					nRegister = m;
				if (n > 1) pData[n - 2] = m;
				n++;
			}
			if (n < 3)
				return goldenImg;
			nDataLength = n - 2;
			if (bWriteCmd)
				bq28z610.WriteRegisters(nRegister, (uint8_t*)pData, nDataLength);
			else
			{
				uint8_t pDataFromGauge[nDataLength];
				bq28z610.ReadRegisters(nRegister, pDataFromGauge, nDataLength);
				if (memcmp(pData, pDataFromGauge, nDataLength))
					return goldenImg;
			}
			break;

		case 'X':
			goldenImg++;
			if ((*goldenImg) != ':')
				return goldenImg;
			goldenImg++;
			n = 0;
			while ((goldenImg != pEnd) && (*goldenImg != '\n') &&(n <sizeof(pBuf) - 1))
			{
				pBuf[n++] = *goldenImg;
				goldenImg++;
			}
			pBuf[n] = 0;
			n = atoi(pBuf);
			usleep(n * 1000);
			break;
		default:
			return goldenImg;
		}
		while ((goldenImg != pEnd) && (*goldenImg != '\n'))
			goldenImg++; //skip to next line
		if (goldenImg != pEnd)
			goldenImg++;
	}while (goldenImg != pEnd);

	return goldenImg;
}

int PsyControler::BatteryEraseFirwmare()
{
	uint16_t blockAddress;
	uint8_t *data_block;

	// Write every blocks of firmware data into
	data_block = (uint8_t *)calloc(2 + 64, sizeof(uint8_t));
	if (data_block == NULL) {
		printf("Error: allocating page memory\n");
		return EXIT_FAILURE;
	}

	for(int i=0; i<64; i++){
		data_block[2+i] = 0xFF;
	}

	for(int k=0; k<496; k++){
		blockAddress = k * 64;
		data_block[0] = 0xFF & blockAddress>>8;
		data_block[1] = 0xFF & blockAddress;

		if(sm24LC256.DirectWrite(data_block, (uint16_t)(66)) != EXIT_SUCCESS) {
			printf("Error: Firmware data written failed.\n");
			free(data_block);
			return EXIT_FAILURE;
		}
		printf("Block ID%i erased.\n", k);
		usleep(15000);
	}
	// Free allocations
	free(data_block);

	return EXIT_SUCCESS;
}

int PsyControler::BatteryFirmwareUpdate(char* fwImage, uint32_t count)
{
	uint16_t blockAddress;
	uint8_t *data_block, rmngBytes;
	uint8_t read_back[66];

	// Write every blocks of firmware data into
	data_block = (uint8_t *)calloc(2 + 64, sizeof(uint8_t));
	if (data_block == NULL) {
		printf("Error: allocating page memory\n");
		return EXIT_FAILURE;
	}

	for(uint32_t k=0; k<count/64; k++){
		blockAddress = k * 64;
		data_block[0] = 0xFF & blockAddress>>8;
		data_block[1] = 0xFF & blockAddress;
		read_back[0]  = data_block[0];
		read_back[1]  = data_block[1];

		rmngBytes = count - blockAddress >= 64 ? 64 : count - blockAddress;
		for (int i=0; i<rmngBytes; i++){
			data_block[2+i] = fwImage[blockAddress+i];
		}
		// Add some trailing bytes to the end
		if(rmngBytes!=64){
			for (int i=rmngBytes; i<64; i++){
				data_block[2+i] = 0x00;
			}
		}

		if(sm24LC256.DirectWrite(data_block, (uint16_t)(66)) != EXIT_SUCCESS) {
			printf("Error: Firmware data written failed.\n");
			free(data_block);
			return EXIT_FAILURE;
		}
		usleep(16000);

		if(sm24LC256.DirectWrite(data_block, (uint16_t)(2)) != EXIT_SUCCESS) {
			printf("Error: Failed to point to register address.\n");
			free(data_block);
			return EXIT_FAILURE;
		}
		usleep(5000);
		// Read back to verify programmed data
		if(sm24LC256.DirectRead(read_back, (uint16_t)(64)) != EXIT_SUCCESS) {
			printf("Error: Firmware data readback failed.\n");
			free(data_block);
			return EXIT_FAILURE;
		}

		for(int i=0; i<64; i++){
			if(read_back[i] != data_block[2+i]){
				printf("Error: Firmware data readback is inconsistent with programming buffer.\n");
				free(data_block);
				return EXIT_FAILURE;
			}
		}
		printf("Block ID%i programming.\n", k);
		usleep(3000);
	}

	// Free allocations
	free(data_block);

	return EXIT_SUCCESS;
}

