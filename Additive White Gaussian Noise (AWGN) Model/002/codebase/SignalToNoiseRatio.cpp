#include "SignalToNoiseRatio.hpp"
#include <cmath>
#include <numeric>
#include "Common.hpp"

SignalToNoiseRatio::SignalToNoiseRatio(double targetSNRdB, double bitRate, double bandwidth)
    : targetSNRdB_(targetSNRdB), bitRate_(bitRate), bandwidth_(bandwidth) {}

double SignalToNoiseRatio::calculateEbN0(const std::vector<double>& signal) const {
    // Calculate signal power
    double signalPower = std::accumulate(signal.begin(), signal.end(), 0.0,
        [](double sum, double x) { return sum + x * x; }) / signal.size();
    
    // Calculate noise power from SNR
    double noisePower = calculateNoisePower(signalPower, targetSNRdB_);
    
    // Eb/N0 = (Signal Power / Bit Rate) / (Noise Power / Bandwidth)
    double eb = signalPower / bitRate_;
    double n0 = noisePower / bandwidth_;
    
    // Convert to dB
    return 10.0 * std::log10(eb / n0);
}

double SignalToNoiseRatio::calculateNoisePower(double signalPower, double snr_dB) const {
    return signalPower / std::pow(10.0, snr_dB / 10.0);
}

void SignalToNoiseRatio::adjustNoisePower(std::vector<double>& signal, double& noisePower) const {
    // Calculate current signal power
    double signalPower = std::accumulate(signal.begin(), signal.end(), 0.0,
        [](double sum, double x) { return sum + x * x; }) / signal.size();
    
    // Adjust noise power to achieve target SNR
    noisePower = calculateNoisePower(signalPower, targetSNRdB_);
}

double SignalToNoiseRatio::getTargetSNRdB() const {
    return targetSNRdB_;
}

void SignalToNoiseRatio::setTargetSNRdB(double snr_dB) {
    targetSNRdB_ = snr_dB;
}

double SignalToNoiseRatio::getBitRate() const {
    return bitRate_;
}

void SignalToNoiseRatio::setBitRate(double bitRate) {
    bitRate_ = bitRate;
}

double SignalToNoiseRatio::getBandwidth() const {
    return bandwidth_;
}

void SignalToNoiseRatio::setBandwidth(double bandwidth) {
    bandwidth_ = bandwidth;
}