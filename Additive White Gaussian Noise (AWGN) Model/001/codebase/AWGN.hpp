#ifndef AWGN_HPP
#define AWGN_HPP

#include <vector>

class AWGN {
private:
    double targetSNRdB_;
    unsigned int seed_;
    double calculateNoisePower(double signalPower, double snr_dB);

public:
    AWGN(double targetSNRdB, unsigned int seed = 0);
    std::vector<double> addNoise(const std::vector<double>& signal);
};

#endif // AWGN_HPP