#ifndef AWGN_HPP
#define AWGN_HPP

#include <vector>
#include "SignalToNoiseRatio.hpp"
#include "ChannelModel.hpp"

class AWGN {
private:
    SignalToNoiseRatio snrController_;
    unsigned int seed_;
    ChannelModel channelModel_;

public:
    AWGN(double targetSNRdB, double bitRate, double bandwidth, ModulationType mod, CodingType code = NONE, unsigned int seed = 0);
    std::vector<double> addNoise(const std::vector<double>& signal);
    ChannelModel& getChannelModel();
};

#endif // AWGN_HPP