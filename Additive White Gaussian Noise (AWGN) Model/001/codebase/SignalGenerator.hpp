#ifndef SIGNAL_GENERATOR_HPP
#define SIGNAL_GENERATOR_HPP

#include <vector>
#include "Common.hpp"

class SignalGenerator {
public:
    SignalGenerator(size_t numSamples, double amplitude, double frequency);
    std::vector<double> generateSineWave();

private:
    size_t numSamples_;
    double amplitude_;
    double frequency_;
};

#endif // SIGNAL_GENERATOR_HPP