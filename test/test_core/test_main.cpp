#include <unity.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>

#include "config.h"
#include "logger/core.h"
#include "logger/simulated.h"

using namespace logger;

void setUp() {}
void tearDown() {}

void test_validation_accepts_boundaries() {
  TEST_ASSERT_TRUE(validateEnvironment({-40, 0, 300, true, false}).valid);
  TEST_ASSERT_TRUE(validateEnvironment({85, 100, 1100, true, false}).valid);
}
void test_validation_rejects_nan_and_range() {
  TEST_ASSERT_FALSE(validateEnvironment({NAN, 50, 1000, true, false}).valid);
  TEST_ASSERT_TRUE(hasFlag(validateEnvironment({20, 101, 1000, true, false}).flags,
                           StatusFlag::kInvalidReading));
}
void test_unavailable_sensor_flags_error() {
  auto result = validateEnvironment({});
  TEST_ASSERT_TRUE(hasFlag(result.flags, StatusFlag::kSensorError));
}
void test_battery_conversion_and_thresholds() {
  TEST_ASSERT_EQUAL_UINT8(0, batteryPercent(3.2F));
  TEST_ASSERT_EQUAL_UINT8(50, batteryPercent(3.7F));
  TEST_ASSERT_EQUAL_UINT8(100, batteryPercent(4.2F));
  TEST_ASSERT_EQUAL_UINT16(0, batteryStatus(3.8F));
  TEST_ASSERT_TRUE(hasFlag(batteryStatus(3.5F), StatusFlag::kLowBattery));
  TEST_ASSERT_TRUE(hasFlag(batteryStatus(3.3F), StatusFlag::kCriticalBattery));
  TEST_ASSERT_EQUAL(config::kCriticalSampleIntervalSeconds,
                    chooseSleepInterval(batteryStatus(3.3F)));
}
void test_csv_schema_and_format() {
  Measurement m{{2026, 10, 3, 10, 15, 0, true}, {23.4F, 51.2F, 1013.8F, true, true}, 3.94F, 74, 7,
                flag(StatusFlag::kSimulated)};
  TEST_ASSERT_EQUAL_STRING(
      "timestamp,sequence,temperature_c,humidity_percent,pressure_hpa,battery_voltage,battery_"
      "percent,status",
      csvHeader().c_str());
  TEST_ASSERT_EQUAL_STRING("2026-10-03T10:15:00,7,23.40,51.20,1013.80,3.94,74,SIMULATED",
                           csvRow(m).c_str());
  TEST_ASSERT_EQUAL_STRING("data/2026-10-03.csv", dailyLogPath(m.timestamp).c_str());
  TEST_ASSERT_TRUE(dailyLogPath({}).empty());
}
void test_simulations_are_deterministic() {
  SimulatedSensor first;
  SimulatedSensor second;
  first.begin();
  second.begin();
  TEST_ASSERT_FLOAT_WITHIN(0.001F, first.read().temperature_c, second.read().temperature_c);
  TEST_ASSERT_FLOAT_WITHIN(0.001F, 22.35F, first.read().temperature_c);
  SimulatedClock clock;
  clock.begin();
  TEST_ASSERT_EQUAL_STRING("2026-10-03T10:00:00", formatIso8601(clock.now()).c_str());
  TEST_ASSERT_EQUAL_STRING("2026-10-03T10:05:00", formatIso8601(clock.now()).c_str());
}

class FailingStorage : public IStorage {
 public:
  bool begin() override { return false; }
  bool append(const std::string&, const std::string&, const std::string&) override { return false; }
};

void test_logger_sequence_and_failures() {
  SimulatedSensor sensor(false);
  SimulatedClock clock(false);
  SimulatedBattery battery(3.3F);
  FailingStorage storage;
  DataLogger logger(sensor, clock, battery, storage);
  TEST_ASSERT_FALSE(logger.begin());
  auto first = logger.sample();
  auto second = logger.sample();
  TEST_ASSERT_EQUAL_UINT32(1, first.sequence);
  TEST_ASSERT_EQUAL_UINT32(2, second.sequence);
  TEST_ASSERT_TRUE(hasFlag(first.status, StatusFlag::kSensorError));
  TEST_ASSERT_TRUE(hasFlag(first.status, StatusFlag::kRtcError));
  TEST_ASSERT_TRUE(hasFlag(first.status, StatusFlag::kStorageError));
  TEST_ASSERT_TRUE(hasFlag(first.status, StatusFlag::kCriticalBattery));
  TEST_ASSERT_FALSE(logger.store(first));
}

void test_file_storage_header_and_append() {
  const std::string root = "esp32_logger_test_output";
  std::remove((root + "/data/test.csv").c_str());
  FileStorage storage(root);
  TEST_ASSERT_TRUE(storage.begin());
  TEST_ASSERT_TRUE(storage.append("data/test.csv", "header", "one"));
  TEST_ASSERT_TRUE(storage.append("data/test.csv", "header", "two"));
  std::ifstream input(root + "/data/test.csv");
  std::string contents((std::istreambuf_iterator<char>(input)), {});
  TEST_ASSERT_EQUAL_STRING("header\none\ntwo\n", contents.c_str());
  std::remove((root + "/data/test.csv").c_str());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_validation_accepts_boundaries);
  RUN_TEST(test_validation_rejects_nan_and_range);
  RUN_TEST(test_unavailable_sensor_flags_error);
  RUN_TEST(test_battery_conversion_and_thresholds);
  RUN_TEST(test_csv_schema_and_format);
  RUN_TEST(test_simulations_are_deterministic);
  RUN_TEST(test_logger_sequence_and_failures);
  RUN_TEST(test_file_storage_header_and_append);
  return UNITY_END();
}
