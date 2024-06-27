/**
   @file camera.cpp

   @brief Library for working with a camera

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   https://github.com/espressif/esp32-camera


   @bug: no know bug
*/
#include "camera.h"

Camera SystemCamera(&SystemConfig, &SystemLog, FLASH_GPIO_NUM);

/**
   @brief Init camera constructor
   @param Configuration* - pointer to Configuration class
   @param Logs* - pointer to Logs class
   @param uint8_t - flash pin
   @return none
*/
Camera::Camera(Configuration* i_conf, Logs* i_log, uint8_t i_FlashPin) {
  config = i_conf;
  log = i_log;

  CameraFlashPin = i_FlashPin;
  StreamOnOff = false;
  frameBufferSemaphore = xSemaphoreCreateMutex();
  FrameBufferDuplicate = (camera_fb_t*)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM);
  FrameBufferExif = (camera_fb_t*)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM);
  StreamSendingPhoto = false;

  PhotoExifData.header = NULL;
  PhotoExifData.len = 0;
  PhotoExifData.offset = 0;
  PhotoSending = false;
  CameraCaptureFailedCounter = 0;
}

/**
   @brief Init the camera module and set the camera configuration
   @param none
   @return none
*/
void Camera::Init() {
  log->AddEvent(LogLevel_Info, F("Init camera lib"));

#if (true == ENABLE_CAMERA_FLASH)
  log->AddEvent(LogLevel_Info, F("Init GPIO"));
  ledcAttach(FLASH_GPIO_NUM, FLASH_PWM_FREQ, FLASH_PWM_RESOLUTION);
  SetFlashStatus(false);
#endif

  InitCameraModule();
  ApplyCameraCfg();
  GetCameraModel();
}

/**
   @brief Init the camera module
   @param none
   @return none
*/
void Camera::InitCameraModule() {
  log->AddEvent(LogLevel_Info, F("Init camera module"));
  /* Turn-off the 'brownout detector' */
#if (true == ENABLE_BROWN_OUT_DETECTION)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif
  esp_err_t err;

  CameraConfig.ledc_channel = LEDC_CHANNEL_0;
  CameraConfig.ledc_timer = LEDC_TIMER_0;
  CameraConfig.pin_d0 = Y2_GPIO_NUM;
  CameraConfig.pin_d1 = Y3_GPIO_NUM;
  CameraConfig.pin_d2 = Y4_GPIO_NUM;
  CameraConfig.pin_d3 = Y5_GPIO_NUM;
  CameraConfig.pin_d4 = Y6_GPIO_NUM;
  CameraConfig.pin_d5 = Y7_GPIO_NUM;
  CameraConfig.pin_d6 = Y8_GPIO_NUM;
  CameraConfig.pin_d7 = Y9_GPIO_NUM;
  CameraConfig.pin_xclk = XCLK_GPIO_NUM;
  CameraConfig.pin_pclk = PCLK_GPIO_NUM;
  CameraConfig.pin_vsync = VSYNC_GPIO_NUM;
  CameraConfig.pin_href = HREF_GPIO_NUM;
  CameraConfig.pin_sccb_sda = SIOD_GPIO_NUM;
  CameraConfig.pin_sccb_scl = SIOC_GPIO_NUM;
  CameraConfig.pin_pwdn = PWDN_GPIO_NUM;
  CameraConfig.pin_reset = RESET_GPIO_NUM;
  CameraConfig.xclk_freq_hz = 15000000;       // or 3000000; 16500000; 20000000
  CameraConfig.pixel_format = PIXFORMAT_JPEG; /* YUV422,GRAYSCALE,RGB565,JPEG */

  /* OV2640
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
    FRAMESIZE_UXGA (1600 x 1200)

    CAMERA_GRAB_WHEN_EMPTY - Fills buffers when they are empty. Less resources but first 'fb_count' frames might be old
    CAMERA_GRAB_LATEST     - Except when 1 frame buffer is used, queue will always contain the last 'fb_count' frames
  */

  CameraConfig.frame_size = TFrameSize;             /* FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA */
  CameraConfig.jpeg_quality = PhotoQuality;         /* 10-63 lower number means higher quality */
  CameraConfig.fb_count = 1;                        /* picture frame buffer alocation */
  CameraConfig.grab_mode = CAMERA_GRAB_LATEST;      /* CAMERA_GRAB_WHEN_EMPTY or CAMERA_GRAB_LATEST */
#if (true == ENABLE_PSRAM)
  CameraConfig.fb_location = CAMERA_FB_IN_PSRAM;    /* CAMERA_FB_IN_PSRAM or CAMERA_FB_IN_DRAM  */
#else
  CameraConfig.fb_location = CAMERA_FB_IN_DRAM;     /* CAMERA_FB_IN_PSRAM or CAMERA_FB_IN_DRAM  */
#endif
  
  if (CameraConfig.fb_location == CAMERA_FB_IN_DRAM) {
    log->AddEvent(LogLevel_Verbose, F("Camera frame buffer location: DRAM"));
  } else if (CameraConfig.fb_location == CAMERA_FB_IN_PSRAM) {
    log->AddEvent(LogLevel_Verbose, F("Camera frame buffer location: PSRAM"));
  } else {
    log->AddEvent(LogLevel_Verbose, F("Camera frame buffer location: Unknown"));
  }

  /* Camera init */
  err = esp_camera_init(&CameraConfig);
  if (err != ESP_OK) {
    log->AddEvent(LogLevel_Warning, F("Camera init failed. Error: "), String(err, HEX));
    log->AddEvent(LogLevel_Warning, F("Reset ESP32-cam!"));
    ESP.restart();
  } 
}

/**
   @brief Load camera CFG from EEPROM
   @param none
   @return none
*/
void Camera::LoadCameraCfgFromEeprom() {
  log->AddEvent(LogLevel_Info, F("Load camera CFG from EEPROM"));
  PhotoQuality = config->LoadPhotoQuality();
  FrameSize = config->LoadFrameSize();
  TFrameSize = TransformFrameSizeDataType(config->LoadFrameSize());
  brightness = config->LoadBrightness();
  contrast = config->LoadContrast();
  saturation = config->LoadSaturation();
  awb = config->LoadAwb();
  awb_gain = config->LoadAwbGain();
  wb_mode = config->LoadAwbMode();
  aec2 = config->LoadAec2();
  ae_level = config->LoadAeLevel();
  aec_value = config->LoadAecValue();
  gain_ctrl = config->LoadGainCtrl();
  agc_gain = config->LoadAgcGain();
  bpc = config->LoadBpc();
  wpc = config->LoadWpc();
  raw_gama = config->LoadRawGama();
  hmirror = config->LoadHmirror();
  vflip = config->LoadVflip();
  lensc = config->LoadLensCorrect();
  exposure_ctrl = config->LoadExposureCtrl();
  CameraFlashEnable = config->LoadCameraFlashEnable();
  CameraFlashTime = config->LoadCameraFlashTime();
  imageExifRotation = config->LoadCameraImageExifRotation();
}

/**
   @brief transform uint8_t from web interface to framesize_t
   @param uint8_t - int value from web
   @return framesize_t
*/
framesize_t Camera::TransformFrameSizeDataType(uint8_t i_data) {
  framesize_t ret = FRAMESIZE_QVGA;
  switch (i_data) {
    case 0:
      ret = FRAMESIZE_QVGA;
      break;
    case 1:
      ret = FRAMESIZE_CIF;
      break;
    case 2:
      ret = FRAMESIZE_VGA;
      break;
    case 3:
      ret = FRAMESIZE_SVGA;
      break;
    case 4:
      ret = FRAMESIZE_XGA;
      break;
    case 5:
      ret = FRAMESIZE_SXGA;
      break;
    case 6:
      ret = FRAMESIZE_UXGA;
      break;
    default:
      ret = FRAMESIZE_QVGA;
      log->AddEvent(LogLevel_Warning, F("Bad frame size. Set default value. "), String(i_data));
      break;
  }

  return ret;
}

/**
   @brief Function set photo sending status
   @param bool i_data - true = on, false = off
   @return none
*/
void Camera::SetPhotoSending(bool i_data) {
  PhotoSending = i_data;
}

/**
   @brief Function set flash status
   @param bool i_data - true = on, false = off
   @return none
*/
void Camera::SetFlashStatus(bool i_data) {
#if (true == ENABLE_CAMERA_FLASH)
  if (true == i_data) {
    ledcWrite(FLASH_GPIO_NUM, FLASH_ON_STATUS);
  } else if (false == i_data) {
    ledcWrite(FLASH_GPIO_NUM, FLASH_OFF_STATUS);
  }
#endif
}

/**
   @brief Function get flash status
   @param none
   @return bool - true = on, false = off
*/
bool Camera::GetFlashStatus() {
  if (ledcRead(FLASH_GPIO_NUM) == FLASH_OFF_STATUS) {
    return false;
  } else if (ledcRead(FLASH_GPIO_NUM) == FLASH_ON_STATUS) {
    return true;
  }

  return false;
}

/**
   @brief Function set camer acfg
   @param none
   @return none
*/
void Camera::ApplyCameraCfg() {
  log->AddEvent(LogLevel_Info, F("Set camera CFG"));

  /* sensor configuration */
  sensor = esp_camera_sensor_get();
  sensor->set_brightness(sensor, brightness);         // -2 to 2
  sensor->set_contrast(sensor, contrast);             // -2 to 2
  sensor->set_saturation(sensor, saturation);         // -2 to 2
  sensor->set_special_effect(sensor, 0);              // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  sensor->set_whitebal(sensor, awb);                  // automatic white balancing 0 = disable , 1 = enable
  sensor->set_awb_gain(sensor, awb_gain);             // automatic white balancing gain 0 = disable , 1 = enable
  sensor->set_wb_mode(sensor, wb_mode);               // white balancing mode 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  sensor->set_exposure_ctrl(sensor, exposure_ctrl);   // exposition controll 0 = disable , 1 = enable
  sensor->set_aec2(sensor, aec2);                     // enable exposition controll 0 = disable , 1 = enable
  sensor->set_ae_level(sensor, ae_level);             // automatic exposition level -2 to 2
  sensor->set_aec_value(sensor, aec_value);           // expozition time - 0 to 1200
  sensor->set_gain_ctrl(sensor, gain_ctrl);           // automatic gain controll 0 = disable , 1 = enable
  sensor->set_agc_gain(sensor, agc_gain);             // automatic gain controll level 0 to 30
  sensor->set_gainceiling(sensor, (gainceiling_t)0);  // maximum gain 0 to 6
  sensor->set_bpc(sensor, bpc);                       // bad pixel correction 0 = disable , 1 = enable
  sensor->set_wpc(sensor, wpc);                       // white pixel correction 0 = disable , 1 = enable
  sensor->set_raw_gma(sensor, raw_gama);              // raw gama correction 0 = disable , 1 = enable
  sensor->set_lenc(sensor, lensc);                    // lens correction 0 = disable , 1 = enable
  sensor->set_hmirror(sensor, hmirror);               // horizontal mirror 0 = disable , 1 = enable
  sensor->set_vflip(sensor, vflip);                   // vertical flip 0 = disable , 1 = enable
  sensor->set_dcw(sensor, 1);                         // 0 = disable , 1 = enable
  sensor->set_colorbar(sensor, 0);                    // external collor lines, 0 = disable , 1 = enable
}

/**
   @brief Function for reinit camera module
   @param none
   @return none
*/
void Camera::ReinitCameraModule() {
  esp_err_t err = esp_camera_deinit();
  if (err != ESP_OK) {
    log->AddEvent(LogLevel_Warning, F("Camera error deinit camera module. Error: "), String(err, HEX));
  }
  delay(100);
  InitCameraModule();
  ApplyCameraCfg();
}

/**
   @brief Function for get camera model and type
   @param void
   @return none
*/
void Camera::GetCameraModel() {
  log->AddEvent(LogLevel_Info, F("Get camera model and type"));
  if (sensor == NULL) {
    log->AddEvent(LogLevel_Error, F("Camera sensor is NULL"));
    return;
  } 
  
  camera_sensor_info_t *info = esp_camera_sensor_get_info(&sensor->id);
  if (info == NULL) {
    log->AddEvent(LogLevel_Error, F("Camera sensor info is NULL"));
    return;
  }

  CameraType = (camera_pid_t) sensor->id.PID;
  CameraName = info->name;
  log->AddEvent(LogLevel_Info, F("Camera type: "), String(CameraType));
  log->AddEvent(LogLevel_Info, F("Camera name: "), String(CameraName));
  log->AddEvent(LogLevel_Info, F("Camera model: "), String(info->model));
  log->AddEvent(LogLevel_Info, F("Camera PID: "), String(info->pid));
  log->AddEvent(LogLevel_Info, F("Camera MAX framesize: "), String(info->max_size));
  log->AddEvent(LogLevel_Info, F("Camera support jpeg: "), String(info->support_jpeg));
}

/**
   @brief Capture Photo and Save it to string array
   @param none
   @return none
*/
void Camera::CapturePhoto() {

  /* Check if photo is already sending */
  if (true == PhotoSending) {
    log->AddEvent(LogLevel_Info, F("Sending photo"));
    return;
  }

  /* Check if stream is on */
  if (false == StreamOnOff) {
    if (!xSemaphoreTake(frameBufferSemaphore, portMAX_DELAY)) {
      log->AddEvent(LogLevel_Error, F("Failed to take frame buffer semaphore"));
      return;
    }

    CameraCaptureSuccess = false;
    /* check flash, and enable FLASH LED */
    if (true == CameraFlashEnable) {
      SetFlashStatus(true);
      delay(CameraFlashTime);
    }

    if (FrameBuffer) {
      esp_camera_fb_return(FrameBuffer);
    }

    /* Capturing a training photo. Without this sequence, the camera will not obtain the current photo but photo from the previous cycle. */
    FrameBuffer = esp_camera_fb_get();
    if (FrameBuffer) {
      esp_camera_fb_return(FrameBuffer);
      log->AddEvent(LogLevel_Verbose, F("Camera capture training photo"));
    } else {
      esp_camera_fb_return(FrameBuffer);
      log->AddEvent(LogLevel_Error, F("Camera capture failed training photo"));
      //ReinitCameraModule();
    }

    int attempts = 0;
    const int maxAttempts = 5;
    PhotoExifData.header = NULL;
    do {
      log->AddEvent(LogLevel_Info, F("Taking photo..."));

      FrameBuffer = esp_camera_fb_get();
      if (!FrameBuffer) {
        CameraCaptureFailedCounter++;
        log->AddEvent(LogLevel_Error, F("Camera capture failed! photo. Attempt: "), String(CameraCaptureFailedCounter));
        xSemaphoreGive(frameBufferSemaphore);  // Release semaphore before returning
        return;
      }

      char buf[150] = { '\0' };
      uint8_t ControlFlag = (uint8_t)FrameBuffer->buf[15];
      sprintf(buf, "The picture has been saved. Size: %d bytes, Photo resolution: %zu x %zu", FrameBuffer->len, FrameBuffer->width, FrameBuffer->height);
      log->AddEvent(LogLevel_Info, buf);

      if (ControlFlag != 0x00) {
        log->AddEvent(LogLevel_Error, "Camera capture failed! flag: " + String(ControlFlag, HEX));
        FrameBuffer->len = 0;

      } else {
        log->AddEvent(LogLevel_Info, F("Photo OK! "), String(ControlFlag, HEX));
        CameraCaptureFailedCounter = 0;

        /* generate exif header */
        update_exif_from_cfg(imageExifRotation);
        get_exif_header(FrameBuffer, &PhotoExifData.header, &PhotoExifData.len);
        PhotoExifData.offset = get_jpeg_data_offset(FrameBuffer);
        CameraCaptureSuccess = true;
        
        if (PhotoExifData.header != NULL) {
          log->AddEvent(LogLevel_Info, F("Exif header OK! Len: "), String(PhotoExifData.len));
        } else {
          log->AddEvent(LogLevel_Error, F("Exif header failed! "), String(PhotoExifData.len));
        }
      }

      attempts++;
      if (attempts >= maxAttempts) {
        log->AddEvent(LogLevel_Error, F("Failed to capture a valid photo after max attempts"));
        break;
      }
    } while (!(FrameBuffer->len > 100));

    /* Disable flash */
    if (true == CameraFlashEnable) {
      SetFlashStatus(false);
    }
    xSemaphoreGive(frameBufferSemaphore);

  } else {
    /* Stream is on, set flag for sending photo */
    if (xSemaphoreTake(frameBufferSemaphore, portMAX_DELAY)) {
      StreamSendingPhoto = true;
      xSemaphoreGive(frameBufferSemaphore);
    }
  }

  /* Reinit camera module if photo capture failed */
  if (CameraCaptureFailedCounter > CAMERA_MAX_FAIL_CAPTURE) { 
    log->AddEvent(LogLevel_Error, F("Camera capture failed! photo max attempts"));
    CameraCaptureFailedCounter = 0;
    ReinitCameraModule();
  }
}

/**
   @brief Capture Stream
   @param camera_fb_t * - pointer to camera_fb_t
   @return none
*/
void Camera::CaptureStream(camera_fb_t* i_buf) {
  if (xSemaphoreTake(frameBufferSemaphore, portMAX_DELAY)) {
    do {
      /* capture final photo */
      FrameBuffer = esp_camera_fb_get();
      if (!FrameBuffer) {
        log->AddEvent(LogLevel_Error, F("Camera capture failed! stream"));
        i_buf = NULL;
        return;
      }

      /* check if photo is correctly saved */
    } while (!(FrameBuffer->len > 100));

    /* check if the photo is rotated */
    bool ExifStatus = false;
#if false
    if (1 != imageExifRotation) { /* 1 = image rotation 0 degree */
      /* generate exif header */
      update_exif_from_cfg(imageExifRotation);
      get_exif_header(FrameBuffer, &PhotoExifData.header, &PhotoExifData.len);
      PhotoExifData.offset = get_jpeg_data_offset(FrameBuffer);
      if (PhotoExifData.header != NULL) {
        /* memory allocation release */
        if (FrameBufferExif != NULL) {
          if (FrameBufferExif->buf != NULL) {
            free(FrameBufferExif->buf);
            FrameBufferExif->buf = NULL; /* Set to NULL after freeing */
          }

          free(FrameBufferExif);
          FrameBufferExif = NULL; /* Set to NULL after freeing */
        }

        /* Allocate memory for the duplicate frame structure */
        FrameBufferExif = (camera_fb_t*)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM);

        /* Calculate the total size of the buffer */
        size_t totalSize = PhotoExifData.len + FrameBuffer->len - PhotoExifData.offset;

        /* Allocate memory for the image data */
        FrameBufferExif->buf = (uint8_t*)heap_caps_malloc(totalSize, MALLOC_CAP_SPIRAM);
        if (FrameBufferExif->buf == NULL) {
          log->AddEvent(LogLevel_Error, F("Failed to allocate memory for EXIF buffer"));
          return;
        }

        /* copy the EXIF data to the buffer */
        memcpy(FrameBufferExif->buf, PhotoExifData.header, PhotoExifData.len);

        /* copy the image data to the buffer */
        memcpy(FrameBufferExif->buf + PhotoExifData.len, FrameBuffer->buf + PhotoExifData.offset, FrameBuffer->len - PhotoExifData.offset);

        /* Set the length of the buffer */
        FrameBufferExif->len = totalSize;

        *i_buf = *FrameBufferExif;
        ExifStatus = true;
        
      } else {
        /* copy the frame buffer to the buffer */
        *i_buf = *FrameBuffer;
      }

    } else {
      /* copy the frame buffer to the buffer */
      *i_buf = *FrameBuffer;
    }
#else
    *i_buf = *FrameBuffer;
#endif

    /* copy the frame buffer to the duplicate frame buffer. For sending photo to Prusa Connect */
    if (false == StreamSendingPhoto) {
      /* memory allocation release */
      if (FrameBufferDuplicate != NULL) {
        if (FrameBufferDuplicate->buf != NULL) {
          free(FrameBufferDuplicate->buf);
          FrameBufferDuplicate->buf = NULL; /* Set to NULL after freeing */
        }

        free(FrameBufferDuplicate);
        FrameBufferDuplicate = NULL; /* Set to NULL after freeing */
      }
      
      /* Allocate memory for the duplicate frame structure */
      FrameBufferDuplicate = (camera_fb_t*)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM);

      /* Copy the metadata */
      if (true == ExifStatus) {
        memcpy(FrameBufferDuplicate, FrameBufferExif, sizeof(camera_fb_t));
      } else {
        memcpy(FrameBufferDuplicate, FrameBuffer, sizeof(camera_fb_t));
      }

      /* Allocate memory for the image data */
      if (true == ExifStatus) {
        FrameBufferDuplicate->buf = (uint8_t*)heap_caps_malloc(FrameBufferExif->len, MALLOC_CAP_SPIRAM);
      } else {
        FrameBufferDuplicate->buf = (uint8_t*)heap_caps_malloc(FrameBuffer->len, MALLOC_CAP_SPIRAM);
      }

      /* Check if memory allocation was successful */
      if (!FrameBufferDuplicate->buf) {
        /* Handle error */
        free(FrameBufferDuplicate);
        Serial.println("Failed to allocate memory for the duplicate frame buffer");
      } else {
        /* Copy the image data */
        if (true == ExifStatus) {
          memcpy(FrameBufferDuplicate->buf, FrameBufferExif->buf, FrameBufferExif->len);
          
        } else {
          memcpy(FrameBufferDuplicate->buf, FrameBuffer->buf, FrameBuffer->len);
        }
      }
    }

    xSemaphoreGive(frameBufferSemaphore);
  }
}

/**
   @brief Capture Return Frame Buffer
   @param none
   @return none
*/
void Camera::CaptureReturnFrameBuffer() {
  esp_camera_fb_return(FrameBuffer);
}

/**
   @brief Set Stream Status
   @param bool - true = on, false = off
   @return none
*/
void Camera::SetStreamStatus(bool i_status) {
  StreamOnOff = i_status;
  if (FrameBuffer) {
    esp_camera_fb_return(FrameBuffer);
  }
  log->AddEvent(LogLevel_Info, F("Camera video stream: "), String(StreamOnOff));
}

/**
   @brief Get Stream Status
   @param none
   @return bool - true = on, false = off
*/
bool Camera::GetStreamStatus() {
  return StreamOnOff;
}

bool Camera::GetCameraCaptureSuccess() {
  return CameraCaptureSuccess;
}

/**
   @brief Set Frame Size
   @param uint16_t - frame size
   @return none
*/
void Camera::StreamSetFrameSize(uint16_t i_data) {
  StreamAverageSize = (StreamAverageSize + i_data) / 2;
}

/**
   @brief Set average Frame Fps
   @param float - frame fps
   @return none
*/
void Camera::StreamSetFrameFps(float i_data) {
  StreamAverageFps = (StreamAverageFps + i_data) / 2.0;
}

/**
   @brief Get average stream Frame Size
   @param none
   @return uint16_t - frame size
*/
uint16_t Camera::StreamGetFrameAverageSize() {
  return StreamAverageSize;
}

/**
   @brief Get average stream Frame Fps
   @param none
   @return float - frame fps
*/
float Camera::StreamGetFrameAverageFps() {
  return StreamAverageFps;
}

/**
   @brief Clear Frame Data
   @param none
   @return none
*/
void Camera::StreamClearFrameData() {
  StreamAverageFps = 0.0;
  StreamAverageSize = 0;
}

/**
   @brief Set Sending Photo for Stream
   @param bool - sending photo
   @return none
*/
void Camera::StreamSetSendingPhoto(bool i_data) {
  StreamSendingPhoto = i_data;
}

/**
   @brief Get Photo
   @param none
   @return String - photo
*/
String Camera::GetPhoto() {
  Photo = "";
  for (size_t i = 0; i < FrameBuffer->len; i++) {
    Photo += (char)FrameBuffer->buf[i];
  }
  return Photo;
}

/**
   @brief Get Photo Frame Buffer
   @param none
   @return camera_fb_t* - photo frame buffer
*/
camera_fb_t* Camera::GetPhotoFb() {
  return FrameBuffer;
}

/**
   @brief Get Photo Frame Buffer Duplicate
   @param none
   @return camera_fb_t* - photo frame buffer duplicate
*/
camera_fb_t* Camera::GetPhotoFbDuplicate() {
  return FrameBufferDuplicate;
}

/**
   @brief Get Photo Exif Data
   @param none
   @return PhotoExifData_t* - photo exif data
*/
PhotoExifData_t* Camera::GetPhotoExifData() {
  return &PhotoExifData;
}

/**
   @brief Copy Photo
   @param camera_fb_t* - pointer to camera_fb_t
   @return none
*/
void Camera::CopyPhoto(camera_fb_t* i_data) {
  *i_data = *FrameBuffer;
}

/**
   @brief Copy Photo
   @param String* - pointer to string
   @return none
*/
void Camera::CopyPhoto(String* i_data) {
  Photo = "";
  for (size_t i = 0; i < FrameBuffer->len; i++) {
    Photo += (char)FrameBuffer->buf[i];
  }
  *i_data = Photo;
}

/**
 * @brief Copy photo from frame buffer to string array with range
 * 
 * @param i_data - pointer to string 
 * @param i_from - start index
 * @param i_to - end index
 */
void Camera::CopyPhoto(String* i_data, int i_from, int i_to) {
  *i_data = "";

  for (int i = i_from; i < i_to; i++) {
    *i_data += (char)FrameBuffer->buf[i];
  }
}

/**
 * @brief Copy photo from frame buffer to char array with range
 * 
 * @param i_data - pointer to char array
 * @param i_from - start index
 * @param i_to - end index
 */
void Camera::CopyPhoto(char* i_data, int i_from, int i_to) {
  int length = i_to - i_from;
  memcpy(i_data, FrameBuffer->buf + i_from, length);
  i_data[length + 1] = '\0';
}

/**
 * @brief Get Photo Size
 * 
 * @return int - photo size
 */
int Camera::GetPhotoSize() {
  log->AddEvent(LogLevel_Verbose, F("Photo size: "), String(FrameBuffer->len));
  return (int)FrameBuffer->len;
}

/**
   @brief Set Photo Quality
   @param uint8_t - photo quality
   @return none
*/
void Camera::SetPhotoQuality(uint8_t i_data) {
  config->SavePhotoQuality(i_data);
  PhotoQuality = i_data;
  ReinitCameraModule();
}

/**
   @brief Set Frame Size
   @param uint8_t - frame size
   @return none
*/
void Camera::SetFrameSize(uint8_t i_data) {
  config->SaveFrameSize(i_data);
  FrameSize = i_data;
  TFrameSize = TransformFrameSizeDataType(i_data);
  ReinitCameraModule();
}

/**
   @brief Set Brightness
   @param int8_t - brightness
   @return none
*/
void Camera::SetBrightness(int8_t i_data) {
  config->SaveBrightness(i_data);
  brightness = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Contrast
   @param int8_t - contrast
   @return none
*/
void Camera::SetContrast(int8_t i_data) {
  config->SaveContrast(i_data);
  contrast = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Saturation
   @param int8_t - saturation
   @return none
*/
void Camera::SetSaturation(int8_t i_data) {
  config->SaveSaturation(i_data);
  saturation = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic white balance
   @param bool - automatic white balance
   @return none
*/
void Camera::SetAwb(bool i_data) {
  config->SaveAwb(i_data);
  awb = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic white balance gain
   @param bool - automatic white balance gain
   @return none
*/
void Camera::SetAwbGain(bool i_data) {
  config->SaveAwbGain(i_data);
  awb_gain = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic white balance mode
   @param uint8_t - automatic white balance mode
   @return none
*/
void Camera::SetAwbMode(uint8_t i_data) {
  config->SaveAwbMode(i_data);
  wb_mode = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic exposure control 2
   @param bool - automatic exposure control 2
   @return none
*/
void Camera::SetAec2(bool i_data) {
  config->SaveAec2(i_data);
  aec2 = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic exposure level
   @param int8_t - automatic exposure level
   @return none
*/
void Camera::SetAeLevel(int8_t i_data) {
  config->SaveAeLevel(i_data);
  ae_level = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic exposition control value
   @param uint16_t - automatic exposure control value
   @return none
*/
void Camera::SetAecValue(uint16_t i_data) {
  config->SaveAecValue(i_data);
  aec_value = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Gain Ctrl
   @param bool - gain ctrl
   @return none
*/
void Camera::SetGainCtrl(bool i_data) {
  config->SaveGainCtrl(i_data);
  gain_ctrl = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set automatic gain control gain
   @param uint8_t - automatic gain control gain
   @return none
*/
void Camera::SetAgcGain(uint8_t i_data) {
  config->SaveAgcGain(i_data);
  agc_gain = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set bad pixel correction
   @param bool - bad pixel correction
   @return none
*/
void Camera::SetBpc(bool i_data) {
  config->SaveBpc(i_data);
  bpc = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set white pixel correction
   @param bool - white pixel correction
   @return none
*/
void Camera::SetWpc(bool i_data) {
  config->SaveWpc(i_data);
  wpc = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Raw Gama
   @param bool - raw gama
   @return none
*/
void Camera::SetRawGama(bool i_data) {
  config->SaveRawGama(i_data);
  raw_gama = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set horizontal Mirror
   @param bool - horizontal mirror
   @return none
*/
void Camera::SetHMirror(bool i_data) {
  config->SaveHmirror(i_data);
  hmirror = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set V Flip
   @param bool - vflip
   @return none
*/
void Camera::SetVFlip(bool i_data) {
  config->SaveVflip(i_data);
  vflip = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Lens correction
   @param bool - lens correction
   @return none
*/
void Camera::SetLensC(bool i_data) {
  config->SaveLensCorrect(i_data);
  lensc = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Exposure Ctrl
   @param bool - exposure ctrl
   @return none
*/
void Camera::SetExposureCtrl(bool i_data) {
  config->SaveExposureCtrl(i_data);
  exposure_ctrl = i_data;
  ApplyCameraCfg();
}

/**
   @brief Set Camera Flash Enable
   @param bool - flash enable/disable
   @return none
*/
void Camera::SetCameraFlashEnable(bool i_data) {
  config->SaveCameraFlashEnable(i_data);
  CameraFlashEnable = i_data;
}

/**
   @brief Set Camera Flash Time
   @param uint16_t - flash time
   @return none
*/
void Camera::SetCameraFlashTime(uint16_t i_data) {
  config->SaveCameraFlashTime(i_data);
  CameraFlashTime = i_data;
}

void Camera::SetCameraImageRotation(uint8_t i_data) {
  config->SaveCameraImageExifRotation(i_data);
  imageExifRotation = i_data;
}

/**
   @brief Get Photo Quality
   @param none
   @return uint8_t - photo quality
*/
uint8_t Camera::GetPhotoQuality() {
  return PhotoQuality;
}

/**
   @brief Get Frame Size
   @param none
   @return uint8_t - frame size
*/
uint8_t Camera::GetFrameSize() {
  return FrameSize;
}

/**
 * @brief transform framesize_t to uint16_t width
 * 
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
    FRAMESIZE_UXGA (1600 x 1200)
 * 
 * @return uint16_t 
 */
uint16_t Camera::GetFrameSizeWidth() {
  uint16_t ret = 0;

  switch (FrameSize) {
    case 0:
      ret = 320;
      break;
    case 1:
      ret = 352;
      break;
    case 2:
      ret = 640;
      break;
    case 3:
      ret = 800;
      break;
    case 4:
      ret = 1024;
      break;
    case 5:
      ret = 1280;
      break;
    case 6:
      ret = 1600;
      break;
    default:
      ret = 320;
      break;
  }

  return ret;
}

/**
 * @brief transform framesize_t to uint16_t height
 * 
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
    FRAMESIZE_UXGA (1600 x 1200)
 * 
 * @return uint16_t 
 */
uint16_t Camera::GetFrameSizeHeight() {
  uint16_t ret = 0;
  switch (FrameSize) {
    case 0:
      ret = 240;
      break;
    case 1:
      ret = 288;
      break;
    case 2:
      ret = 480;
      break;
    case 3:
      ret = 600;
      break;
    case 4:
      ret = 768;
      break;
    case 5:
      ret = 1024;
      break;
    case 6:
      ret = 1200;
      break;
    default:
      ret = 240;
      break;
  }

  return ret;
}

/**
   @brief Get Brightness
   @param none
   @return int8_t - brightness
*/
int8_t Camera::GetBrightness() {
  return brightness;
}

/**
   @brief Get Contrast
   @param none
   @return int8_t - contrast
*/
int8_t Camera::GetContrast() {
  return contrast;
}

/**
   @brief Get Saturation
   @param none
   @return int8_t - saturation
*/
int8_t Camera::GetSaturation() {
  return saturation;
}

/**
   @brief Get Auto white balance status
   @param none
   @return bool - Auto white balance status
*/
bool Camera::GetAwb() {
  return awb;
}

/**
   @brief Get Auto white balance gain status
   @param none
   @return bool -  Auto white balance gain status
*/
bool Camera::GetAwbGain() {
  return awb_gain;
}

/**
   @brief Get Auto white balance mode
   @param none
   @return uint8_t - Auto white balance mode
*/
uint8_t Camera::GetAwbMode() {
  return wb_mode;
}

/**
   @brief Get automatic exposure control 2 status
   @param none
   @return bool - Automatic exposure control 2 status
*/
bool Camera::GetAec2() {
  return aec2;
}

/**
   @brief Get automatic exposure level
   @param none
   @return int8_t - Automatic exposure level
*/
int8_t Camera::GetAeLevel() {
  return ae_level;
}

/**
   @brief Get automatic exposure control value
   @param none
   @return uint16_t - Automatic exposure control value
*/
uint16_t Camera::GetAecValue() {
  return aec_value;
}

/**
   @brief Get Gain Ctrl status
   @param none
   @return bool - Gain control status
*/
bool Camera::GetGainCtrl() {
  return gain_ctrl;
}

/**
   @brief Get Agc Gaint
   @param none
   @return uint8_t - agc gain
*/
uint8_t Camera::GetAgcGaint() {
  return agc_gain;
}

/**
   @brief Get Bpc
   @param none
   @return bool - bpc status
*/
bool Camera::GetBpc() {
  return bpc;
}

/**
   @brief Get Wpc
   @param none
   @return bool - wpc status
*/
bool Camera::GetWpc() {
  return wpc;
}

/**
   @brief Get Raw Gama value
   @param none
   @return bool - raw gamma value
*/
bool Camera::GetRawGama() {
  return raw_gama;
}

/**
   @brief Get horizontal Mirror status
   @param none
   @return bool - horizontal mirror status
*/
bool Camera::GetHMirror() {
  return hmirror;
}

/**
   @brief Get vertical Flip status
   @param none
   @return bool - vertical flip status
*/
bool Camera::GetVFlip() {
  return vflip;
}

/**
   @brief Get Lens correction status
   @param none
   @return bool - lens correction status
*/
bool Camera::GetLensC() {
  return lensc;
}

/**
   @brief Get exposure control status
   @param none
   @return bool - exposure control status
*/
bool Camera::GetExposureCtrl() {
  return exposure_ctrl;
}

/**
   @brief Get Camera Flash Enable status
   @param none
   @return bool - camera flash enable status
*/
bool Camera::GetCameraFlashEnable() {
  return CameraFlashEnable;
}

/**
 * @brief Get camera flash time
 * @param none
 * @return uint16_t - camera flash time
 */
uint16_t Camera::GetCameraFlashTime() {
  return CameraFlashTime;
}

uint8_t Camera::GetCameraImageRotation() {
  return imageExifRotation;
}

/* EOF */