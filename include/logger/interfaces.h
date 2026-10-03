#pragma once

#include <string>

#include "logger/types.h"

namespace logger {

class IEnvironmentalSensor {
 public:
  virtual ~IEnvironmentalSensor() = default;
  virtual bool begin() = 0;
  virtual EnvironmentalReading read() = 0;
};

class IClock {
 public:
  virtual ~IClock() = default;
  virtual bool begin() = 0;
  virtual DateTime now() = 0;
};

class IBatteryMonitor {
 public:
  virtual ~IBatteryMonitor() = default;
  virtual float readVoltage() = 0;
};

class IStorage {
 public:
  virtual ~IStorage() = default;
  virtual bool begin() = 0;
  virtual bool append(const std::string& path, const std::string& header,
                      const std::string& row) = 0;
};

class IStatusDisplay {
 public:
  virtual ~IStatusDisplay() = default;
  virtual bool begin() = 0;
  virtual void show(const Measurement& measurement, bool storage_ok,
                    uint32_t next_sample_seconds) = 0;
};

}  // namespace logger
