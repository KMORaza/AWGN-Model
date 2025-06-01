#ifndef SIGNAL_TO_NOISE_RATIO_HPP
#define SIGNAL_TO_NOISE_RATIO_HPP

#include <vector>

class SignalToNoiseRatio {
private:
    double targetSNRdB_;
    double bitRate_;
    double bandwidth_;
    double calculateNoisePower(double signalPower, double snr_dB) const;

public:
    SignalToNoiseRatio(double targetSNRdB, double bitRate, double bandwidth);
    double calculateEbN0(const std::vector<double>& signal) const;
    void adjustNoisePower(std::vector<double>& signal, double& noisePower) const;
    double getTargetSNRdB() const;
    void setTargetSNRdB(double snr_dB);
    double getBitRate() const;
    void setBitRate(double bitRate);
    double getBandwidth() const;
    void setBandwidth(double bandwidth);
};

#endif // SIGNAL_TO_NOISE_RATIO_HPP