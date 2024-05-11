# Instructions to use the PrusaConnectCam with the FreeNove ESP32-WROVER 

The Freenove ESP32-WROVER is an alternative option to the ESP32 Cam.  It is available at 
[https://store.freenove.com/products/fnk0060](https://store.freenove.com/products/fnk0060) as
well as places like Amazon and Ali Express.

The main difference between this board and the ESP32-Cam is what pins are used.  The steps below is everything you need to do, starting from scratch, to connect to set up and program the board.  

## Caveats

As of this writing, I have not tested putting an LED flash on the board, since I have a separate
lighting rig.  If someone confirms that it works, this README can be updated.

## Install tools and set up the programming environment

The FreeNove documentation for the board is excellent.  You can find it 
[here](https://github.com/Freenove/Freenove_ESP32_WROVER_Board/tree/main), especially the 
[PDF C Tutorial](https://github.com/Freenove/Freenove_ESP32_WROVER_Board/blob/main/C/C_Tutorial.pdf).  

Make sure you do the following things from the FreeNove C tutorial.  Note that some of these things
overlap with the main README instructions, but this is a full set of steps that works for a new
board and a new environment.

1. Page 5 - Install the CH340 Drivers if you don't already have them.
2. Page 15 - Install the Arduino IDE.
3. Page 18 - Set the Preferences > Additional Board Manager URLS and add 
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
4. Page 19 - Install the `esp32 tools` in the Arduino board manager
5. Page 21 - Select the `esp32 Wrover Module` from the available boards 
6. Page 48 - Format the included SD card.  This step is optional.  Formatting and
   installing the card will allow logs to be written to the card.
  
Install the additional libraries as per the main readme: 
[ here ](README.md#arduino_lib) <-- Note: For some reason, this link doesn't always take you 
directly to the right place in the main readme.  If not, go to [README.md](README.md), find the 
`What we need for functionality` section, and follow the link under 
`Install the necessary libraries in the Arduino IDE here`.

## Get the code and configure it for the WRover board

Clone this repository.  In the Arduino editor, open the file 
`ESP32_PrusaConnectCam/ESP32_PrusaConnectCam.ino`.

You need to edit two files:

- `ESP32_PrusaConnectCam/mcu_cfg.h`
- `ESP32_PrusaConnectCam/Camera_cfg.h`

In each file, uncomment the line:

``` C++
// #define USE_ESP32_WROVER 1
```

to make it:

``` C++
#define USE_ESP32_WROVER 1
```

This will change the pinouts from those that match the ESP32-Cam to those that match 
the Freenove WRover board.  Here is a summary of the changes:

- Change the GPIOs used to connect to the camera
- Change the GPIO used for the status LED
- Change the GPIO used for the optional LED flash to GPIO32

## Program the board

Note that the programming configuration is **NOT** the same as the main README.

1. Install the 1GB flash card included with the Freenove kit into the SD card slot.  This is
   optional, but doing so allows the board to write logs to it.
2. Use a USB-C cable to plug the USB port on the board into your computer.
3. Under the `Tools` menu in the Arduino IDE, check the following settings:
   - `Board`:  `ESP32 WRover Module`
   - `Port`: choose the COM port for the board
   - `Erase All Flash before sketch upload` : Set this to `enable` **the first time you program the board**.  The set it to disable.  If you don't, it will erase the saved settings each time you program the board.
   - `Flash Frequency`: `80 MHz`
   - `Flash Mode`: `QIO`
   - `Partition Scheme`: `Huge App (3MB No OTA/1MB SPIFFS)`
   - `Upload Speed`: `115200`
4. Hit the green Arrow button at the top left of the IDE to start programming the board.
5. While the board is programming, go to `Tools > Serial Monitor` and set the serial monitor for `115200`.  
6. Once the board is programmed, open the serial monitor tab.  You should see everything working.  If you see a message about a periodic reset, then this probably means the pin configuration is wrong.  In that case, double check that you   make sure you have uncommented the `#define USE_ESP32_WROVER 1` line in BOTH files mentioned above.

## Connecting to the board

The first time you program it, the board makes its own access point.  You can connect your computer to it, then visit `http://192.168.0.1` to set the WIFI name and password.

Once it has WIFI credentials, the board will reset and you can find its IP address in your router's DHCP table.  I recommend reserving an IP for it in the router configuration so you can always find it.

When you connect to the camera's IP from your local network, you can configure the camera parameters (image size, brightness, etc) and add the Prusa token for your Prusa Connect configuration.

- Navigate to [connect.prusa3D.com](https://connect.prusa3D.com)
- Open up the `printer details` page for the printer
- Open up `Cameras` page.
- Click `+ ADD NEW CAMERA`
- You may have to refresh the page, but an "Unnamed Camera" option will appear.  
- Copy the Token for the unnamed camera and input it on the camera's configuration page.

If everything is working, you should see shots from your camera showing up in Prusa Connect.  If
not, look at the serial monitor output to see if there are errors reported.

## Installing the board in the enclosure

There are several options to mount the board

- [2 axis pivot for mounting on the enclosure wall](https://www.printables.com/model/876390)
- [This remixed arm for mounting the camera on the Z axis carriage](https://www.printables.com/model/791446-freenove-esp32-wrover-cam-mount-prusa-mk3-mk4)
