/*
	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o write main.c -L. -lftd2xx -Wl,-rpath,/usr/local/lib
*/

#include <stdio.h>
#include <sys/time.h>
#include "../../ftd2xx.h"

int main(int argc, char *argv[])
{
	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle0;
	int iport;
	
	if(argc > 1) {
		sscanf(argv[1], "%d", &iport);
	}
	else {
		iport = 0;
	}
	printf("opening port %d\n", iport);
	FT_SetVIDPID(0x0403, 0x6011);
	ftStatus = FT_Open(iport, &ftHandle0);
	
	if(ftStatus != FT_OK) {
		/* 
			This can fail if the ftdi_sio driver is loaded
		 	use lsmod to check this and rmmod ftdi_sio to remove
			also rmmod usbserial
		 */
		printf("FT_Open(%d) failed\n", iport);
		return 1;
	}

	printf("ftHandle0 = %p\n", ftHandle0);

#ifdef BM_DEVICE
	FT_PROGRAM_DATA Data;
	Data.Signature1 = 0x00000000;
	Data.Signature2 = 0xffffffff;
	Data.VendorId = 0x0403;				
	Data.ProductId = 0x6001;
	Data.Manufacturer =  "FTDI";
	Data.ManufacturerId = "FT";
	Data.Description = "USB <-> Serial";
	Data.SerialNumber = "FT000001";		// if fixed, or NULL
	
	Data.MaxPower = 44;
	Data.PnP = 1;
	Data.SelfPowered = 0;
	Data.RemoteWakeup = 1;
	Data.Rev4 = 1;
	Data.IsoIn = 0;
	Data.IsoOut = 0;
	Data.PullDownEnable = 1;
	Data.SerNumEnable = 1;
	Data.USBVersionEnable = 0;
	Data.USBVersion = 0x110;
#else // If 2232C	

	/*Data.Signature1 = 0x00000000;
	Data.Signature2 = 0xffffffff;
	Data.VendorId = 0x0403;				
	Data.ProductId = 0x6010;
	Data.Manufacturer =  "FTDI";
	Data.ManufacturerId = "FT";
	Data.Description = "SPI";
	Data.SerialNumber = "FT123452";		// if fixed, or NULL
	
	Data.MaxPower = 200;
	Data.PnP = 1;
	Data.SelfPowered = 0;
	Data.RemoteWakeup = 0;
	Data.Rev4 = 0;
	Data.IsoIn = 0;
	Data.IsoOut = 0;
	Data.PullDownEnable = 0;
	Data.SerNumEnable = 0;
	Data.USBVersionEnable = 0;
	Data.USBVersion = 0;

	Data.Rev5 = 1;					// non-zero if Rev5 chip, zero otherwise
	Data.IsoInA = 0;				// non-zero if in endpoint is isochronous
	Data.IsoInB = 0;				// non-zero if in endpoint is isochronous
	Data.IsoOutA = 0;				// non-zero if out endpoint is isochronous
	Data.IsoOutB = 0;				// non-zero if out endpoint is isochronous
	Data.PullDownEnable5 = 0;		// non-zero if pull down enabled
	Data.SerNumEnable5 = 1;			// non-zero if serial number to be used
	Data.USBVersionEnable5 = 0;		// non-zero if chip uses USBVersion
	Data.USBVersion5 = 0x0200;		// BCD (0x0200 => USB2)
	Data.AIsHighCurrent = 0;		// non-zero if interface is high current
	Data.BIsHighCurrent = 0;		// non-zero if interface is high current
	Data.IFAIsFifo = 1;				// non-zero if interface is 245 FIFO
	Data.IFAIsFifoTar = 0;			// non-zero if interface is 245 FIFO CPU target
	Data.IFAIsFastSer = 0;			// non-zero if interface is Fast serial
	Data.AIsVCP = 0;				// non-zero if interface is to use VCP drivers
	Data.IFBIsFifo = 1;				// non-zero if interface is 245 FIFO
	Data.IFBIsFifoTar = 0;			// non-zero if interface is 245 FIFO CPU target
	Data.IFBIsFastSer = 0;			// non-zero if interface is Fast serial
	Data.BIsVCP = 0;				// non-zero if interface is to use VCP drivers
	*/
	
	
	FT_PROGRAM_DATA Data = {
		0x00000000, // Header - must be 0x00000000
		0xFFFFFFFF, // Header - must be 0xffffffff
		0x00000004, // Header - FT_PROGRAM_DATA version
		0x0403, // VID
		0x6014, // PID
		"FTDI", // Manufacturer
		"FT", // Manufacturer ID
		"C232HM-VSW3V3", // Description
		"FT000001", // Serial Number
		500, // MaxPower
		1, // PnP
		0, // SelfPowered
		0, // RemoteWakeup
		1, // non-zero if Rev4 chip, zero otherwise
		0, // non-zero if in endpoint is isochronous
		0, // non-zero if out endpoint is isochronous
		0, // non-zero if pull down enabled
		1, // non-zero if serial number to be used
		0, // non-zero if chip uses USBVersion
		0x0110, // BCD (0x0200 => USB2)
		//
		// FT2232C extensions (Enabled if Version = 1 or greater)
		//
		0, // non-zero if Rev5 chip, zero otherwise
		0, // non-zero if in endpoint is isochronous
		0, // non-zero if in endpoint is isochronous
		0, // non-zero if out endpoint is isochronous
		0, // non-zero if out endpoint is isochronous
		0, // non-zero if pull down enabled
		0, // non-zero if serial number to be used
		0, // non-zero if chip uses USBVersion
		0x0, // BCD (0x0200 => USB2)
		0, // non-zero if interface is high current
		0, // non-zero if interface is high current
		0, // non-zero if interface is 245 FIFO
		0, // non-zero if interface is 245 FIFO CPU target
		0, // non-zero if interface is Fast serial
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if interface is 245 FIFO
		0, // non-zero if interface is 245 FIFO CPU target
		0, // non-zero if interface is Fast serial
		0, // non-zero if interface is to use VCP drivers
		//
		// FT232R extensions (Enabled if Version = 2 or greater)
		//
		0, // Use External Oscillator
		0, // High Drive I/Os
		0, // Endpoint size
		0, // non-zero if pull down enabled
		0, // non-zero if serial number to be used
		0, // non-zero if invert TXD
		0, // non-zero if invert RXD
		0, // non-zero if invert RTS
		0, // non-zero if invert CTS
		0, // non-zero if invert DTR
		0, // non-zero if invert DSR
		0, // non-zero if invert DCD
		0, // non-zero if invert RI
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // non-zero if using D2XX drivers
		//
		// Rev 7 (FT2232H) Extensions (Enabled if Version = 3 or greater)
		//
		0, // non-zero if pull down enabled
		0, // non-zero if serial number to be used
		0, // non-zero if AL pins have slow slew
		0, // non-zero if AL pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if AH pins have slow slew
		0, // non-zero if AH pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if BL pins have slow slew
		0, // non-zero if BL pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if BH pins have slow slew
		0, // non-zero if BH pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if interface is 245 FIFO
		0, // non-zero if interface is 245 FIFO CPU target
		0, // non-zero if interface is Fast serial
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if interface is 245 FIFO
		0, // non-zero if interface is 245 FIFO CPU target
		0, // non-zero if interface is Fast serial
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if using BCBUS7 to save power for self-
		// powered designs
		//
		// Rev 8 (FT4232H) Extensions (Enabled if Version = 4)
		//
		0, // non-zero if pull down enabled
		0, // non-zero if serial number to be used
		0, // non-zero if AL pins have slow slew
		0, // non-zero if AL pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if AH pins have slow slew
		0, // non-zero if AH pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if BL pins have slow slew
		0, // non-zero if BL pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if BH pins have slow slew
		0, // non-zero if BH pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if port A uses RI as RS485 TXDEN
		0, // non-zero if port B uses RI as RS485 TXDEN
		0, // non-zero if port C uses RI as RS485 TXDEN
		0, // non-zero if port D uses RI as RS485 TXDEN
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if interface is to use VCP drivers
		0, // non-zero if interface is to use VCP drivers
		//
		// Rev 9 (FT232H) Extensions (Enabled if Version = 5)
		//
		0, // non-zero if pull down enabled
		0, // non-zero if serial number to be used
		0, // non-zero if AC pins have slow slew
		0, // non-zero if AC pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		0, // non-zero if AD pins have slow slew
		0, // non-zero if AD pins are Schmitt input
		0, // valid values are 4mA, 8mA, 12mA, 16mA
		2, // Cbus Mux control
		1, // Cbus Mux control
		1, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // Cbus Mux control
		0, // non-zero if interface is 245 FIFO
		0, // non-zero if interface is 245 FIFO CPU target
		0, // non-zero if interface is Fast serial
		0, // non-zero if interface is FT1248
		0, // FT1248 clock polarity - clock idle high (1) or
		// clock idle low (0)
		0, // FT1248 data is LSB (1) or MSB (0)
		0, // FT1248 flow control enable
		0, // non-zero if interface is to use VCP drivers
		0 // non-zero if using ACBUS7 to save power for
		// self-powered designs
		};

#endif								

	ftStatus = FT_EE_Program(ftHandle0, &Data);
	if(ftStatus != FT_OK) {
		printf("FT_EE_Program failed (%d)\n", (int)ftStatus);
		FT_Close(ftHandle0);
	}
	FT_Close(ftHandle0);
	return 0;
}
