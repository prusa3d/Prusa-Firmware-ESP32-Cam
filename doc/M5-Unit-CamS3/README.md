# M5Stack Unit CamS3

For the [Unit CamS3 (U174)](https://docs.m5stack.com/en/unit/Unit_CamS3) with OV2640. Verified on firmware **1.1.2**: web interface, JPEG snapshots, MJPEG video and successful Prusa Connect photo uploads.

1. Install the [Arduino prerequisites](../../README.md#arduino_lib), open `ESP32_PrusaConnectCam.ino`, and set only `ESP32_M5_UNIT_CAMS3` to `true` in [mcu_cfg.h](../../ESP32_PrusaConnectCam/mcu_cfg.h); set all other board flags to `false`.
2. Select **ESP32S3 Dev Module** and use the [board header's Tools settings](../../ESP32_PrusaConnectCam/module_ESP32-M5_Unit_CamS3.h), including **8MB flash** and **OPI PSRAM**.
3. With power disconnected, connect a **Grove2USB-C adapter** and jumper **G0 to GND**; attach USB, then remove the jumper once the serial port appears.
4. Select the port and upload. Enable **Erase All Flash Before Sketch Upload** for the first upload or recovery (clears saved settings); disable it for updates. Power-cycle with G0 disconnected.
5. Follow the [Wi-Fi and Prusa Connect setup](../../README.md#prusa_connect). Find the camera's IP in your router's client list (it may appear as `esp32s3-...`) and open `http://<camera-ip>/`; alternatively, try `http://prusa-esp32cam.local` where mDNS works.
6. View snapshots at `http://<camera-ip>/saved-photo.jpg` and video at `http://<camera-ip>/stream.mjpg`. microSD is unverified; camera flash/DHT are disabled and the microphone is unsupported. Build from source; release builds and cloud updates do not include this board.
