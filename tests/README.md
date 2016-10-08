Please use this with a RISC-V I toolchain.

This is only available from the server. If you want to compile your own 
toolchain, a good startpoint is to go to:

https://github.com/onchipuis/mriscv

Then get your RISC-V I toolchain.

Commands used are on the source of every test:

- test1 : Leds blinking
- test2 : Turns ON leds
- test3 : Turns OFF leds
- test4 : "Knight Rider" (leds turning ON and OFF in a serial sequence;
one single led ON at a time)
- test5 : Turns ON all leds, one by one. Then it turns them OFF in order 
