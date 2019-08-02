# Memphis Controller
Arkathon Project Memphis Controller Software
Arduino Framework based Skeleton Embedded Application, powered by [PlatformIO](http://platformio.org "Cross-platform build system")

This project is created for an [Adafruit Huzzah ESP8266](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout) controller module.

## Purpose
This project is the Fun Heartbeat animated GIF T-Shirt controller Firmware.
It is designed and tested on an Adafruit Adafruit Huzzah ESP8266 Microcontroller Unit. It could be made running on an Arduino Mega 2560 as well.

An animated GIF converted into RGB pixel information available in a 2-dimensional array (16x16 pixel) is sent out to a 16x16 Neo Pixel Matrix. 

The current branch (*refactor-showcase*) is designed for stability, so no network connectivity (no WiFi, no Thingspeak uplink) available. Furthermore the Pulse Heartbeat Sensor is not connected and a fictive heartbeat rate progression is simulated (HBR modulating between 62 and 91 beats per minute, changing every 2 seconds).

## Hardware
The Hardware design is shown below:
![BumpinHeart_Steckplatine](C:\git\pio-prj\memphis-controller\doc\design\hw\BumpinHeart_Steckplatine.png)


## Toolchain
[PlatformIO](http://platformio.org "Cross-platform build system") is the ideal foundation when developing IoT devices. It supports cross-platform development for many different controller boards widely used in the maker field and also for industrial applications.

### Installation
#### Python 2.7

**Windows**

* download Python 2.7.x from: https://www.python.org/downloads/
* install for all users
* select destinationdirectory (keep default): C:\Python27\
* add python.exe to path

**Linux**

Install Python 2.7 using your package manager.

#### PlatformIO
Install PlatformIO using the Python Package Manager
(see also http://docs.platformio.org/en/latest/installation.html#python-package-manager):

* in a cmd shell enter: `pip install -U platformio`
* upgrade pip, in the cmd shell enter: `python -m pip install --upgrade pip`
* upgrade PlatformIO, in the cmd shell enter: `pio upgrade`


## How to build for Eclipse CDT
  1. Create a directory where your Eclipse Workspace will be stored and where this project shall be cloned into. E.g. `C:\git`
  2. Clone the repository - [memphis-controller](https://github.com/ERNICommunity/memphis-controller) - into the folder you created before, `git clone git@github.com:ERNICommunity/dust-measurement-network.git`
  3. Open a command shell in the just cloned project folder and dive into `detector` (one level down), i.e. in `C:\git\memphis-controller`
  4. Run the command `pio init --ide eclipse`, this prepares the project to be edited and built using Eclipse CDT
  5. Run the command `pio run`, this starts the project build in the shell

## Open project in Eclipse CDT
  1. Open Eclipse CDT, as workspace choose the folder you created before, i.e `C:\git\pio-prj`
  2. Import the project with File->Import->General->Existing Projects into Workspace, choose the `memphis-controller` (i.e `C:\git\pio-prj\memphis-controller`)

## Connect Terminal Emulation
In order to test and run the CLI commands, a terminal emulation program shall be used. The one giving you the best experience will be the [HTerm](http://www.der-hammer.info/terminal/). 

Load the _hterm-com10.cfg_ file to configure HTerm properly. Alter the COM10 accordingly to the one that has been selected on your computer.
Important setting in HTERM config for Status lines: DTR & RTS shall be set to true.

## Debug Features
### Debug CLI Commands
#### Command Tree
     dbg                      Debug CLI root node
       tr                     Debug Trace Port config access
         heap                 Particular Trace Port (heap: see below)
           out                Trace Output cfg access for current port
             get              Show the assigned Trace Output
             set <outName>    Set a particular Trace Output name
             list             Show all available Trace Output names (and the currently selected)
           lvl                Trace Level config access for the current trace port
             get              Show the current trace level
             set <level>      Set a particular trace level
             list             Show all available trace levels (and the currenntly selected)

#### Example commands
* `dbg tr heap lvl set debug`


### Trace Port

|Trace Port|default level|functionality|
|----------|-------------|:------------|
|heap|info|if set to debug level: automatically print free heap memory [bytes], every 10 seconds|

## Library Usage
This chapter lists all the libraries this project is using.

### PlatformIO Libraries

|ID|Name|URL|Description|
|:---|:------------|:----------------|:-----------------------|
|13|Adafruit GFX Library|https://github.com/adafruit/Adafruit-GFX-Library|Adafruit GFX graphics core library, this is the 'core' class that all our other graphics libraries derive from: [https://learn.adafruit.com/adafruit-gfx-graphics-library](https://learn.adafruit.com/adafruit-gfx-graphics-library)|
|215|Adafruit NeoMatrix|https://github.com/adafruit/Adafruit_NeoMatrix|Adafruit_GFX-compatible library for NeoPixel grids|
|28|Adafruit NeoPixel|https://github.com/adafruit/Adafruit_NeoPixel|Arduino library for controlling single-wire LED pixels (NeoPixel, WS2812, etc.)|
| 173|SerialCommand|https://github.com/kroimon/Arduino-SerialCommand|A Wiring/Arduino library to tokenize and parse commands received over a serial port.|
|1699|wiring-timer |https://github.com/dniklaus/wiring-timer|Universal recurring or non-recurring Timer. <br />Configurable timer to schedule events without having to use Arduino delay() function; helps to improve your application's architecture by encapsulating the timers into your components and thus make them active.|
|1716|debug-cli|https://github.com/ERNICommunity/debug-cli|Debug CLI for Embedded Applications - Command Line  Interface for debugging and testing based on object oriented tree structure.|
|1717|dbg-trace|https://github.com/ERNICommunity/dbg-trace|Debug Trace component for Embedded Applications - Debug and Trace Log message system based on trace ports with adjustable levels.|



### Homebrew Libraries

|Name|URL|Description|
|:------|:---------------------|:-------------------------------|
|RamUtils|https://github.com/dniklaus/arduino-utils-mem|Arduino Memory Utilities - helps to determine the free Ram that is currently available|
|App-Debug  |https://github.com/dniklaus/wiring-app-debug.git|Wiring application debug setup component - boilerplate code setting up all the debug environment such as CLI and Tracing and free RAM info printer                                                                                         |
|Battery |https://github.com/dniklaus/Battery|LiPo Battery Voltage Surveillance component |

