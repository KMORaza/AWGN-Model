#ifndef CHANNEL_MODEL_HPP
#define CHANNEL_MODEL_HPP

#include <vector>
#include <string>
#include "SignalToNoiseRatio.hpp"

enum ModulationType { BPSK, QPSK, QAM16 };
enum CodingType { NONE, CONVOLUTIONAL }; // Turbo and LDPC as future extensions

class ChannelModel {
private:
    ModulationType modulation_;
    CodingType coding_;
    size_t bitsPerSymbol_;
    double codeRate_;
    std::vector<int> encodeConvolutional(const std::vector<int>& bits);
    std::vector<int> decodeConvolutional(const std::vector<double>& softBits);
    std::vector<double> modulateBPSK(const std::vector<int>& bits);
    std::vector<double> modulateQPSK(const std::vector<int>& bits);
    std::vector<double> modulateQAM16(const std::vector<int>& bits);
    std::vector<int> demodulateBPSK(const std::vector<double>& symbols);
    std::vector<int> demodulateQPSK(const std::vector<double>& symbols);
    std::vector<int> demodulateQAM16(const std::vector<double>& symbols);

public:
    ChannelModel(ModulationType mod, CodingType code = NONE);
    std::vector<double> modulate(const std::vector<int>& bits);
    std::vector<int> demodulate(const std::vector<double>& symbols);
    std::vector<int> encode(const std::vector<int>& bits);
    std::vector<int> decode(const std::vector<double>& softBits);
    size_t getBitsPerSymbol() const;
    double getCodeRate() const;
};

#endif // CHANNEL_MODEL_HPP