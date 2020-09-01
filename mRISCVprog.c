/*!
 * \file mRISCVprog.c
 *
 * \author Ckristian Duran (At On-Chip)
 * \date 20160810 (AAAAMMDD)
 *
 * Copyleft 2015-2016 On-Chip Investigation Group at UIS
 * 
 *
 *
 * Project: libMPSSE
 * Module: SPI libMPSSE mRISC-V programmer
 *
 * Rivision History:
 * 0.1  - 20160810 - Initial version
 */

// compile using:
// gcc -Wall mRISCVprog.c greset.c -o mRISCVprog -L./ -lMPSSE -lftd2xx -ldl
// gcc -Wall mRISCVprog.c greset.c -DGW32C -o mRISCVprog.exe -L./ -lMPSSE -lftd2xx -lgw32c -lintl-8
// execute it using:
// LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ ./mRISCVprog
/******************************************************************************/
/* 							 Include files										   */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <argp.h>
#include<unistd.h>
#include <signal.h>
/* OS specific libraries */
#ifdef _WIN32
//#include<windows.h>
#endif

#ifdef __linux
#include<dlfcn.h>
#include "WinTypes.h"
#endif

/* Include D2XX header*/
//#include "WinTypes.h"
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_spi.h"

/* Include Global Reset*/
#include "greset.h"

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);return 0;}else{;}};
#define APP_CHECK_STATUS_INV(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);return 1;}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);return 0;}else{;}};
#define CHECK_NULL_INV(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);return 1;}else{;}};
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/* Application specific macro definations */
#define CHANNEL_TO_OPEN				0	/*0 for first available channel, 1 for next... */
#define SPI_DEVICE_BUFFER_SIZE		256
#define SPI_CLOCK_RATE_HZ			1000000
#define MRISCV_MAX_SIZE				4096
#define MRISCV_SPI_DUMMY_BITS		10
#define MRISCV_SPI_TASK_BITS		2
#define MRISCV_SPI_ADDR_BITS 		32
#define MRISCV_SPI_DATA_BITS 		32
#define MRISCV_SPI_COMM_WRITE_BITS	(MRISCV_SPI_TASK_BITS+MRISCV_SPI_ADDR_BITS+MRISCV_SPI_DATA_BITS)
#define MRISCV_SPI_COMM_READ_BITS	(MRISCV_SPI_TASK_BITS+MRISCV_SPI_ADDR_BITS+MRISCV_SPI_DATA_BITS)
#define MRISCV_SPI_COMM_RESET_BITS	(MRISCV_SPI_TASK_BITS+MRISCV_SPI_ADDR_BITS+MRISCV_SPI_DATA_BITS)
#define MRISCV_SPI_COMM_STATUS_BITS	MRISCV_SPI_TASK_BITS
#define MRISCV_SPI_COMM_SEND_BITS	MRISCV_SPI_TASK_BITS
#define MRISCV_SPI_READ_STATUS_BITS	MRISCV_SPI_DATA_BITS
#define MRISCV_SPI_READ_SEND_BITS	MRISCV_SPI_DATA_BITS
#define MRISCV_CONFIG_MPSSE_READ	(SPI_CONFIG_OPTION_MODE2 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW)
#define MRISCV_CONFIG_MPSSE_WRITE	(SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW)
#define MRISCV_TASK_STATUS			0x0
#define MRISCV_TASK_READ			0x1
#define MRISCV_TASK_WRITE			0x2
#define MRISCV_TASK_SEND			0x3
#define MAX_COUNT_TIMEOUT			10

/******************************************************************************/
/*								Struct definitions							  	    */
/******************************************************************************/

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *strFile;                /* the FILE arg */
  int silent, verbose, force, isfile, act, noact, nodeact, isaddr, isdata, isdump, check;
  uint32 addr;
  uint32 data;
  uint32 addrdump;
  uint32 sizedump;
};
struct arguments arguments;

/******************************************************************************/
/*						Public function declarations					  		   */
/******************************************************************************/
static error_t
parse_opt (int key, char *arg, struct argp_state *state);
static int reset_status(uint8 state);

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
ChannelConfig channelConf = {0};
static FT_HANDLE ftHandle;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};
/* Program documentation. */
static char doc[] =
  "mRISCVprog -- a program using FTDI USB-MPSSE SPI function for programming a mRISC-V";

/* A description of the arguments we accept. */
static char args_doc[] = "FILE";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"quiet",    'q', 0,      0,  "Don't produce any output" },
  {"silent",   's', 0,      OPTION_ALIAS },
  {"force",    'f', 0,      0,  "Force to program the device" },
  {"noact",    'n', 0,      0,  "DO NOT activate the device after programming (Do not use with -c)" },
  {"nodeact",  'e', 0,      0,  "DO NOT DEactivate the device after programming (Do not use with -c)" },
  {"act",      'c', 0,      0,  "ONLY activate the device (Do not use with -n)" },
  {"addr",     'a', "DATA",      0,  "Read data from device at specified addr (For write use also -d)" },
  {"data",     'd', "DATA",      0,  "Write this data to device at specified addr (addr required)" },
  {"dump",     'u', "START,SIZE",      0,  "Dump memory from START to START+SIZE" },
  {"check",    'h', 0,      0,  "Check the program" },
  { 0 }
};
/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/
/*!
 * \brief Parse option from argp function.
 *
 * This function communicates options passed to program via argp.
 *
 * \param[in] key Pre-established key for argument to process.
 * \param[in] arg String with the argument.
 * \param[in] state argp_state structure.
 * \return
 * \note
 * \warning
 */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q': case 's':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'f':
      arguments->force = 1;
      break;
    case 'n':
      arguments->noact = 1;
      break;
    case 'e':
      arguments->nodeact = 1;
      break;
    case 'c':
      arguments->act = 1;
      break;
    case 'a':
      arguments->isaddr = 1;
      arguments->addr = arg ? strtol(arg, NULL, 16) : 0;
      break;
    case 'd':
      arguments->isdata = 1;
      arguments->data = arg ? strtol(arg, NULL, 16) : 0;
      break;
    case 'h':
      arguments->check = 1;
      break;
    case 'u':
      arguments->isdump = 1;
      if(arg)
      {
      	arguments->addrdump = 0;
      	arguments->sizedump = MRISCV_MAX_SIZE;
      }
      else{
		  char* argf = strchr(arg ,',');
		  if(argf == NULL) 
		  {
		  	fprintf(stderr, "WARN: Bad argument using -d (needs to be START,SIZE)\n");
		  	arguments->addrdump = 0;
		  	arguments->sizedump = MRISCV_MAX_SIZE;
		  }
		  else
		  {
			  (*argf) = 0; 
			  arguments->addrdump = strtol(arg, NULL, 16);
			  arguments->sizedump = strtol(argf+1, NULL, 16);
		  }
      }
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);

      arguments->strFile = arg;
      arguments->isfile = 1;

      break;

    case ARGP_KEY_END:
      //if (state->arg_num < 1)
        /* Not enough arguments. */
        //argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static int send_dummy(void)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	FT_STATUS status;
	
	sizeToTransfer=0;		// This is for forcing CSdisable
	sizeTransfered=0;
	buffer[0] = 0;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status); 
	
	sizeToTransfer=MRISCV_SPI_DUMMY_BITS;
	sizeTransfered=0;
	buffer[0] = 0;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status); 
	return 1;
}

static int read_status(uint8 *stat)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	FT_STATUS status;
	send_dummy();
	/*Write 2 bit SEND STATUS*/
	sizeToTransfer=MRISCV_SPI_COMM_STATUS_BITS;
	sizeTransfered=0;
	buffer[0] = MRISCV_TASK_STATUS << 6;	/* Write command (2bit, 6-bit displaced)*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	APP_CHECK_STATUS(status);

	/*Read the status*/
	sizeToTransfer=MRISCV_SPI_READ_STATUS_BITS;

	sizeTransfered=0;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();
	
	/*Is done?*/
	(*stat) = buffer[3];
	
	send_dummy();
	return 1;
}

/*!
 * \brief Write some word at some addr.
 *
 * This function writes a 32-bit word to a specified address in mRISC-V
 *
 * \param[in] address 32-bit address from the whole bus to write
 * \param[in] data 32-bit word data that is to be written
 * \return Return 1 is sucessful, 0 if write is not possible to write.
 * \note
 * \warning
 */
static int write_single_word(uint32 address, uint32 data)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	uint8 stat;
	uint32 count = 0;
	FT_STATUS status;
RESTART_WRITE:
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();

	/* CS_High + Write command + Address */
	sizeToTransfer=MRISCV_SPI_COMM_WRITE_BITS;
	sizeTransfered=0;
	//buffer[0] = ;	/* Write command (2bit, 6-bit displaced)*/
	buffer[0] = (MRISCV_TASK_WRITE << 6) | ( ( address >> 26 ) & 0x3F ); 	/*MSB  6-bit addr bits*/
	buffer[1] = ( ( address >> 18 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[2] = ( ( address >> 10 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[3] = ( ( address >> 2 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[4] = ( ( address & 0x3 ) << 6 ); 				/*Last 2-bit addr bits*/
	buffer[4] = buffer[4] | ( ( data >> 26 ) & 0x3F ); 		/*MSB  6-bit data bits*/
	buffer[5] = ( ( data >> 18 ) & 0xFF ); 					/*Next 8-bit data bits*/
	buffer[6] = ( ( data >> 10 ) & 0xFF ); 					/*Next 8-bit data bits*/
	buffer[7] = ( ( data >> 2 ) & 0xFF ); 					/*Next 8-bit data bits*/
	buffer[8] = ( ( data & 0x3 ) << 6 ); 					/*Last 2-bit data bits*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);
	
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();
	
	while(1) {
		read_status(&stat);
		if((stat & 0x7)) 
		{
			count ++;
			if(count > MAX_COUNT_TIMEOUT)
			{
				count = 0;
				printf("TIMEOUT WRITE, RETRYING\n");
			  sendGlobalReset(1);
			  send_dummy();
				if(!arguments.nodeact)
				{
				  reset_status(0xFF);
				
				  sleep(2);
				}
				
			  sendGlobalReset(0);
			  send_dummy();
				if(!arguments.nodeact)
				{
			    reset_status(0x0);
			
			    sleep(2);
				}
				goto RESTART_WRITE;
			}
		}
		else break;
	}

	return 1;
}

/*!
 * \brief Read some word at some addr.
 *
 * This function reads a 32-bit word to a specified address in mRISC-V
 *
 * \param[in] address 32-bit address from the whole bus to write
 * \param[out] data 32-bit word data that is to be written
 * \return Return 1 is sucessful, 0 if write is not possible to read.
 * \note
 * \warning
 */
static int read_single_word(uint32 address, uint32 *data)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	uint8 stat;
	uint32 count = 0;
	FT_STATUS status;

RESTART_READ:
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();

	/* CS_High + Write command + Address */
	sizeToTransfer=MRISCV_SPI_COMM_WRITE_BITS;
	sizeTransfered=0;
	//buffer[0] = ;	/* Write command (2bit, 6-bit displaced)*/
	buffer[0] = (MRISCV_TASK_READ << 6) | ( ( address >> 26 ) & 0x3F ); 	/*MSB  6-bit addr bits*/
	buffer[1] = ( ( address >> 18 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[2] = ( ( address >> 10 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[3] = ( ( address >> 2 ) & 0xFF ); 				/*Next 8-bit addr bits*/
	buffer[4] = ( ( address & 0x3 ) << 6 ); 				/*Last 2-bit addr bits*/
	//buffer[4] = buffer[4] | 0; 							/*MSB  6-bit data bits (IGNORED)*/
	buffer[5] = 0; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[6] = 0; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[7] = 0; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[8] = 0; 										/*Last 2-bit data bits (IGNORED)*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);
	
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();
	
	while(1) {
		read_status(&stat);
		if((stat & 0x7)) 
		{
			count ++;
			if(count > MAX_COUNT_TIMEOUT)
			{
				count = 0;
				printf("TIMEOUT READ, RETRYING\n");
				goto RESTART_READ;
			}
		}
		else break;
	}
	
	/*Write 2 bit SEND RDATA*/
	sizeToTransfer=MRISCV_SPI_COMM_SEND_BITS;
	sizeTransfered=0;
	buffer[0] = MRISCV_TASK_SEND << 6;	/* Write command (2bit, 6-bit displaced)*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
	APP_CHECK_STATUS(status);

	/*Read the data*/
	sizeToTransfer=MRISCV_SPI_READ_SEND_BITS;
	sizeTransfered=0;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);
	
	(*data) =  buffer[0] << 24;	/*IS NOT FUCKING LITTLE ENDIAN*/
	(*data) |= buffer[1] << 16;
	(*data) |= buffer[2] << 8 ;
	(*data) |= buffer[3];
	
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();

	return 1;
}

/*!
 * \brief Set reset to some state.
 *
 * This function puts on MRISCV_RST on any state if required.
 * Put it on 0 to activate, put in 1 to reset and deactivate.
 *
 * \param[in] state RESET state-
 * \return Return 1 is sucessful, 0 if write is not possible to put the RESET.
 * \note
 * \warning
 */
static int reset_status(uint8 state)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	uint8 writeComplete=0;
	FT_STATUS status;
	send_dummy();

	/* CS_High + Write command + Address */
	sizeToTransfer=MRISCV_SPI_COMM_RESET_BITS+10;
	sizeTransfered=0;
	//buffer[0] = ;	/* Write command (2bit, 6-bit displaced)*/
	buffer[0] = (MRISCV_TASK_STATUS << 6) | (state & 0x3F); 								/*MSB  6-bit addr bits (IGNORED)*/
	buffer[1] = state; 							 				/*Next 8-bit addr bits (IGNORED)*/
	buffer[2] = state; 							 				/*Next 8-bit addr bits (IGNORED)*/
	buffer[3] = state; 							 				/*Next 8-bit addr bits (IGNORED)*/
	buffer[4] = state; 							 				/*Last 2-bit addr bits, MSB 6-bit data bits (IGNORED)*/
	buffer[5] = state; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[6] = state; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[7] = state; 											/*Next 8-bit data bits (IGNORED)*/
	buffer[8] = state;										/*Last 2-bit data bits (RESET STATE)*/
	buffer[9] = state;										/*Last 2-bit data bits (RESET STATE)*/
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);
	
	/* Dummy Bits, for issue aditional clk cycles */
	send_dummy();

	return 1;
}

/*!
 * \brief Signal callback for SIGTERM
 *
 * This function will close the MPSSE library, for avoiding some undesired behavioral.
 *
 * \param[in] sig Signal number
 * \param[in] siginfo Signal structure
 * \param[in] context Aditional context passed to the function
 * \return none 
 * \sa

 * \note
 * \warning

 */
void termination_handler (int sig)
{
	FT_STATUS status = FT_OK;
	send_dummy();
	status = SPI_CloseChannel(ftHandle);

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif
	fprintf(stderr, "mRISCVprog interrupted!\n");
}

/*!
 * \brief Main function / Entry point to the application
 *
 * This function is the entry point to the sample application. Reads the data from a .dat file, then
 * writes it to memory via SPI into mRISC-V.
 *
 * \param[in] argc Number of arguments passed to application
 * \param[in] argv Arguments
 * \return Returns 0 for success, otherwise if failed 
 * \sa
 * \note
 * \warning
 */
int main(int argc, char **argv)
{
	FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint8 latency = 255;
	uint32 i;
	uint32 file_size;
	uint32 data;
	uint32 dev_to_open;
	
	/* signal binding*/
	if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);
  if (signal (SIGHUP, termination_handler) == SIG_IGN)
    signal (SIGHUP, SIG_IGN);
  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);
    
  /* Init Global Reset */
  initGlobalReset();
	
	
	/*Argument checking*/	
	memset(&arguments, 0, sizeof(struct arguments));
	if(argp_parse (&argp, argc, argv, 0, 0, &arguments) != 0) return 1;
	if(arguments.act && arguments.noact) {fprintf(stderr, "ERROR: Argument mismatch. Cannot use -c with -n\n"); return -1;}
	
	if(arguments.verbose) printf("Welcome to mRISC-V programmer!\n");
	
	channelConf.ClockRate = SPI_CLOCK_RATE_HZ;
	channelConf.LatencyTimer = latency;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x00000000;	/*According to manual, this is not needed because directions of pins are overriden in SPI*/

	/* init library */
#ifdef _MSC_VER
	Init_libMPSSE();
#endif
	status = SPI_GetNumChannels(&channels);
	APP_CHECK_STATUS_INV(status);
	if(arguments.verbose)  printf("Number of available SPI channels = %d\n",(int)channels);

	if(channels>0)
	{
		// NO, WE ARENT GOING TO FUCKING OPEN THE FIRST AVAILABLE
		// WE'LL SEARCH OUR PROGRAMMER
		dev_to_open = 0xFFFFFFFF;
		for(i = 0; i < channels; i++){
			status = SPI_GetChannelInfo (i,&devList);
			APP_CHECK_STATUS_INV(status);
			
			if(arguments.verbose)  {
				printf("SPI channel = %u\n", i);
				printf(" Flags=0x%x\n",devList.Flags);
				printf(" Type=0x%x\n",devList.Type);
				printf(" ID=0x%x\n",devList.ID);
				printf(" LocId=0x%x\n",devList.LocId);
				printf(" SerialNumber=%s\n",devList.SerialNumber);
				printf(" Description=%s\n",devList.Description);
				printf(" ftHandle=0x%x\n",devList.ftHandle); 
			}
			if(/*devList.Flags == 0x2 && devList.Type == 0x8 && */devList.ID == 0x4036014/* && i == 0*/)
			{
				dev_to_open = i;
				break;
			}
		}
		if(dev_to_open == 0xFFFFFFFF) goto NOTHING_TO_DO;
		
		status = SPI_OpenChannel(dev_to_open,&ftHandle);
		APP_CHECK_STATUS_INV(status);
		
		status = SPI_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS_INV(status);
		
		send_dummy();
		
		if(arguments.act)
		{
			if(arguments.verbose) printf("Putting mRISC-V reset in 1...\n");
			if(!reset_status(0xFF)) {fprintf(stderr, "ERROR: Cannot activate the mRISC-V, re-program it and try again\n"); goto PANIC_EXIT;}
			if(arguments.verbose) printf("Sucess reset to 1.\n");
			goto NORMAL_EXIT;
		}
		
		else if(arguments.isaddr)
		{
			if(arguments.isdata)
			{
				if(arguments.verbose) printf("Send command WRITE 0x%x 0x%x.\n", arguments.addr, arguments.data);
				if(!write_single_word(arguments.addr, arguments.data)) {fprintf(stderr, "ERROR: Cannot send command WRITE 0x%x 0x%x.\n", arguments.addr, arguments.data); goto PANIC_EXIT;}
			}
			else
			{
				if(arguments.verbose) printf("Send command READ 0x%x.\n", arguments.addr);
				if(!read_single_word(arguments.addr, &data)) {fprintf(stderr, "ERROR: Cannot send command READ 0x%x.\n", arguments.addr); goto PANIC_EXIT;}
				if(arguments.verbose) printf("READ responds 0x%x.\n", data);
				else printf("0x%x", data);
			}
		}
		
		else if(arguments.isfile && arguments.isdump)
		{
			if(arguments.verbose) printf("Putting mRISC-V reset in 0...\n");
			if(!reset_status(0)) {fprintf(stderr, "ERROR: Cannot deactivate the mRISC-V, re-program it and try again\n"); goto PANIC_EXIT;}
			if(arguments.verbose) printf("Sucess reset to 0.\n");
			
			FILE *IN_FILE;
			if(arguments.verbose) printf("Trying to open specified file\n");
    		IN_FILE = fopen(arguments.strFile,"wb");
    		if(IN_FILE == NULL) {fprintf(stderr, "ERROR: Failed to open data file.\n"); goto PANIC_EXIT;}
			
			for(i = arguments.addrdump; i < (arguments.addrdump+arguments.sizedump); i+=4)
			{
				if(arguments.verbose) printf("Send command READ 0x%x.", i);
				if(!read_single_word(i, &data)) {fprintf(stderr, "ERROR: Cannot send command READ 0x%x.\n", i); goto PANIC_EXIT;}
				if(arguments.verbose) printf(" Returned 0x%x.\n", data);
				fwrite(&data, 4, 1, IN_FILE);
			}
			fclose(IN_FILE);
			
			if(!arguments.noact)
			{
				if(arguments.verbose) printf("Putting mRISC-V reset in 1...\n");
				if(!reset_status(0xFF)) {fprintf(stderr, "ERROR: Cannot activate the mRISC-V, re-program it and try again\n"); goto PANIC_EXIT;}
				if(arguments.verbose) printf("Sucess reset to 1.\n");
			}
		}
		
		else if(arguments.isfile)
		{
			FILE *IN_FILE;
			if(arguments.verbose) printf("Trying to open specified file\n");
  		IN_FILE = fopen(arguments.strFile,"rb");
  		if(IN_FILE == NULL) {fprintf(stderr, "ERROR: Failed to open data file.\n"); goto PANIC_EXIT;}
  		
			fseek(IN_FILE,0,SEEK_END);
			file_size = ftell(IN_FILE);
			fseek(IN_FILE,0,SEEK_SET);
			
			if(file_size > MRISCV_MAX_SIZE && !arguments.force) {fprintf(stderr, "ERROR: File size is too big (max 4096 bytes), please use -f option.\n"); goto PANIC_EXIT;}
			file_size = min(MRISCV_MAX_SIZE, file_size);
			
			for(i = 0; i < (file_size/4); i+=1)
			{
				fread(&data, 4, 1, IN_FILE);
				while(1)
				{
					if(arguments.verbose) printf("Send command WRITE 0x%x 0x%x.\n", i, data);
					if(!write_single_word(i, data)) {fprintf(stderr, "ERROR: Cannot send command WRITE 0x%x 0x%x.\n", i, data); goto PANIC_EXIT;}
					if(arguments.check)
					{
					  uint32 datar;
					  if(arguments.verbose) printf("Send command READ 0x%x.\n", i);
					  if(!read_single_word(i, &datar)) {fprintf(stderr, "ERROR: Cannot send command READ 0x%x.\n", i); goto PANIC_EXIT;}
					
					  if(data != datar) printf("ERROR: read and written data is different, retrying... (0x%x != 0x%x)\n", data, datar);
					  else break;
					}
					else break;
				}
				
			}
			fclose(IN_FILE);
			// Put the reset thing
			int l;
			for(l = 0; l < 1; l++)
			{
			  if(!arguments.nodeact)
			  {
			    if(arguments.verbose) printf("Putting mRISC-V reset in 1...\n");
			    if(!reset_status(0)) {fprintf(stderr, "ERROR: Cannot deactivate the mRISC-V, re-program it and try again\n"); goto PANIC_EXIT;}
			    if(arguments.verbose) printf("Sucess reset to 1.\n");
			    sleep(1);
			  }
			
			  if(!arguments.noact)
			  {
				  if(arguments.verbose) printf("Putting mRISC-V reset in 0...\n");
				  if(!reset_status(0xFF)) {fprintf(stderr, "ERROR: Cannot activate the mRISC-V, re-program it and try again\n"); goto PANIC_EXIT;}
				  if(arguments.verbose) printf("Sucess reset to 0.\n");
				  sleep(1);
			  }
			}
		}
		
		else
		{
			if(arguments.verbose) printf("Nothing to do.\n");
		}
	}
	else 
	{
NOTHING_TO_DO:
		fprintf(stderr, "ERROR: FTDI not attached.\n");
		return 1;
	}
	
NORMAL_EXIT:
	status = SPI_CloseChannel(ftHandle);
    
  /* Fini Global Reset */
  finiGlobalReset();

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif

	if(arguments.verbose) printf("Sucesfully programmed mRISC-V!\n");
	return 0;

PANIC_EXIT:
	status = SPI_CloseChannel(ftHandle);
    
  /* Fini Global Reset */
  finiGlobalReset();

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif

	if(arguments.verbose) printf("mRISCVprog terminated with errors!\n");
	return -1;
}

