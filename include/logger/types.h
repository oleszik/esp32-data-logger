#pragma once

#include <cstdint>
#include <string>

namespace logger {

struct DateTime {
  int year{};
  int month{};
  int day{};
  int hour{};
  int minute{};
  int second{};
  bool valid{};
};

struct EnvironmentalReading {
  float temperature_c{};
  float humidity_percent{};
  float pressure_hpa{};
  bool available{};
  bool simulated{};
};

enum class StatusFlag : uint16_t {
  kNone = 0,
  kSensorError = 1U << 0,
  kRtcError = 1U << 1,
  kStorageError = 1U << 2,
  kLowBattery = 1U << 3,
  kCriticalBattery = 1U << 4,
  kInvalidReading = 1U << 5,
  kSimulated = 1U << 6,
};

using StatusFlags = uint16_t;
constexpr StatusFlags flag(StatusFlag value) { return static_cast<StatusFlags>(value); }
constexpr bool hasFlag(StatusFlags flags, StatusFlag value) { return (flags & flag(value)) != 0; }

struct Measurement {
  DateTime timestamp{};
  EnvironmentalReading environment{};
  float battery_voltage{};
  uint8_t battery_percent{};
  uint32_t sequence{};
  StatusFlags status{};
};

std::string formatIso8601(const DateTime& time);
std::string statusToString(StatusFlags flags);

}  // namespace logger
