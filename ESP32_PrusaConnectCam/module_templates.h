/**
   @file module_templates.h

   @brief Definition of the module templates

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

*/

#pragma once

#include "mcu_cfg.h"

#if ((AI_THINKER_ESP32_CAM + ESP32_WROVER_DEV + CAMERA_MODEL_ESP32_S3_DEV_CAM + CAMERA_MODEL_ESP32_S3_EYE_2_2 + CAMERA_MODEL_XIAO_ESP32_S3_CAM + CAMERA_MODEL_ESP32_S3_CAM + ESP32_S3_WROOM_FREENOVE + M5STACK_TIMERCAMERA) != 1)
#error "Exactly one camera model must be defined as true."
#endif

#if (true == AI_THINKER_ESP32_CAM)
#include "module_AI_Thinker_ESP32-CAM.h"

#elif (true == ESP32_WROVER_DEV)
#include "module_ESP32-WROVER-DEV.h"

#elif (true == CAMERA_MODEL_ESP32_S3_DEV_CAM)
#include "module_ESP32_S3_DEV_CAM.h"

#elif (true == CAMERA_MODEL_ESP32_S3_EYE_2_2)
#include "module_ESP32-S3-EYE_2_2.h"

#elif (true == CAMERA_MODEL_XIAO_ESP32_S3_CAM)
#include "module_XIAO_ESP32-S3-cam.h"

#elif (true == CAMERA_MODEL_ESP32_S3_CAM)
#include "module_ESP32-S3-CAM.h"

#elif (true == ESP32_S3_WROOM_FREENOVE)
#include "module_ESP32-S3_Wroom_Freenove.h"

#elif (true == M5STACK_TIMERCAMERA)
#include "module_M5Stack_TimerCam.h"

#else
#error "No module selected"

#endif

/* EOF */