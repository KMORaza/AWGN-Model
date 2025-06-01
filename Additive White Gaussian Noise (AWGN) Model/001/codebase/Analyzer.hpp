#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include <vector>
#include <tuple>
#include <cstddef> // For size_t

class Analyzer {
public:
    double computeSNR(const std::vector<double>& original, const std::vector<double>& noisy);
    double computeZeroCrossings(const std::vector<double>& noisy, double frequency, double bandwidth, double snr_db);
    std::vector<size_t> computeZeroCrossingPoints(const std::vector<double>& noisy);
    std::tuple<double, double, double> computePhasorStatistics(const std::vector<double>& noisy, const std::vector<double>& original, unsigned int seed);
};

#endif // ANALYZER_HPP