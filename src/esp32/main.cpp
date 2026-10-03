#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <RTClib.h>
#include <SD.h>
#include <Wire.h>
#include <esp_sleep.h>

#include "config.h"
#include "logger/core.h"

namespace {

class Bme280Sensor final : public logger::IEnvironmentalSensor {
 public:
  bool begin() override { return sensor_.begin(logger::config::kBme280Address, &Wire); }
  logger::EnvironmentalReading read() override {
    return {sensor_.readTemperature(), sensor_.readHumidity(), sensor_.readPressure() / 100.0F,
            true, false};
  }

 private:
  Adafruit_BME280 sensor_;
};

class Ds3231Clock final : public logger::IClock {
 public:
  bool begin() override {
    if (!rtc_.begin() || rtc_.lostPower()) return false;
    return true;
  }
  logger::DateTime now() override {
    const auto value = rtc_.now();
    const bool valid = value.year() >= 2024 && value.year() <= 2099;
    return {value.year(),   value.month(),  value.day(), value.hour(),
            value.minute(), value.second(), valid};
  }

 private:
  RTC_DS3231 rtc_;
};

class AdcBattery final : public logger::IBatteryMonitor {
 public:
  float readVoltage() override {
    const uint32_t millivolts = analogReadMilliVolts(logger::config::kBatteryAdcPin);
    return (millivolts / 1000.0F) * logger::config::kBatteryDividerRatio *
           logger::config::kBatteryCalibration;
  }
};

class SdStorage final : public logger::IStorage {
 public:
  bool begin() override {
    if (!SD.begin(logger::config::kSdChipSelectPin)) return false;
    if (!SD.exists(logger::config::kDataDirectory)) return SD.mkdir(logger::config::kDataDirectory);
    return true;
  }
  bool append(const std::string& path, const std::string& header, const std::string& row) override {
    const std::string absolute = "/" + path;
    const bool needs_header = !SD.exists(absolute.c_str());
    File file = SD.open(absolute.c_str(), FILE_APPEND);
    if (!file) return false;
    if (needs_header && !file.println(header.c_str())) return false;
    const bool ok = file.println(row.c_str()) > 0;
    file.flush();
    file.close();
    return ok;
  }
};

Bme280Sensor sensor;
Ds3231Clock clock_source;
AdcBattery battery;
SdStorage storage;
logger::DataLogger app(sensor, clock_source, battery, storage);

const char* wakeReason() {
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "power-on/reset";
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(150);
  Wire.begin(logger::config::kI2cSdaPin, logger::config::kI2cSclPin);
  analogSetPinAttenuation(logger::config::kBatteryAdcPin, ADC_11db);
  Serial.printf("ESP32 Environmental Data Logger\nFirmware: %s\nWake reason: %s\n",
                logger::config::kFirmwareVersion, wakeReason());
  const bool all_ready = app.begin();
  auto measurement = app.sample();
  const bool written = app.store(measurement);
  Serial.printf("Timestamp: %s\nBattery: %.2f V (%u%%)\nStatus: %s\nCSV write: %s\n",
                logger::formatIso8601(measurement.timestamp).c_str(), measurement.battery_voltage,
                measurement.battery_percent, logger::statusToString(measurement.status).c_str(),
                written ? "OK" : "ERROR");
  if (measurement.environment.available) {
    Serial.printf("Temperature: %.2f C\nHumidity: %.2f %%\nPressure: %.2f hPa\n",
                  measurement.environment.temperature_c, measurement.environment.humidity_percent,
                  measurement.environment.pressure_hpa);
  }
  const uint32_t interval = logger::chooseSleepInterval(measurement.status);
  Serial.printf("Initialization: %s\nNext sample: %lu s\nEntering deep sleep...\n",
                all_ready ? "OK" : "DEGRADED", static_cast<unsigned long>(interval));
  Serial.flush();
  esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(interval) * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}
