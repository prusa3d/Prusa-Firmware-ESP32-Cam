/**
   @file camera.h

   @brief Library for working with a camera

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "cfg.h"

#include "Camera_cfg.h"
#include "Arduino.h"
#include "mcu_cfg.h"
#include "var.h"
#include "log.h"

class Camera {
private:
  uint8_t PhotoQuality;      ///< photo quality
  uint8_t FrameSize;         ///< framesize
  int8_t brightness;         ///< brightness
  int8_t contrast;           ///< contrast
  int8_t saturation;         ///< saturation
  bool awb;                  ///< automatic white balancing
  bool awb_gain;             ///< automatic white balancing gain
  uint8_t wb_mode;           ///< white balancing mode
  bool aec2;                 ///< automatic exposition controll
  int8_t ae_level;           ///< automatic exposition level
  uint16_t aec_value;        ///< automatic exposition time
  bool gain_ctrl;            ///< automatic gain
  uint8_t agc_gain;          ///< automatic gain level
  bool bpc;                  ///< bad pixel correction
  bool wpc;                  ///< white pixel correction
  bool raw_gama;             ///< raw gama correction
  bool hmirror;              ///< horizontal mirror
  bool vflip;                ///< vertical flip
  bool lensc;                ///< lens corection
  bool exposure_ctrl;        ///< exposure control
  bool CameraFlashEnable;    ///< enable/disable camera flash function
  uint16_t CameraFlashTime;  ///< camera fash duration time
  uint8_t CameraFlashPin;    ///< GPIO pin for LED
  framesize_t TFrameSize;    ///< framesize_t type for camera module

  /* OV2640 camera module pinout and cfg*/
  camera_config_t CameraConfig;             ///< camera configuration
  camera_fb_t *FrameBuffer;                 ///< frame buffer
  String Photo;                             ///< photo in string format
  bool StreamOnOff;                         ///< stream on/off
  SemaphoreHandle_t frameBufferSemaphore;   ///< semaphore for frame buffer
  float StreamAverageFps;                   ///< stream average fps
  uint16_t StreamAverageSize;               ///< stream average size

  Configuration *config;                    ///< pointer to Configuration object
  Logs *log;                                ///< pointer to Logs object

  void InitCameraModule();

public:
  Camera(Configuration*, Logs*, uint8_t);
  ~Camera(){};
  void Init();
  void ApplyCameraCfg();
  void LoadCameraCfgFromEeprom();
  void ReinitCameraModule();
  void CapturePhoto();
  void CaptureStream(camera_fb_t *);
  void CaptureReturnFrameBuffer();
  void SetStreamStatus(bool);
  bool GetStreamStatus();

  void StreamSetFrameSize(uint16_t);
  void StreamSetFrameFps(float);
  uint16_t StreamGetFrameAverageSize();
  float StreamGetFrameAverageFps();
  void StreamClearFrameData();
  
  void CopyPhoto(camera_fb_t *);
  void CopyPhoto(String*);
  String GetPhoto();
  camera_fb_t *GetPhotoFb();
  framesize_t TransformFrameSizeDataType(uint8_t);
  
  void SetFlashStatus(bool);
  bool GetFlashStatus();

  void SetPhotoQuality(uint8_t);
  void SetFrameSize(uint8_t);
  void SetBrightness(int8_t);
  void SetContrast(int8_t);
  void SetSaturation(int8_t);
  void SetAwb(bool);
  void SetAwbGain(bool);
  void SetAwbMode(uint8_t);
  void SetAec2(bool);
  void SetAeLevel(int8_t);
  void SetAecValue(uint16_t);
  void SetGainCtrl(bool);
  void SetAgcGain(uint8_t);
  void SetBpc(bool);
  void SetWpc(bool);
  void SetRawGama(bool);
  void SetHMirror(bool);
  void SetVFlip(bool);
  void SetLensC(bool);
  void SetExposureCtrl(bool);
  void SetCameraFlashEnable(bool);
  void SetCameraFlashTime(uint16_t);

  uint8_t GetPhotoQuality();
  uint8_t GetFrameSize();
  uint16_t GetFrameSizeWidth();
  uint16_t GetFrameSizeHeight();
  int8_t GetBrightness();
  int8_t GetContrast();
  int8_t GetSaturation();
  bool GetAwb();
  bool GetAwbGain();
  uint8_t GetAwbMode();
  bool GetAec2();
  int8_t GetAeLevel();
  uint16_t GetAecValue();
  bool GetGainCtrl();
  uint8_t GetAgcGaint();
  bool GetBpc();
  bool GetWpc();
  bool GetRawGama();
  bool GetHMirror();
  bool GetVFlip();
  bool GetLensC();
  bool GetExposureCtrl();
  bool GetCameraFlashEnable();
  uint16_t GetCameraFlashTime();
};

extern Camera SystemCamera; ///< Camera object

#endif

/* EOF */