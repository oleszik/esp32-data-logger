#include <fstream>
#include <iostream>

#include "config.h"
#include "logger/core.h"
#include "logger/simulated.h"

#ifndef UNIT_TEST
int main(int argc, char** argv) {
  const std::string output_root = argc > 1 ? argv[1] : "simulation-output";
  logger::SimulatedSensor sensor;
  logger::SimulatedClock clock;
  logger::SimulatedBattery battery;
  logger::FileStorage storage(output_root);
  logger::DataLogger app(sensor, clock, battery, storage);

  std::cout << "ESP32 Environmental Data Logger\nFirmware: " << logger::config::kFirmwareVersion
            << "\nMode: NATIVE SIMULATION (synthetic data, not hardware verification)\n";
  app.begin();
  for (int cycle = 0; cycle < 6; ++cycle) {
    auto measurement = app.sample();
    const bool stored = app.store(measurement);
    std::cout << "[INFO] " << logger::formatIso8601(measurement.timestamp)
              << " seq=" << measurement.sequence
              << " temperature=" << measurement.environment.temperature_c
              << "C humidity=" << measurement.environment.humidity_percent
              << "% pressure=" << measurement.environment.pressure_hpa
              << "hPa battery=" << measurement.battery_voltage << "V ("
              << static_cast<unsigned>(measurement.battery_percent)
              << "%) status=" << logger::statusToString(measurement.status)
              << " csv=" << (stored ? "OK" : "ERROR") << '\n';
  }
  std::cout << "[INFO] Next sample: " << logger::config::kSampleIntervalSeconds << " s\n";
  return 0;
}
#endif
