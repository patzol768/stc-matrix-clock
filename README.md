# Firmware replacement for Chinese STC based processor DIY LED matrix clock kit

This is a replacement program for the STC 8051 core uP based DIY digital clock kits available from numerous Chinese sources. The specific clock used to develop this firmware was purchased long ago, a direct link is not available now. By front look [this one](https://www.banggood.com/5V-DIY-Dot-Matrix-Digit-LED-Electronic-Clock-Kit-Temperature-24-Hours-Display-p-1093866.html) is the same, though from the back it has slightly different placement of parts and has light sensor as well.

Code base and the idea comes from [STC LED replacement firmware](https://github.com/aFewBits/stc-led-clock) by [aFewBits](https://github.com/aFewBits), though in several aspects heavily modified to use for a dot matrix board.

![Front image](https://imgaz.staticbg.com/images/oaupload/banggood/images/C9/6C/46796700-a8d3-454d-9a8e-82aeff828bad.jpg)
![Back image](https://imgaz.staticbg.com/images/oaupload/banggood/images/C9/6C/e5a84574-711c-46ec-9251-89f1930af011.jpg)

## Getting Started
You'll need [SDCC](http://sdcc.sf.net) to build and [STC-ISP](http://www.stcmcudata.com/STCISP/stc-isp-15xx-v6.86D.zip) or [STCGAL](https://github.com/grigorig/stcgal) to set the clock speed, processor hardware options and to flash the firmware.

In addition to the above tools, you will need one serial port with TTL outputs for programming the STC processor. The simplest solution is usually an inexpensive USB to serial TTL converter. These can be based on the FT232 chipset, PL2303 or CH340. They all work equally well, it's just a matter of what you find at the ready when you need it.

### Clock Features
* Time display (12 and 24 hour modes, with or without leading zero).
* Alarm (using the rather loud internal buzzer) with snooze.
* Date, year and day of week display. On/off selectable.
* Temperature display. F/C selectable. Offset adjustable. On/Off selectable.
* Display Auto-Dim with pre-programmed limits.
* Auto-increment when setting times/alarms/etc. No need to repeatedly press the same key.
* Initial screen content can be changed (programmatically).
* Default setting can be changed (programmatically).
* While setting, the highlight mode can be changed (programmatically).

## Things that could be implemented
* More straightforward menu.
* Better font faces.
* Creating missing font faces. Even accented chars if one needs.
* Multi language month name and day name.
* Better isolation of long press and short press. Identification of double (triple) press. Hook specific functions on the identified events.
* Etc.

## How to use
S1 (Top switch) long press changes to setting mode and back; short press (in setting mode) cycles
S2 (Bottom) cycles the screens in normal mode; changes the value in setting mode

### Programming with STC-ISP
Connect to the STC processor using the four pin header which hopefully you installed prior to soldering the four LED display in place. If your clock is already built and your desoldering skills and/or tools are not up to the task, you can solder the necessary header(s) using wire between the header(s) and the top of the board.

 ----------------------------
| P1 header | Serial adapter |
|-----------|----------------|
| 5V        | 5V             |
| GND       | GND            |
| P3.0      | TXD            |
| P3.1      | RXD            |
 ----------------------------

Once you're connected, you have your choice of either STC-ISP, which is a Windows application (runs fine on most VM's) or your can use [STCGAL](https://github.com/grigorig/stcgal) if you're comfy with Python. Once your tool is running, you'll need to load the .HEX output file into the processor. The STC-ISP tool is pretty straight forward, the process is:

* Select the correct processor type
* Select com port
* Open code file (main.hex)
* Select clock speed of 22.1184 MHz on the Hardware Option tab
* Click the Download/Program button
* Interrupt the +5 supply to the STC processor and restore

The last step above can be as easy as removing the connector to P3 and restoring. Or, if you're going to do additional development, a NC momentary push button in the +5 line and a diode and resistor in the RX/TX lines to prevent the serial adaptor signals from keeping the processor alive are required. The schematic for these connections is detailed in the STC technical document. Links below.

### Making changes
It is a simple matter to rebuild with the provided Makefile for SDCC. The Makefile originated with zerog2k's STC DIY-Clock project and I extended it with the additional file structure I created. In doing so, I found that there were several file inter dependencies that required a fair number of "make clean" followed by "make" commands so I added the ".phony" rule to just recompile everything per session since the compile and link times were insignificant. Better to wait three seconds for a complete rebuild than to waste twenty minutes on trying to figure out why the changes didn't appear in the code.

If you want to make any significant code changes, you'll probably wish you had some debugging capability. If you have the STC15W408AS part in your clock, you're in luck as this processor has a second timer and UART. The code already has initialization in place for this UART  if turned on in the global.h header file. SDCC supports a small footprint printf (printf_tiny) that only requires about 400 additional bytes of flash. The timer 2 default configuration is for 115200 baud, 8/1/N. On the Banggood board, the RX and TX have separate pins on a two pin connector, P3. Connection to a serial device is:

 ----------------------------
| P3 header | Serial adapter |
|-----------|----------------|
| P3.6      | TXD            |
| P3.7      | RXD            |
 ----------------------------

In order to pick up the GND connection, you'll need to parallel the main serial port on P1. The connections for P1 are detailed in the previous section.

## Program Assumptions
The cpu clock is set to run at 22.1184MHz. I haven't experimented with other clock speeds.

## Code notes and structures
The C code is formatted with using clang-format, except ds1302.c (where some of the asm parts became scrambled if formatting applied). Style file attached.

## Authors
* **Z Patocs** - *Initial work* - [patzol768](https://github.com/patzol768)

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Disclaimer
This code is provided as-is, with NO guarantees or liabilities as per the License. Use at your own risk.

Since the original firmware loaded on the STC processor cannot be copied, there is no duplicating what you originally received with your clock. If you want to retain this code for any reason, you must purchase another STC chip of the same type (or same base type with more flash if desired or needed). These are readily available on Aliexpress in small quantities as well as eBay although the prices are usually somewhat higher. Just be prepared to wait for them to arrive, especially from Aliexpress.

## Acknowledgments
* [aFewBits](https://github.com/aFewBits) for his STC DIY Clock work
* All the acknowledged sources mentioned in the original
* All the contributors to the SDCC tool set

### References
Main STC website(in Chinese - Google translate is your friend!), STCMCU.COM redirects here:
http://www.gxwmcu.com

STC15 series English datasheet:
http://www.stcmcu.com/datasheet/stc/STC-AD-PDF/STC15-English.pdf

Older STC15F204EA English datasheet:
http://gxwmcu.com/datasheet/stc/STC-AD-PDF/STC15F204EA-series-english.pdf

Other STC documents (English)
http://stcmicro.com/sjsc.html

SDCC User Guide (PDF):
http://sdcc.sourceforge.net/doc/sdccman.pdf

Maxim DS1302 datasheet:
http://datasheets.maximintegrated.com/en/ds/DS1302.pdf

Maxim DS1302 application note 3449 with example code for 8051 interfacing:
https://www.maximintegrated.com/en/app-notes/index.mvp/id/3449

