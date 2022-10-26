//============================================================================
// Name        : Lumedix-drivers-Test.cpp
// Author      : JC
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <unistd.h>

#include <libdigiapix/i2c.h>

#include "i2c_drv.h"
#include "psy_drv.h"
#include "lp5562_drv.h"
#include "pca9624_drv.h"
#include "thm_drv.h"
#include "uvs_drv.h"


#define IMX8X_I2C0_BUS  		0
#define IMX8X_I2C3_BUS  		3
#define IMX8X_MIPI_I2C0_BUS  	17

#define LEDS_CTRL_I2C_BUS "LEDS_I2C_BUS"

#define VIS_PCA9624_CTRL_GPIO "VIS_CTRL_GPIO"
#define UV_PCA9624_CTRL_GPIO "UV_CTRL_GPIO"


#define DEFAULT_I2C_BUS		"DEFAULT_I2C_BUS"
#define SYS_POWER_CTRL_GPIO "LEDS_I2C_BUS"
#define LEDS_CTRL_I2C_BUS 	"LEDS_I2C_BUS"

#define I2C3_I2C_BUS 		17

#define FIRMWARE_FILENAME 		"PsysFirmware-v3.1.bin"

using namespace std;


int LoadFirmwareBinaries(PsyControler *ctrler, char *fwImage)
{
	//write object into the file
	fstream file;
	std::streampos fwImageLength;

	printf("Opening firmware image %s \n", FIRMWARE_FILENAME);
	file.open(FIRMWARE_FILENAME,ios::in|ios::binary);
	if(!file){
		cout<<"Could not find or open firmware update file...\n";
		return -1;
	}

	// Get firmware file size
	fwImageLength = file.tellg();
    file.seekg( 0, std::ios::end );
    fwImageLength = file.tellg() - fwImageLength;

	printf("Firmware payload is %i bytes\n", (int)fwImageLength);

    // Allocate space for fw image
    fwImage = (char *)calloc(fwImageLength, sizeof(uint8_t));
	if (fwImage == NULL) {
		printf("Error: allocating firmware image memory\n");
		return EXIT_FAILURE;
	}
    // Then provide file stream to
    file.seekg( 0, std::ios::beg );
	if(file.read(fwImage, fwImageLength)){
			cout<<endl<<endl;
			cout<<"Data extracted from file..\n";
	}
	else{
		cout<<"Error in reading data from file...\n";
		free(fwImage);
		return -1;
	}

	// First erase external memory
	cout<<"Erasing flash...\n";
	ctrler->BatteryEraseFirwmare();

	// Start firmware update procedure
	cout<<"Programming flash...\n";
	ctrler->BatteryFirmwareUpdate(fwImage, (uint32_t)fwImageLength);

	free(fwImage);

	file.close();
	return 0;
}

int main() {
	int i=0;
	uint16_t uin16Value;
	PsyControler powerSys;

	powerSys.Init("SYS_PWR_GPIO", IMX8X_MIPI_I2C0_BUS);

	if(false){
		LoadFirmwareBinaries(&powerSys, FIRMWARE_FILENAME);
	}

	powerSys.BatteryPackTemperature(&uin16Value);

	powerSys.BatteryPackVoltage(&uin16Value);

	powerSys.BatteryInstantCurrent(&uin16Value);

	powerSys.BatteryRemainingCapacity(&uin16Value);

	powerSys.BatteryFullChargeCapacity(&uin16Value);

	powerSys.BatteryRelativeStateOfCharge(&uin16Value);

	powerSys.CartridgeDetectionState();

	powerSys.USBConnectionState();

	cout << "Starting Lumedix-driver tests" << endl;
	cout << "  " << endl;

	if(true){
		cout << "Testing User Led interface drivers" << endl;
		IhmController rgbLedCtlr, dotLedCtlr;

		rgbLedCtlr.lpDriver.Init(IMX8X_MIPI_I2C0_BUS, 0x30);
		if(false){
			rgbLedCtlr.lpDriver.SetLed(Lp5562::B_LED, 0x20);
			rgbLedCtlr.lpDriver.SetLed(Lp5562::G_LED, 0x20);
			rgbLedCtlr.lpDriver.SetLed(Lp5562::R_LED, 0x20);
			rgbLedCtlr.lpDriver.SetLed(Lp5562::W_LED, 0x20);
			rgbLedCtlr.lpDriver.LoadIntensities();
			rgbLedCtlr.SetMode(IhmController::BT_SEARCH);
		}else{
			rgbLedCtlr.SetMode(IhmController::BT_SEARCH);
		}

		dotLedCtlr.lpDriver.Init(IMX8X_MIPI_I2C0_BUS, 0x32);
		if(true){
			dotLedCtlr.lpDriver.SetLed(Lp5562::B_LED, 0x20);
			dotLedCtlr.lpDriver.SetLed(Lp5562::G_LED, 0x20);
			dotLedCtlr.lpDriver.SetLed(Lp5562::R_LED, 0x20);
			dotLedCtlr.lpDriver.SetLed(Lp5562::W_LED, 0x20);
			dotLedCtlr.lpDriver.LoadIntensities();
			dotLedCtlr.SetMode(IhmController::BT_SEARCH);
		}else{
			dotLedCtlr.SetMode(IhmController::BT_SEARCH);
		}

		cout << "End of User Led interface driver Tests" << endl;
	}


	cout << "Testing UV Photo-diode drivers" << endl;
	Uvs uvSensor;
	uvSensor.Init(IMX8X_MIPI_I2C0_BUS, 0x39);
	//uvSensor.ReadChipId();

	// Then configure chip for UVB acquisition
	uvSensor.SetResolution(Uvs::RES_UV_100MS);
	uvSensor.SetRange(Uvs::RANGE_UV_8x);
	uvSensor.ConfigSensor();

	// Proceed to several readings at specific frequence
	for(i=0; i<3; i++){
		usleep(250000);
		uvSensor.ReadUVB();
	}

	cout << "End of UV Photo-diode driver Tests" << endl;

	if(true){
		cout << "Testing Led drivers Test" << endl;

		Led_driver visLedDriver, uvLedDriver;
		if(visLedDriver.Init("UV_CTRL_GPIO", IMX8X_I2C3_BUS, 0x3A)!=0) // UV_CTRL_GPIO
			cout << "PCA driver for visible control initialization failed" << endl;

		if(uvLedDriver.Init("VIS_CTRL_GPIO", IMX8X_I2C3_BUS, 0x38)!=0) // VIS_CTRL_GPIO
			cout << "PCA driver for visible control initialization failed" << endl;

		cout << "PCA driver initialized" << endl;

		// First enable visible leds and test
		visLedDriver.Enable();
		visLedDriver.Off();

		uvLedDriver.Enable();

		visLedDriver.Off();

		if(false){
			if(uvLedDriver.FullBrightness())
				cout << "Led driver refresh failed" << endl;
			else
				cout << "Led driver refreshed" << endl;
		}else{
			uvLedDriver.SetState(Led_driver::LED_CHAN0, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN1, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN2, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN3, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN4, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN5, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN6, Led_driver::LED_IND_PWM);
			uvLedDriver.SetState(Led_driver::LED_CHAN7, Led_driver::LED_IND_PWM);

			uvLedDriver.SetIntensity(Led_driver::LED_CHAN0, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN1, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN2, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN3, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN4, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN5, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN6, 0x3C);
			uvLedDriver.SetIntensity(Led_driver::LED_CHAN7, 0x3C);

			uvLedDriver.Refresh();
		}


		// Wait 250 ms before taking picture
		//usleep(50000);
		//system("gst-launch-1.0 v4l2src device=/dev/video3 num-buffers=1 ! video/x-raw,width=1024,height=768 ! jpegenc ! filesink location=sample.jpg");
		//usleep(250000);

		// Proceed to several readings at specific frequence
		for(i=0; i<3; i++){
			usleep(250000);
			uvSensor.ReadUVB();
		};

		sleep(7);

		if(uvLedDriver.Off())
			cout << "Led driver refresh failed" << endl;
		else
			cout << "Led driver refreshed" << endl;

		/*if(visLedDriver.Blink(100000))
			cout << "Led driver refresh failed" << endl;
		else
			cout << "Led driver refreshed" << endl;*/

		visLedDriver.Blink(25000);

		// Disable
		visLedDriver.Disable();

		uvLedDriver.Disable();

		cout << "End of Led driver Tests" << endl;

		sleep(1);
	}

	sleep(1);

	if(true){
		cout << "Testing Visible Photo-diode drivers" << endl;
		Vis visSensor;
		visSensor.Init(IMX8X_MIPI_I2C0_BUS, VEML6035_I2C_ADDRESS);

		// Then configure chip for UVB acquisition
		visSensor.ConfigSensor();

		// Proceed to several readings at specific frequence
		for(i=0; i<3; i++){
			usleep(250000);
			visSensor.ReadAls();
			visSensor.ReadWhite();
		}
	}

	cout << "Testing Temperature and humidity sensor drivers" << endl;

	ThmSensor thmSensor;
	if(true){
		thmSensor.Init(IMX8X_MIPI_I2C0_BUS, HDC1080_I2C_ADDRESS);
		thmSensor.ReadIDs();
		thmSensor.ConfigAcquisition();

		// Proceed to several readings at specific frequence
		for(i=0; i<3; i++){
			thmSensor.StartAcquisition();
			usleep(250000);
			thmSensor.ReadResults();
			thmSensor.GetTemperature();
			thmSensor.GetHumidity();
		}
		cout << "End of Temperature sensor driver Tests" << endl;
	}



	cout << "Testing Battery drivers" << endl;

	// Measure current battery level through I2C
	cout << "End of Battery driver Tests" << endl;


	cout << "End of Lumedix-driver Tests" << endl;
	return 0;
}

