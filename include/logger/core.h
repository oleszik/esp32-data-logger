#pragma once

#include <string>

#include "logger/interfaces.h"

namespace logger {

struct ValidationResult {
  bool valid;
  StatusFlags flags;
};

ValidationResult validateEnvironment(const EnvironmentalReading& reading);
uint8_t batteryPercent(float voltage);
StatusFlags batteryStatus(float voltage);
uint32_t chooseSleepInterval(StatusFlags status);
std::string csvHeader();
std::string csvRow(const Measurement& measurement);
std::string dailyLogPath(const DateTime& time);

class DataLogger {
 public:
  DataLogger(IEnvironmentalSensor& sensor, IClock& clock, IBatteryMonitor& battery,
             IStorage& storage);
  bool begin();
  Measurement sample();
  bool store(Measurement& measurement);

 private:
  IEnvironmentalSensor& sensor_;
  IClock& clock_;
  IBatteryMonitor& battery_;
  IStorage& storage_;
  bool sensor_ready_{false};
  bool clock_ready_{false};
  bool storage_ready_{false};
  uint32_t sequence_{0};
};

}  // namespace logger
