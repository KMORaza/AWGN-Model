#include "AWGN.hpp"
#include <random>
#include <numeric>
#include <cmath>
#include <glib.h> // For G_PI

AWGN::AWGN(double targetSNRdB, double bitRate, double bandwidth, ModulationType mod, CodingType code, unsigned int seed)
    : snrController_(targetSNRdB, bitRate, bandwidth), seed_(seed), channelModel_(mod, code) {}

std::vector<double> AWGN::addNoise(const std::vector<double>& signal) {
    double noisePower;
    snrController_.adjustNoisePower(const_cast<std::vector<double>&>(signal), noisePower);
    double noiseStdDev = std::sqrt(noisePower);

    std::mt19937 gen(seed_);
    std::uniform_real_distribution<> uniform(0.0, 1.0);

    std::vector<double> noisySignal(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        // Box-Muller transform
        double u1 = uniform(gen);
        double u2 = uniform(gen);
        double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * G_PI * u2); // Standard normal
        noisySignal[i] = signal[i] + noiseStdDev * z; // Scale by noiseStdDev
    }

    return noisySignal;
}

ChannelModel& AWGN::getChannelModel() {
    return channelModel_;
}