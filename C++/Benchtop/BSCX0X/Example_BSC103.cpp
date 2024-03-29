/*
BSC103 Simple Example
Date of Creation(YYYY-MM-DD): 2022-06-06
Date of Last Modification on Github: 2022-06-08
C++ Version Used: ISO C++ 14
Kinesis Version Tested: 1.14.40
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

// Include device-specific header file
#include "Thorlabs.MotionControl.Benchtop.StepperMotor.h"

int __cdecl wmain(int argc, wchar_t* argv[])
{
	// Uncomment this line (and TLI_UnitializeSimulations at the bottom of the page)
	// If you are using a simulated device
	//TLI_InitializeSimulations();

	// Change this line to reflect your device's serial number
	int serialNo = 70000001;
	
	// Optionally set the position (in device units)
	int position = 0;
	
	// Optionally set the position (in device units/second)
	int velocity = 0;
	
	// identify and access device
	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);

	// Build list of connected device
    if (TLI_BuildDeviceList() == 0)
    {
		// get device list size 
        short n = TLI_GetDeviceListSize();
		// get BBD serial numbers
        char serialNos[100];
		TLI_GetDeviceListByTypeExt(serialNos, 100, 70);

		// output list of matching devices
		{
			char *searchContext = nullptr;
			char *p = strtok_s(serialNos, ",", &searchContext);

			while (p != nullptr)
			{
				TLI_DeviceInfo deviceInfo;
				// get device info from device
				TLI_GetDeviceInfo(p, &deviceInfo);
				// get strings from device info structure
				char desc[65];
				strncpy_s(desc, deviceInfo.description, 64);
				desc[64] = '\0';
				char serialNo[9];
				strncpy_s(serialNo, deviceInfo.serialNo, 8);
				serialNo[8] = '\0';
				// output
				printf("Found Device %s=%s : %s\r\n", p, serialNo, desc);
				p = strtok_s(nullptr, ",", &searchContext);
			}
		}

		// open device
		if(SBC_Open(testSerialNo) == 0)
		{
			// start the device polling at 200ms intervals
			SBC_StartPolling(testSerialNo, 1, 200);

			// enable device so that it can move
			SBC_EnableChannel(testSerialNo, 1);

			Sleep(3000);
			// Home device
			SBC_ClearMessageQueue(testSerialNo, 1);
			SBC_Home(testSerialNo, 1);
			printf("Device %s homing\r\n", testSerialNo);

			// wait for completion
			WORD messageType;
			WORD messageId;
			DWORD messageData;
			SBC_WaitForMessage(testSerialNo, 1, &messageType, &messageId, &messageData);
			while(messageType != 2 || messageId != 0)
			{
				SBC_WaitForMessage(testSerialNo, 1, &messageType, &messageId, &messageData);
			}

			// set velocity if desired
			if(velocity > 0)
			{
				int currentVelocity, currentAcceleration;
				SBC_GetVelParams(testSerialNo, 1, &currentAcceleration, &currentVelocity);
				SBC_SetVelParams(testSerialNo, 1, currentAcceleration, velocity);
			}

			// move to position (channel 1)
			SBC_ClearMessageQueue(testSerialNo, 1);
			SBC_MoveToPosition(testSerialNo, 1, position);
			printf("Device %s moving\r\n", testSerialNo);

			// wait for completion
			SBC_WaitForMessage(testSerialNo, 1, &messageType, &messageId, &messageData);
			while(messageType != 2 || messageId != 1)
			{
				SBC_WaitForMessage(testSerialNo, 1, &messageType, &messageId, &messageData);
			}

			// get actual poaition
			int pos = SBC_GetPosition(testSerialNo, 1);
			printf("Device %s moved to %d\r\n", testSerialNo, pos);

			// stop polling
			SBC_StopPolling(testSerialNo, 1);
			// close device
			SBC_Close(testSerialNo);
	    }
    }

	// Uncomment this line if you are using simulations
	//TLI_UnitializeSimulations;
	char c = _getch();
	return 0;
}
