# mriscvprog 

mriscv programer using LIBMPSSE SPI library.

DOCUMENTATION:

Usage: mRISCVprog [OPTION...] FILE
mRISCVprog -- a program using FTDI USB-MPSSE SPI function for programming a
mRISC-V

  -a, --addr=DATA            Read data from device at specified addr (For write
                             use also -d)
  -c, --act                  ONLY activate the device (Do not use with -n)
  -d, --data=DATA            Write this data to device at specified addr (addr
                             required)
  -f, --force                Force to program the device
  -n, --noact                DO NOT activate the device after programming (Do
                             not use with -c)
  -q, -s, --quiet, --silent  Don't produce any output
  -u, --data=START,SIZE      Dump memory from START to START+SIZE
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message

BASIC USES:

- For programming use this command
./mRISCVprog /path/to/dat 

- For programming WITHOUT activating the core, use this command:
./mRISCVprog /path/to/dat -n

- For core use activation (ONLY):
./mRISCprog -c

- For reading in a specific address:
./mRISCprog -a [address]

- For writting in a specific address some data:
./mRISCprog -a [address] -d [data]

- For dumping data (experimental):
./mRISCprog -u START,END

NOTES:
- Every address and data must be indicated using hexadecimal values
- Do not use 0xXXXX values, only XXXX values
- Use -v for use verbose for knowing "what's happening".

HOW IT WORKS:

Common programming is issued to the microcontroller following the 
next three simple steps:
- Putting RESET to 0 (deactivate core)
- Write from address 0 to addres 3FF the RAM data.
- Putting RESET to 1 (activate core)

WARNINGS:

- To unload of the ftdi serial drivers, execute the next commands:
rmmod ftdi_sio
rmmod usbserial
- mRISCVtest only displays a "is busy commands" message and returns state.
