/**
   @file module_templates.h

   @brief Definition of the module templates

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

*/

#pragma once

#include "mcu_cfg.h"

#ifdef AI_THINKER_ESP32_CAM
#include "module_AI_Thinker_ESP32-CAM.h"
#endif

#ifdef ESP32_WROVER_DEV
#include "module_ESP32-WROVER-DEV.h"
#endif

/* EOF */