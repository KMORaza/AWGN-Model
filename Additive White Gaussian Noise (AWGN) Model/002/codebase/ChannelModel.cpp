#include "ChannelModel.hpp"
#include <cmath>
#include <random>
#include <stdexcept>
#include "Common.hpp"

ChannelModel::ChannelModel(ModulationType mod, CodingType code)
    : modulation_(mod), coding_(code), codeRate_(1.0) {
    switch (modulation_) {
        case BPSK: bitsPerSymbol_ = 1; break;
        case QPSK: bitsPerSymbol_ = 2; break;
        case QAM16: bitsPerSymbol_ = 4; break;
        default: throw std::invalid_argument("Unsupported modulation type");
    }
    if (coding_ == CONVOLUTIONAL) {
        codeRate_ = 0.5; // 1/2 rate convolutional code
    }
}

std::vector<int> ChannelModel::encodeConvolutional(const std::vector<int>& bits) {
    // Simple 1/2 rate convolutional encoder (generator polynomials: 7, 5 in octal)
    std::vector<int> encoded;
    int state = 0;
    for (size_t i = 0; i < bits.size(); ++i) {
        int input = bits[i];
        encoded.push_back((input ^ ((state >> 1) & 1) ^ (state & 1)) & 1);
        encoded.push_back((input ^ (state & 1)) & 1);
        state = ((state >> 1) | (input << 2)) & 7;
    }
    return encoded;
}

std::vector<int> ChannelModel::decodeConvolutional(const std::vector<double>& softBits) {
    // Simplified Viterbi decoder for 1/2 rate code
    std::vector<int> decoded;
    // For simplicity, convert soft bits to hard bits (threshold at 0)
    for (size_t i = 0; i < softBits.size(); i += 2) {
        int b1 = softBits[i] > 0 ? 1 : 0;
        int b2 = softBits[i + 1] > 0 ? 1 : 0;
        // Basic decoding: assume b1^b2 gives original bit (simplified)
        decoded.push_back(b1 ^ b2);
    }
    return decoded;
}

std::vector<double> ChannelModel::modulateBPSK(const std::vector<int>& bits) {
    std::vector<double> symbols(bits.size());
    for (size_t i = 0; i < bits.size(); ++i) {
        symbols[i] = bits[i] ? 1.0 : -1.0;
    }
    return symbols;
}

std::vector<double> ChannelModel::modulateQPSK(const std::vector<int>& bits) {
    std::vector<double> symbols(bits.size() / 2 * 2); // Ensure even number
    const double scale = std::sqrt(2.0) / 2.0;
    for (size_t i = 0; i < bits.size() - 1; i += 2) {
        double I = bits[i] ? scale : -scale;
        double Q = bits[i + 1] ? scale : -scale;
        symbols[i] = I;     // Real part
        symbols[i + 1] = Q; // Imaginary part
    }
    return symbols;
}

std::vector<double> ChannelModel::modulateQAM16(const std::vector<int>& bits) {
    std::vector<double> symbols(bits.size() / 4 * 4); // Ensure multiple of 4
    const double scale = std::sqrt(10.0); // Normalize power
    for (size_t i = 0; i < bits.size() - 3; i += 4) {
        int I_bits = bits[i] * 2 + bits[i + 1];
        int Q_bits = bits[i + 2] * 2 + bits[i + 3];
        double I = (I_bits == 0 ? -3.0 : I_bits == 1 ? -1.0 : I_bits == 2 ? 3.0 : 1.0) / scale;
        double Q = (Q_bits == 0 ? -3.0 : Q_bits == 1 ? -1.0 : Q_bits == 2 ? 3.0 : 1.0) / scale;
        symbols[i] = I;
        symbols[i + 1] = Q;
        symbols[i + 2] = I; // Duplicate for compatibility
        symbols[i + 3] = Q;
    }
    return symbols;
}

std::vector<int> ChannelModel::demodulateBPSK(const std::vector<double>& symbols) {
    std::vector<int> bits(symbols.size());
    for (size_t i = 0; i < symbols.size(); ++i) {
        bits[i] = symbols[i] > 0 ? 1 : 0;
    }
    return bits;
}

std::vector<int> ChannelModel::demodulateQPSK(const std::vector<double>& symbols) {
    std::vector<int> bits(symbols.size());
    for (size_t i = 0; i < symbols.size() - 1; i += 2) {
        bits[i] = symbols[i] > 0 ? 1 : 0;
        bits[i + 1] = symbols[i + 1] > 0 ? 1 : 0;
    }
    return bits;
}

std::vector<int> ChannelModel::demodulateQAM16(const std::vector<double>& symbols) {
    std::vector<int> bits(symbols.size());
    const double scale = std::sqrt(10.0);
    for (size_t i = 0; i < symbols.size() - 3; i += 4) {
        double I = symbols[i] * scale;
        double Q = symbols[i + 1] * scale;
        bits[i] = (I > 0) ? (I > 2 ? 1 : 0) : (I < -2 ? 0 : 1);
        bits[i + 1] = (I > 0) ? (I > 2 ? 0 : 1) : (I < -2 ? 1 : 0);
        bits[i + 2] = (Q > 0) ? (Q > 2 ? 1 : 0) : (Q < -2 ? 0 : 1);
        bits[i + 3] = (Q > 0) ? (Q > 2 ? 0 : 1) : (Q < -2 ? 1 : 0);
    }
    return bits;
}

std::vector<double> ChannelModel::modulate(const std::vector<int>& bits) {
    std::vector<int> encodedBits = (coding_ == CONVOLUTIONAL) ? encodeConvolutional(bits) : bits;
    switch (modulation_) {
        case BPSK: return modulateBPSK(encodedBits);
        case QPSK: return modulateQPSK(encodedBits);
        case QAM16: return modulateQAM16(encodedBits);
        default: throw std::invalid_argument("Unsupported modulation type");
    }
}

std::vector<int> ChannelModel::demodulate(const std::vector<double>& symbols) {
    std::vector<int> bits;
    switch (modulation_) {
        case BPSK: bits = demodulateBPSK(symbols); break;
        case QPSK: bits = demodulateQPSK(symbols); break;
        case QAM16: bits = demodulateQAM16(symbols); break;
        default: throw std::invalid_argument("Unsupported modulation type");
    }
    return (coding_ == CONVOLUTIONAL) ? decodeConvolutional(symbols) : bits;
}

std::vector<int> ChannelModel::encode(const std::vector<int>& bits) {
    return (coding_ == CONVOLUTIONAL) ? encodeConvolutional(bits) : bits;
}

std::vector<int> ChannelModel::decode(const std::vector<double>& softBits) {
    return (coding_ == CONVOLUTIONAL) ? decodeConvolutional(softBits) : demodulate(softBits);
}

size_t ChannelModel::getBitsPerSymbol() const {
    return bitsPerSymbol_;
}

double ChannelModel::getCodeRate() const {
    return codeRate_;
}