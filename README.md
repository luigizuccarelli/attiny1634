# attiny1634
attiny1634 projects 

## iotcontroller

  A simple controller based on the attiny1634 mcu. The controller is an 8 channel adc converter with code to interface to a master SPI controller.
  The interface also has code to control an IO expander mcp23s17 with 2X8 channel digital ouputs

  Command line to flash iotcontroller

  ```bash

  avrdude -c usbtiny -p t1634 -v -Uflash:w:iotcontroller.hex:i

  ```

  Fuse settings (spi enabled,8MHz internal clock, divide by 8, start up time at max)

  ```bash

  avrdude -c usbtiny -p t1634 -V -U lfuse:w:0x62:m -U hfuse:w:0xdF:m -U efuse:w:0xFF:m

  ```

## mcp23s17-test
 
  Simple routine to test the spi interface and io expander chip

## blinktest

  A simple blinktest, switching an led on and off with some delay
