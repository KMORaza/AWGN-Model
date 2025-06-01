#include "SignalGenerator.hpp"

SignalGenerator::SignalGenerator(size_t numSamples, double amplitude, double frequency)
    : numSamples_(numSamples), amplitude_(amplitude), frequency_(frequency) {}

std::vector<double> SignalGenerator::generateSineWave() {
    std::vector<double> signal(numSamples_);
    for (size_t i = 0; i < numSamples_; ++i) {
        signal[i] = amplitude_ * std::sin(2.0 * Constants::PI * frequency_ * i);
    }
    return signal;
}