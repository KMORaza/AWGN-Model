#include "Analyzer.hpp"
#include <numeric>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <random>

double Analyzer::computeSNR(const std::vector<double>& original, const std::vector<double>& noisy) {
    if (original.size() != noisy.size()) {
        throw std::invalid_argument("Signal and noisy signal must have the same size");
    }

    double signalPower = std::accumulate(original.begin(), original.end(), 0.0,
        [](double sum, double x) { return sum + x * x; }) / original.size();

    std::vector<double> noise(original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        noise[i] = noisy[i] - original[i];
    }
    double noisePower = std::accumulate(noise.begin(), noise.end(), 0.0,
        [](double sum, double x) { return sum + x * x; }) / noise.size();

    if (noisePower == 0.0) {
        return std::numeric_limits<double>::infinity();
    }
    return 10.0 * std::log10(signalPower / noisePower);
}

double Analyzer::computeZeroCrossings(const std::vector<double>& noisy, double frequency, double bandwidth, double snr_db) {
    double snr_linear = std::pow(10.0, snr_db / 10.0);
    double term = (snr_linear + 1 + (bandwidth * bandwidth) / (12 * frequency * frequency)) / (snr_linear + 1);
    return frequency * std::sqrt(term);
}

std::vector<size_t> Analyzer::computeZeroCrossingPoints(const std::vector<double>& noisy) {
    std::vector<size_t> crossingPoints;
    for (size_t i = 1; i < noisy.size(); ++i) {
        if ((noisy[i-1] < 0 && noisy[i] >= 0) || (noisy[i-1] > 0 && noisy[i] <= 0)) {
            crossingPoints.push_back(i);
        }
    }
    return crossingPoints;
}

std::tuple<double, double, double> Analyzer::computePhasorStatistics(const std::vector<double>& noisy, const std::vector<double>& original, unsigned int seed) {
    if (noisy.size() != original.size()) {
        throw std::invalid_argument("Signal and noisy signal must have the same size");
    }

    // Compute noise
    std::vector<double> noise(noisy.size());
    for (size_t i = 0; i < noisy.size(); ++i) {
        noise[i] = noisy[i] - original[i];
    }

    // Generate bandlimited AWGN
    double noisePower = std::accumulate(noise.begin(), noise.end(), 0.0,
        [](double sum, double x) { return sum + x * x; }) / noise.size();
    double sigma = std::sqrt(noisePower / 2);

    std::mt19937 gen(seed);
    std::normal_distribution<> dist(0.0, sigma);
    std::vector<double> real_parts(noisy.size()), imag_parts(noisy.size());
    for (size_t i = 0; i < noisy.size(); ++i) {
        real_parts[i] = dist(gen);
        imag_parts[i] = dist(gen);
    }

    // Compute magnitudes
    int count1 = 0, count2 = 0, count3 = 0;
    for (size_t i = 0; i < noisy.size(); ++i) {
        double magnitude = std::sqrt(real_parts[i] * real_parts[i] + imag_parts[i] * imag_parts[i]);
        if (magnitude <= sigma) count1++;
        if (magnitude <= 2 * sigma) count2++;
        if (magnitude <= 3 * sigma) count3++;
    }

    return {
        static_cast<double>(count1) / noisy.size(),
        static_cast<double>(count2) / noisy.size(),
        static_cast<double>(count3) / noisy.size()
    };
}