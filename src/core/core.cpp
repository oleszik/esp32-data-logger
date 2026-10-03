#include "logger/core.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "config.h"

namespace logger {

ValidationResult validateEnvironment(const EnvironmentalReading& r) {
  const bool finite = std::isfinite(r.temperature_c) && std::isfinite(r.humidity_percent) &&
                      std::isfinite(r.pressure_hpa);
  const bool ranges =
      r.temperature_c >= config::kTemperatureMinC && r.temperature_c <= config::kTemperatureMaxC &&
      r.humidity_percent >= config::kHumidityMinPercent &&
      r.humidity_percent <= config::kHumidityMaxPercent &&
      r.pressure_hpa >= config::kPressureMinHpa && r.pressure_hpa <= config::kPressureMaxHpa;
  const bool valid = r.available && finite && ranges;
  StatusFlags flags = valid ? 0 : flag(StatusFlag::kInvalidReading);
  if (!r.available) flags |= flag(StatusFlag::kSensorError);
  return {valid, flags};
}

uint8_t batteryPercent(float voltage) {
  if (!std::isfinite(voltage) || voltage <= config::kBatteryEmptyVoltage) return 0;
  if (voltage >= config::kBatteryFullVoltage) return 100;
  const auto value = (voltage - config::kBatteryEmptyVoltage) * 100.0F /
                     (config::kBatteryFullVoltage - config::kBatteryEmptyVoltage);
  return static_cast<uint8_t>(value + 0.5F);
}

StatusFlags batteryStatus(float voltage) {
  if (!std::isfinite(voltage) || voltage <= config::kBatteryCriticalVoltage)
    return flag(StatusFlag::kLowBattery) | flag(StatusFlag::kCriticalBattery);
  if (voltage <= config::kBatteryWarningVoltage) return flag(StatusFlag::kLowBattery);
  return 0;
}

uint32_t chooseSleepInterval(StatusFlags status) {
  return hasFlag(status, StatusFlag::kCriticalBattery) ? config::kCriticalSampleIntervalSeconds
                                                       : config::kSampleIntervalSeconds;
}

std::string csvHeader() {
  return "timestamp,sequence,temperature_c,humidity_percent,pressure_hpa,battery_voltage,battery_"
         "percent,status";
}

std::string csvRow(const Measurement& m) {
  std::ostringstream row;
  row << formatIso8601(m.timestamp) << ',' << m.sequence << ',' << std::fixed
      << std::setprecision(2);
  if (m.environment.available) {
    row << m.environment.temperature_c << ',' << m.environment.humidity_percent << ','
        << m.environment.pressure_hpa;
  } else {
    row << ",,";
  }
  row << ',' << m.battery_voltage << ',' << static_cast<unsigned>(m.battery_percent) << ','
      << statusToString(m.status);
  return row.str();
}

std::string dailyLogPath(const DateTime& t) {
  if (!t.valid) return {};
  std::ostringstream path;
  path << "data/" << std::setfill('0') << std::setw(4) << t.year << '-' << std::setw(2) << t.month
       << '-' << std::setw(2) << t.day << ".csv";
  return path.str();
}

DataLogger::DataLogger(IEnvironmentalSensor& sensor, IClock& clock, IBatteryMonitor& battery,
                       IStorage& storage)
    : sensor_(sensor), clock_(clock), battery_(battery), storage_(storage) {}

bool DataLogger::begin() {
  sensor_ready_ = sensor_.begin();
  clock_ready_ = clock_.begin();
  storage_ready_ = storage_.begin();
  return sensor_ready_ && clock_ready_ && storage_ready_;
}

Measurement DataLogger::sample() {
  Measurement m;
  m.sequence = ++sequence_;
  m.timestamp = clock_ready_ ? clock_.now() : DateTime{};
  if (!m.timestamp.valid) m.status |= flag(StatusFlag::kRtcError);
  m.environment = sensor_ready_ ? sensor_.read() : EnvironmentalReading{};
  const auto validation = validateEnvironment(m.environment);
  m.status |= validation.flags;
  if (m.environment.simulated) m.status |= flag(StatusFlag::kSimulated);
  m.battery_voltage = battery_.readVoltage();
  m.battery_percent = batteryPercent(m.battery_voltage);
  m.status |= batteryStatus(m.battery_voltage);
  if (!storage_ready_) m.status |= flag(StatusFlag::kStorageError);
  return m;
}

bool DataLogger::store(Measurement& m) {
  if (!m.timestamp.valid) return false;  // Never create a misleading dated file.
  if (!storage_ready_) storage_ready_ = storage_.begin();
  if (!storage_ready_ || !storage_.append(dailyLogPath(m.timestamp), csvHeader(), csvRow(m))) {
    m.status |= flag(StatusFlag::kStorageError);
    storage_ready_ = false;
    return false;
  }
  return true;
}

}  // namespace logger
