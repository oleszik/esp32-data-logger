#pragma once

#include <fstream>
#include <string>

#include "logger/interfaces.h"

namespace logger {

class SimulatedSensor final : public IEnvironmentalSensor {
 public:
  explicit SimulatedSensor(bool available = true) : available_(available) {}
  bool begin() override { return available_; }
  EnvironmentalReading read() override;

 private:
  bool available_;
  uint32_t sample_{0};
};

class SimulatedClock final : public IClock {
 public:
  explicit SimulatedClock(bool available = true) : available_(available) {}
  bool begin() override { return available_; }
  DateTime now() override;

 private:
  bool available_;
  uint32_t sample_{0};
};

class SimulatedBattery final : public IBatteryMonitor {
 public:
  explicit SimulatedBattery(float start_voltage = 4.05F) : voltage_(start_voltage) {}
  float readVoltage() override;

 private:
  float voltage_;
};

class FileStorage final : public IStorage {
 public:
  explicit FileStorage(std::string root) : root_(std::move(root)) {}
  bool begin() override;
  bool append(const std::string& path, const std::string& header, const std::string& row) override;

 private:
  std::string root_;
};

}  // namespace logger
