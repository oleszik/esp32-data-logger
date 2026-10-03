#pragma once

#include <cstdint>

namespace logger {
namespace config {

constexpr char kFirmwareVersion[] = "1.0.0";
constexpr uint32_t kSampleIntervalSeconds = 300;
constexpr uint32_t kCriticalSampleIntervalSeconds = 1800;

constexpr float kTemperatureMinC = -40.0F;
constexpr float kTemperatureMaxC = 85.0F;
constexpr float kHumidityMinPercent = 0.0F;
constexpr float kHumidityMaxPercent = 100.0F;
constexpr float kPressureMinHpa = 300.0F;
constexpr float kPressureMaxHpa = 1100.0F;

constexpr int kBatteryAdcPin = 34;
constexpr float kBatteryDividerRatio = 2.0F;
constexpr float kBatteryCalibration = 1.0F;
constexpr float kBatteryWarningVoltage = 3.55F;
constexpr float kBatteryCriticalVoltage = 3.35F;
constexpr float kBatteryEmptyVoltage = 3.20F;
constexpr float kBatteryFullVoltage = 4.20F;

constexpr int kI2cSdaPin = 21;
constexpr int kI2cSclPin = 22;
constexpr uint8_t kBme280Address = 0x76;
constexpr int kSdChipSelectPin = 5;
constexpr char kDataDirectory[] = "/data";

}  // namespace config
}  // namespace logger
