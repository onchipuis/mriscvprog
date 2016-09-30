#!/bin/bash
rmmod ftdi_sio
rmmod usb_serial
./mRISCVprog -v -n ../test1.dat
#./mRISCVprog -v -n ../test2.dat
#./mRISCVprog -v -n ../test3.dat
#./mRISCVprog -v ../add_tb.dat
#./mRISCVprog -v ./add.dat


