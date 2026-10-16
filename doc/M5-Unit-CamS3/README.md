# M5Stack Unit CamS3

Basic information:

- ESP32-S3 board with OV2640 2MP camera; board documentation and schematic are available [here](https://docs.m5stack.com/en/unit/Unit_CamS3).
- Install the Arduino IDE and libraries from the [main instructions](../../README.md#arduino_lib). In [mcu_cfg.h](../../ESP32_PrusaConnectCam/mcu_cfg.h), set **ESP32_M5_UNIT_CAMS3** to **true** and all other board definitions to **false**.
- Select **ESP32S3 Dev Module** and use the Tools settings in the [board configuration header](../../ESP32_PrusaConnectCam/module_ESP32-M5_Unit_CamS3.h), including **8MB Flash Size** and **OPI PSRAM**.
- For uploading, connect the **Grove2USB-C adapter** with power disconnected. Connect **G0** to **GND**, attach USB and remove the jumper when the serial port appears. Select the port, enable **Erase All Flash Before Sketch Upload** for the first upload or configuration recovery, then upload. Disable flash erasing for updates and restart with G0 disconnected.
- Follow the [Prusa Connect instructions](../../README.md#prusa_connect). Find the IP address in the router's client list and open `http://<camera-ip>/`; alternatively, use `http://prusa-esp32cam.local`. Snapshots are at `/saved-photo.jpg` and video at `/stream.mjpg`.
- Potential issue with this board: microSD is unverified, camera flash/DHT are disabled and the microphone is unsupported. Build from source; release builds and cloud updates do not include this board.
