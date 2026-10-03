#include "logger/simulated.h"

#include <sys/stat.h>

#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#endif

namespace logger {

EnvironmentalReading SimulatedSensor::read() {
  if (!available_) return {};
  static constexpr float temperature[] = {22.10F, 22.35F, 22.60F, 22.40F, 22.20F};
  static constexpr float humidity[] = {48.0F, 48.4F, 48.8F, 48.5F, 48.2F};
  static constexpr float pressure[] = {1012.4F, 1012.6F, 1012.8F, 1012.7F, 1012.5F};
  const auto index = sample_++ % 5;
  return {temperature[index], humidity[index], pressure[index], true, true};
}

DateTime SimulatedClock::now() {
  if (!available_) return {};
  const uint32_t total_minutes = sample_++ * 5;
  return {
      2026, 10,  3, 10 + static_cast<int>(total_minutes / 60), static_cast<int>(total_minutes % 60),
      0,    true};
}

float SimulatedBattery::readVoltage() {
  const float result = voltage_;
  voltage_ -= 0.01F;
  return result;
}

bool FileStorage::begin() {
  const auto make_directory = [](const std::string& path) {
#ifdef _WIN32
    const int result = _mkdir(path.c_str());
#else
    const int result = mkdir(path.c_str(), 0755);
#endif
    return result == 0 || errno == EEXIST;
  };
  return make_directory(root_) && make_directory(root_ + "/data");
}

bool FileStorage::append(const std::string& path, const std::string& header,
                         const std::string& row) {
  const std::string full_path = root_ + "/" + path;
  std::ifstream existing(full_path, std::ios::binary | std::ios::ate);
  const bool needs_header = !existing || existing.tellg() == 0;
  existing.close();
  std::ofstream output(full_path, std::ios::app);
  if (!output) return false;
  if (needs_header) output << header << '\n';
  output << row << '\n';
  output.flush();
  return output.good();
}

}  // namespace logger
