#include "logger/types.h"

#include <array>
#include <cstdio>
#include <sstream>

namespace logger {

std::string formatIso8601(const DateTime& time) {
  if (!time.valid) return "INVALID_TIME";
  std::array<char, 21> buffer{};
  std::snprintf(buffer.data(), buffer.size(), "%04d-%02d-%02dT%02d:%02d:%02d", time.year,
                time.month, time.day, time.hour, time.minute, time.second);
  return buffer.data();
}

std::string statusToString(StatusFlags flags) {
  if (flags == 0) return "OK";
  struct Entry {
    StatusFlag flag;
    const char* text;
  };
  constexpr Entry entries[] = {{StatusFlag::kSensorError, "SENSOR_ERROR"},
                               {StatusFlag::kRtcError, "RTC_ERROR"},
                               {StatusFlag::kStorageError, "SD_ERROR"},
                               {StatusFlag::kLowBattery, "LOW_BATTERY"},
                               {StatusFlag::kCriticalBattery, "CRITICAL_BATTERY"},
                               {StatusFlag::kInvalidReading, "INVALID_READING"},
                               {StatusFlag::kSimulated, "SIMULATED"}};
  std::ostringstream output;
  bool first = true;
  for (const auto& entry : entries) {
    if (!hasFlag(flags, entry.flag)) continue;
    if (!first) output << '|';
    output << entry.text;
    first = false;
  }
  return output.str();
}

}  // namespace logger
