/**
   @file ExternalTemperatureSensor.cpp

   @brief Library for external temperature sensor control

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "ExternalTemperatureSensor.h"

ExternalSensor ExternalTemperatureSensor(DHT_SENSOR_PIN, DHT_SENSOR_ENABLE, &SystemLog, &SystemConfig);

ExternalSensor::ExternalSensor(uint8_t i_pin, bool i_enable, Logs *i_log, Configuration *i_cfg) : DhtSensor(i_pin) {
    Pin = i_pin;
    SystemEnable = i_enable;
    log = i_log;
    config = i_cfg;
}

void ExternalSensor::Init() {
    log->AddEvent(LogLevel_Info, F("Init external temperature sensor"));
    Unit = (TemperatureSensorUnit_enum) config->LoadExternalTemperatureSensorUnit();
    UserEnable = config->LoadExternalTemperatureSensorEnable();
   //DhtSensor.setHumOffset(10);
   //DhtSensor.setTempOffset(-3.5);

   ReadSensorData();
}

String ExternalSensor::GetSensorStatus() {
    String status = "";

    if (false == SystemEnable) {
        status = "Unsupport sensor";
    } else if (false == UserEnable) {
        status = "Sensor disabled";
    } else {
        status = "Detected: ";
        SensorType = DhtSensor.getType();
        switch (SensorType) {
            case 0:
                status += "not defined";
                break;
            case 11:
                status += "DHT11";
                break;
            case 22:
                status += "DHT22";
                break;
            case 23:
                status += "DHT22";
                break;
            case 70:
                status += "Sonoff Si7021";
                break;
            default:
                status += "Unknown";
                break;
        }
    }

    return status;
}

void ExternalSensor::EnableSensor(bool i_enable) {
    if (true == SystemEnable) {
        UserEnable = i_enable;  
        config->SaveExternalTemperatureSensorEnable(UserEnable);
    } else {
        UserEnable = false;
        config->SaveExternalTemperatureSensorEnable(UserEnable);
    }
}

void ExternalSensor::ReadSensorData() {
    if ((true == SystemEnable) && (true == UserEnable)) {
        DhtSensor.read();
    }
}

void ExternalSensor::SetUnit(TemperatureSensorUnit_enum i_unit) {
    Unit = i_unit;
    config->SaveExternalTemperatureSensorUnit(Unit);
}

float ExternalSensor::GetTemperature() {
    float temp = 0.0;
    if ((true == SystemEnable) && (true == UserEnable)) {
        if (Unit == TEMPERATURE_UNIT_CELSIUS) {
            temp = DhtSensor.getTemperature();
        } else if (Unit == TEMPERATURE_UNIT_FAHRENHEIT) {
            temp = DhtSensor.getTemperature() * 9.0 / 5.0 + 32.0;
        }
    }

    return temp;
}

float ExternalSensor::GetHumidity() {
    if ((true == SystemEnable) && (true == UserEnable)) {
        return DhtSensor.getHumidity();
    } else {
        return 0.0;
    }
}
        
String ExternalSensor::GetTemperatureString() {
    String temp = "";

    if (false == SystemEnable) {
        temp = "Unsupport";
    } else if ((false == UserEnable) || (false == SystemEnable)) {
        temp = "Disabled";
    } else {
        if (Unit == TEMPERATURE_UNIT_CELSIUS) {
            temp = String(GetTemperature(), 1) + " °C";
        } else if (Unit == TEMPERATURE_UNIT_FAHRENHEIT) {
            temp = String(GetTemperature(), 1) + " °F";
        }
    }

    return temp;
}

String ExternalSensor::GetHumidityString() {
    String hum = "";

    if (false == SystemEnable) {
        hum = "Unsupport";
    } else if ((false == UserEnable) || (false == SystemEnable)) {
        hum = "Disabled";
    } else {
        hum = String(GetHumidity(), 1) + " %";
    }

    return hum;
}

bool ExternalSensor::GetUserEnableSensor() {
    return UserEnable;
}

TemperatureSensorUnit_enum ExternalSensor::GetTemperatureUnit() {
    return Unit;
}

/* EOF */