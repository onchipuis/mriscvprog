#!/bin/bash
rmmod ftdi_sio
rmmod usb_serial
./mRISCVprog -v -n ./tests/test1.dat
#./mRISCVprog -v -n ./tests/test2.dat
#./mRISCVprog -v -n ./tests/test3.dat
#./mRISCVprog -v ./tests/add_tb.dat
#./mRISCVprog -v ./add.dat


