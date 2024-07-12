/**
   @file ExternalTemperatureSensor.h

   @brief Library for external temperature sensor control

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#pragma once

#include <Arduino.h>
#include "log.h"
#include "mcu_cfg.h"
#include "module_templates.h"
#include <dhtnew.h>
#include "cfg.h"
#include "ExternalTemperatureSensorTypes.h"

class Logs;
class Configuration;

class ExternalSensor {
    private:
        TemperatureSensorUnit_enum Unit;       ///< unit number
        DHTNEW DhtSensor;                      ///< DHT sensor object
        uint8_t Pin;                           ///< pin number
        bool SystemEnable;                     ///< system enable flag
        bool UserEnable;                       ///< user enable flag
        uint8_t SensorType;                    ///< sensor type

        Logs *log;
        Configuration *config;

    public:
        ExternalSensor(uint8_t, bool, Logs *, Configuration *);
        ~ExternalSensor(){};

        void Init();
        String GetSensorStatus();

        void EnableSensor(bool);
        void ReadSensorData();
        void SetUnit(TemperatureSensorUnit_enum);
        
        float GetTemperature();
        float GetHumidity();
        String GetTemperatureString();
        String GetHumidityString();

        bool GetUserEnableSensor();
        TemperatureSensorUnit_enum GetTemperatureUnit();
};

extern ExternalSensor ExternalTemperatureSensor;

/* EOF */