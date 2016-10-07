# mriscvprog 

A programmer of mriscv using LIBMPSSE SPI library.

# PRIMARY WARNING:

THIS PROGRAM IS NOT 100% FUNCTIONAL, in fact this is a 0.1 version.
For programming mRISCV on fpga or chip version needed to do this:

- run ./mRISCVtest1, then press RESET button on the board, then ctrl-c it.
- run ./do.sh for some test programation.
- run ./mRISCVtest1, then press RESET button on the board again, then ctrl-c it.
- run ./mRISCVprog -a 0 -d [first opcode_hex], this forces to be written the 
  first RAM address. (This needs to be run a couple of times)
- run ./mRISCVprog -c for activating mriscvcore.

This worked for us, its very ugly and we promise to fix it as soon as possible.

UPDATE 2:

- run ./mRISCVtest1, then press RESET button on the board, then ctrl-c it.
- run ./do.sh for some test programation.

Now is easier to program this thing

# DOCUMENTATION:

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

- For programming just use this command
./mRISCVprog /path/to/dat 

- For programming and DO NOT activate core, use this command:
./mRISCVprog /path/to/dat -n

- For ONLY activate core use:
./mRISCprog -c

- For reading at random address:
./mRISCprog -a [address]

- For writting at random address some data:
./mRISCprog -a [address] -d [data]

- For dumping data (experimental) use this:
./mRISCprog -u START,END

- Every address and data must be indicated using hexadecimal values
- Do not use 0xXXXX values, only XXXX values
- Use -v for use verbose for knowing "what's happening".

HOW IT WORKS:

Common programming is issued to the microcontroller in those single steps:
- Putting RESET to 0 (deactivate core)
- Write from address 0 to addres 3FF the RAM data.
- Putting RESET to 1 (activate core)

WARNINGS:

- Need to unload ftdi serial drivers, execute the next command to do it:
rmmod ftdi_sio
rmmod usbserial
- mRISCVtest only issues "is busy commands" and returns state.
