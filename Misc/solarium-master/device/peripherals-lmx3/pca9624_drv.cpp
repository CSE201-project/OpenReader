/*
 * pca9624_drv.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include "inc/pca9624_drv.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define DEFAULT_I2C_ADDRESS			0x54

int Pca9624::GpioInit(const char *gpio_alias)
{
	int i=0, pca_en_line, i2c_bus_nb = 0;
	static char pca_en_ctrl[MAX_CONTROLLER_LEN] = { 0 };

	// Get access to ON/OFF button
	ldx_gpio_get_controller(gpio_alias, pca_en_ctrl);
	pca_en_line = ldx_gpio_get_line(gpio_alias);

	if (pca_en_ctrl == NULL || pca_en_line < 0) {
		//printf("Unable to parse PCA enable GPIO\n");
		return EXIT_FAILURE;
	}


	/* Set PCA9624 to disable at startup */
	en_gpio = ldx_gpio_request_by_controller(pca_en_ctrl, pca_en_line, GPIO_OUTPUT_LOW);
	if (!en_gpio) {
		//printf("Failed to initialize input GPIO\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Pca9624::Enable(void)
{
	// Set GPIO to low state
	ldx_gpio_set_value(en_gpio, GPIO_LOW);
	return EXIT_SUCCESS;
}

int Pca9624::Disable(void)
{
	// Set GPIO to high state
	ldx_gpio_set_value(en_gpio, GPIO_HIGH);
	return EXIT_SUCCESS;
}


int Led_driver::Init(const char *ctrl_gpio_alias, uint8_t i2c_bus_id, uint8_t _i2c_address){
	uint8_t i=0, regValue[2];

	pcaDriver.GpioInit(ctrl_gpio_alias);

	pcaDriver.Init(i2c_bus_id, _i2c_address);

	// As a security always start pcaDriver in OFF state
	pcaDriver.Disable();

	// Init all led to OFF state
	for(i=0; i<LED_CHAN_COUNT; i++)
		led_state[i] = LED_IND_PWM;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_pwm[i] = ledMaximumBrightness;

	regValue[0] = 0x00;
	if(pcaDriver.WriteRegisters(Pca9624::MODE1, regValue, 1) != EXIT_SUCCESS){
		return EXIT_FAILURE;
	}

	regValue[0] = 0x0D;
	if(pcaDriver.WriteRegisters(Pca9624::MODE2, &regValue[0], 1) != EXIT_SUCCESS){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int Led_driver::Enable(void)
{
	pcaDriver.Enable();
	return EXIT_SUCCESS;
}

int Led_driver::Disable(void)
{
	pcaDriver.Disable();
	return EXIT_SUCCESS;
}


int Led_driver::SetState(Led_driver::Led_channel ledIdx, Led_driver::Led_brightness state){
	if(ledIdx>=0 && ledIdx<LED_CHAN_COUNT)
	{
		led_state[ledIdx] = state;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


int Led_driver::SetIntensity(Led_driver::Led_channel ledIdx, uint8_t pwm){
	if(ledIdx>=0 && ledIdx<LED_CHAN_COUNT)
	{
		led_pwm[ledIdx] = (pwm < ledMaximumBrightness ? pwm : ledMaximumBrightness); // Av
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}


int Led_driver::Refresh(void){
	uint8_t regValue[2];

	// Update led states
	regValue[0] = (led_state[3] << 6) | (led_state[2] << 4) | (led_state[1] << 2) | (led_state[0]);
	regValue[1] = (led_state[7] << 6) | (led_state[6] << 4) | (led_state[5] << 2) | (led_state[4]);

	if(pcaDriver.WriteRegisters(0x80 |Pca9624::LEDOUT0, regValue, 2) != EXIT_SUCCESS){
		return EXIT_FAILURE;
	}
	//printf("LEDOUT updated 2.\n");

	// Then update led PWM values
	if(pcaDriver.WriteRegisters(0xA0 | Pca9624::PWM0, &led_pwm[0], 8) != EXIT_SUCCESS){
		return EXIT_FAILURE;
	}
	//printf("PWMs updated new.\n");

	// Force read cartridge GPIO

	return EXIT_SUCCESS;
}

int Led_driver::Blink(uint32_t usec){
	uint8_t i;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_state[i] = LED_IND_PWM;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_pwm[i] = ledMaximumBrightness;

	Refresh();
	usleep(usec);

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_state[i] = LED_OFF;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_pwm[i] = 0x00;

	Refresh();

	//printf("Blinked.\n");

	// Force read cartridge GPIO

	return EXIT_SUCCESS;
}

int Led_driver::FullBrightness(void){
	uint8_t i;


	for(i=0; i<LED_CHAN_COUNT; i++)
		led_state[i] = LED_IND_PWM;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_pwm[i] = ledMaximumBrightness;

	Refresh();

	return EXIT_SUCCESS;
}

int Led_driver::Off(void){
	uint8_t i;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_state[i] = LED_OFF;

	for(i=0; i<LED_CHAN_COUNT; i++)
		led_pwm[i] = 0x00;

	Refresh();

	return EXIT_SUCCESS;
}
