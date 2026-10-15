# M5Stack Unit CamS3

This guide covers the **Unit CamS3 (U174)** with the **OV2640 2MP camera**, an ESP32-S3-WROOM-1-N16R8, 16MB flash and 8MB PSRAM. The Unit CamS3-5MP is a different camera variant and is not covered by this configuration. See the [M5Stack board documentation and schematic](https://docs.m5stack.com/en/unit/Unit_CamS3) for identification and pin locations.

- [Build the firmware](#build-the-firmware)
- [Connect and upload](#connect-and-upload)
- [Connect to Prusa Connect](#connect-to-prusa-connect)
- [Status LED](#status-led)
- [Reset configuration](#reset-configuration)
- [Limitations and updates](#limitations-and-updates)

## Build the firmware

Install the Arduino IDE, ESP32 board support and libraries listed in the [main build instructions](../../README.md#arduino_lib). Open `ESP32_PrusaConnectCam/ESP32_PrusaConnectCam.ino`.

In [mcu_cfg.h](../../ESP32_PrusaConnectCam/mcu_cfg.h), enable only the M5Stack board:

```c
#define AI_THINKER_ESP32_CAM            false
#define ESP32_WROVER_DEV                false
#define CAMERA_MODEL_ESP32_S3_DEV_CAM   false
#define CAMERA_MODEL_ESP32_S3_EYE_2_2   false
#define CAMERA_MODEL_XIAO_ESP32_S3_CAM  false
#define CAMERA_MODEL_ESP32_S3_CAM       false
#define ESP32_S3_WROOM_FREENOVE         false
#define ESP32_M5_UNIT_CAMS3             true
```

Use the following **Tools** settings, matching [the board configuration header](../../ESP32_PrusaConnectCam/module_ESP32-M5_Unit_CamS3.h):

| Setting | Value |
|---------|-------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Disabled |
| CPU Frequency | 240MHz (WiFi/BT) |
| Core Debug Level | None |
| USB DFU On Boot | Disabled |
| Erase All Flash Before Sketch Upload | Enabled for the first upload; Disabled for later updates |
| Events Run On | Core 0 |
| Flash Mode | QIO 80MHz |
| Flash Size | 8MB |
| JTAG Adapter | Disabled |
| Arduino Runs On | Core 0 |
| USB Firmware MSC On Boot | Disabled |
| Partition Scheme | Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS) |
| PSRAM | OPI PSRAM |
| Upload Mode | UART0 / Hardware CDC |
| Upload Speed | 921600 |
| USB Mode | Hardware CDC and JTAG |
| Zigbee Mode | Disabled |

The physical board has 16MB flash; the **8MB** setting above follows the current firmware configuration. It does not use all of the physical flash.

## Connect and upload

You need a **Grove2USB-C adapter**, a Grove cable, a USB data cable and a jumper wire. The Grove port exposes the ESP32-S3's native USB pins; use the [M5Stack Grove2USB-C adapter](https://docs.m5stack.com/en/accessory/Grove_to_USBC) for the connection described here.

1. Disconnect power. Connect the camera's Grove port to the Grove2USB-C adapter with the Grove cable.
2. On the camera's exposed pin header, connect **G0 (GPIO0)** to **GND** with the jumper wire.
3. Connect the adapter to your computer with the USB data cable. The board starts in download mode, as described in [M5Stack's programming instructions](https://docs.m5stack.com/en/unit/Unit_CamS3#learn).
4. Once the USB serial port appears, remove the G0-to-GND jumper. Select that port under **Tools -> Port**, then use **Sketch -> Upload**.
5. After the upload finishes, disconnect and reconnect USB power with **G0 disconnected from GND** to start the camera firmware.

For subsequent uploads, set **Erase All Flash Before Sketch Upload** to **Disabled** to preserve the Wi-Fi and camera settings. Repeat the download-mode procedure if the uploader cannot connect. If uploading at 921600 fails, try 115200.

## Connect to Prusa Connect

Keep the camera powered through the Grove2USB-C adapter. Follow the [Prusa Connect setup instructions](../../README.md#prusa_connect) to obtain a camera token and configure the firmware.

On a fresh configuration, connect to the camera's **ESP32_camera_UID** Wi-Fi network with password **12345678**, then open **http://192.168.0.1**. Enter your Wi-Fi details and Prusa Connect camera token, save them, and set the image resolution and trigger interval.

After joining your Wi-Fi, use the camera's assigned IP address or **http://prusa-esp32cam.local** to access it. See [Service AP](../../README.md#service_ap) if you need to reconnect to its setup network.

## Status LED

The onboard programmable LED is connected to **GPIO14**. Its blinking patterns are described in the [main status LED guide](../../README.md#status_led).

## Reset configuration

The current board configuration assigns **GPIO0** to configuration reset. However, holding this pin low at power-on selects the ESP32-S3's [ROM download mode](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/boot-mode-selection.html#gpio0). Following the generic reset procedure with G0 grounded at power-on therefore does not reset the camera settings on this board.

To recover a fresh configuration, repeat the Arduino upload procedure with **Erase All Flash Before Sketch Upload -> Enabled**. This removes the saved Wi-Fi details, camera token and other settings. After uploading, return the option to **Disabled**, remove the jumper, restart the camera and configure it again.

## Limitations and updates

- The microSD slot is present and enabled in the board configuration, but microSD logging and time-lapse operation still need verification on this board.
- Camera flash and the DHT temperature sensor are disabled in this configuration. The onboard microphone is not supported by this firmware.
- `fw_build.sh` does not build this board yet, so it does not produce a CamS3 release archive or the `ESP32-M5-Unit-CamS3.bin` file expected by cloud updates. Build from source using the settings above.
- For a manual OTA update, upload the **application binary** from a CamS3 build through the web interface's **System** tab. Use the same flash and partition settings; do not upload a bootloader, partition table or merged flash image as an OTA update.
