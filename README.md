# Model of Additive White Gaussian Noise (AWGN)

The codebase models Additive White Gaussian Noise (AWGN) and simulates a digital communication system with modulation, noise addition, and performance analysis, focusing on signal processing, modulation schemes, channel coding, and noise modeling.

## Simulation Logic
The simulation emulates a digital communication system where a binary sequence is modulated, passed through an AWGN channel, and demodulated to evaluate performance metrics like Bit Error Rate (BER) and Energy per Bit to Noise Power Spectral Density ratio (Eb/N0). 

### 1.1 Signal Generation
- **Purpose**: Generates a digital bit sequence to serve as the input for modulation.
- **Implementation** (`main.cpp`):
  - A random binary sequence (0s and 1s) is generated using the Mersenne Twister random number generator (`std::mt19937`) with a user-specified seed for reproducibility.
  - The sequence length is determined by the user-defined number of samples (`num_samples`).
  - Example: For `num_samples = 1000`, a vector of 1000 bits is created with equal probability for 0 and 1.

### 1.2 Modulation and Encoding
- **Purpose**: Converts the binary sequence into a modulated signal suitable for transmission.
- **Implementation** (`ChannelModel.cpp`):
  - **Modulation**: Supports three modulation schemes:
    - **BPSK (Binary Phase Shift Keying)**: Maps each bit to +1.0 (for 1) or -1.0 (for 0), producing a real-valued signal.
    - **QPSK (Quadrature Phase Shift Keying)**: Maps pairs of bits to complex symbols with real and imaginary components, scaled by `sqrt(2)/2` for unit power. Each pair produces two values (I and Q components).
    - **16-QAM (16-Quadrature Amplitude Modulation)**: Maps groups of four bits to one of 16 complex symbols, normalized by `sqrt(10)` to maintain unit power. Each group produces four values (duplicated I and Q for compatibility).
  - **Channel Coding**: Optionally applies convolutional coding (1/2 rate) before modulation:
    - Uses generator polynomials (7, 5 in octal) to produce two output bits per input bit, doubling the sequence length.
    - A simplified Viterbi decoder is used for decoding, converting soft decisions (received symbols) to hard decisions by thresholding at zero.

### 1.3 Noise Addition
- **Purpose**: Simulates the effect of an AWGN channel by adding Gaussian noise to the modulated signal.
- **Implementation** (`AWGN.cpp`):
  - Noise is generated using the Box-Muller transform to produce standard normal random variables, scaled by the noise standard deviation (`noiseStdDev`).
  - The noise power is calculated based on the signal power and the target Signal-to-Noise Ratio (SNR) in dB, ensuring the desired noise level is achieved.
  - The noisy signal is computed as: `noisySignal[i] = signal[i] + noiseStdDev * z`, where `z` is a standard normal variable.

### 1.4 Demodulation and Decoding
- **Purpose**: Recovers the original bit sequence from the noisy signal and evaluates performance.
- **Implementation** (`ChannelModel.cpp`):
  - **Demodulation**:
    - **BPSK**: Thresholds each symbol at zero (positive → 1, negative → 0).
    - **QPSK**: Thresholds real and imaginary components separately to recover bit pairs.
    - **16-QAM**: Maps symbols back to four-bit groups using decision boundaries scaled by `sqrt(10)`, comparing against thresholds (e.g., ±2/√10).
  - **Decoding**: If convolutional coding is used, a simplified Viterbi decoder converts soft symbols to hard bits, assuming the XOR of paired bits approximates the original bit.
  - **BER Calculation** (`main.cpp`): Compares the original and decoded bit sequences to compute the Bit Error Rate as the ratio of erroneous bits to total bits.

### 1.5 Performance Analysis
- **Purpose**: Quantifies the system’s performance using metrics like BER and Eb/N0.
- **Implementation** (`SignalToNoiseRatio.cpp`, `Analyzer.cpp`):
  - **BER**: Calculated as `errors / bits.size()`, where `errors` is the count of mismatched bits between input and output.
  - **Eb/N0**: Computed as the ratio of signal power per bit to noise power spectral density, converted to dB:
    - `Eb = signalPower / bitRate`
    - `N0 = noisePower / bandwidth`
    - `Eb/N0 (dB) = 10 * log10(Eb / N0)`
  - **SNR Verification** (`Analyzer.cpp`): Computes the actual SNR of the noisy signal by comparing signal and noise powers, returning infinity if noise power is zero.
  - **Zero Crossings**: Estimates the frequency of zero crossings in the noisy signal, adjusted for SNR, bandwidth, and signal frequency, using the formula:
    - `frequency * sqrt((SNR_linear + 1 + (bandwidth^2)/(12*frequency^2)) / (SNR_linear + 1))`
  - **Phasor Statistics**: Calculates the proportion of noise samples within 1σ, 2σ, and 3σ of a Gaussian distribution, used for phasor plot visualization.

## Modeling Logic
The modeling approach is based on a digital communication system with an AWGN channel, incorporating realistic signal processing and noise characteristics.

### 2.1 Signal Model
- **Base Signal**: Although `SignalGenerator.cpp` defines a sine wave generator, the simulation primarily uses random binary sequences for digital modulation rather than analog signals.
- **Modulation Model** (`ChannelModel.cpp`):
  - Models digital modulation schemes commonly used in communication systems:
    - BPSK: Simplest, with one bit per symbol, robust to noise but low data rate.
    - QPSK: Two bits per symbol, balancing data rate and noise resilience.
    - 16-QAM: Four bits per symbol, higher data rate but more susceptible to noise.
  - Symbols are normalized to maintain unit average power, ensuring fair comparison across modulation types.

### 2.2 Channel Model
- **AWGN Channel** (`AWGN.cpp`):
  - Assumes an additive white Gaussian noise channel, a standard model in communication theory where noise is independent, identically distributed, and Gaussian.
  - Noise power is adjusted dynamically based on the signal power and target SNR, ensuring the simulation reflects realistic channel conditions.
- **Coding Model** (`ChannelModel.cpp`):
  - Implements a 1/2 rate convolutional code with generator polynomials (7, 5), a common choice for error correction in digital communications.
  - The simplified Viterbi decoder assumes hard decisions, which may underestimate performance compared to a full soft-decision Viterbi algorithm.

### 2.3 Noise Model
- **Gaussian Noise**:
  - Uses the Box-Muller transform to generate Gaussian noise, which is statistically accurate for modeling thermal noise in communication channels.
  - Noise is bandlimited by the user-specified bandwidth, affecting the Eb/N0 calculation and zero-crossing analysis.
- **Power Adjustment** (`SignalToNoiseRatio.cpp`):
  - Noise power is computed as `signalPower / 10^(SNR_dB/10)`, ensuring the target SNR is achieved.
  - Signal power is calculated as the mean squared value of the signal, consistent with standard signal processing definitions.

## Utilization of Algorithms
The simulation employs several algorithms to implement the communication system and analyze its performance.

### 3.1 Random Number Generation
- **Algorithm**: Mersenne Twister (`std::mt19937`) for generating random bits and noise.
- **Usage**: Ensures reproducible random sequences for bits (`main.cpp`) and Gaussian noise (`AWGN.cpp`, `Analyzer.cpp`).
- **Rationale**: Mersenne Twister provides high-quality pseudorandom numbers with a long period, suitable for communication simulations.

### 3.2 Box-Muller Transform
- **Algorithm**: Generates standard normal random variables from uniform distributions using:
  - `z = sqrt(-2 * log(u1)) * cos(2 * π * u2)`
- **Usage**: In `AWGN.cpp` for noise generation and `Analyzer.cpp` for phasor statistics.
- **Rationale**: Efficiently produces Gaussian-distributed noise, critical for AWGN channel modeling.

### 3.3 Convolutional Coding
- **Algorithm**: 1/2 rate convolutional encoder with generator polynomials (7, 5).
- **Usage**: In `ChannelModel.cpp` to encode bits, producing two output bits per input bit.
- **Rationale**: Provides error correction, improving BER in noisy conditions, though the simplified Viterbi decoder limits performance.

### 3.4 Viterbi Decoding (Simplified)
- **Algorithm**: Hard-decision decoding by thresholding soft symbols and XORing paired bits.
- **Usage**: In `ChannelModel.cpp` to recover original bits from noisy symbols.
- **Rationale**: Simplifies implementation but sacrifices accuracy compared to a full Viterbi algorithm with soft decisions and path metrics.

### 3.5 Zero Crossing Detection
- **Algorithm**: Identifies points where the noisy signal changes sign (positive to negative or vice versa).
- **Usage**: In `Analyzer.cpp` for `computeZeroCrossingPoints` and `computeZeroCrossings`.
- **Rationale**: Helps analyze signal integrity and noise impact, particularly for time-domain analysis.

## Utilization of Physics Models

### 4.1 AWGN Channel Model
- **Physics Basis**: Models thermal noise in communication channels as white (flat spectrum) and Gaussian-distributed, consistent with the central limit theorem for thermal noise sources.
- **Implementation**: Noise is added to each signal sample, with variance determined by the target SNR and signal power.
- **Accuracy**: The model assumes ideal conditions (no fading, no interference), which is standard for baseline communication system analysis.

### 4.2 Signal Power and Noise Power
- **Physics Basis**: Signal power is the mean squared amplitude, and noise power is derived from the SNR, following the relationship:
  - `SNR = 10 * log10(signalPower / noisePower)`
- **Implementation**: `SignalToNoiseRatio.cpp` calculates signal power as the average of squared samples and adjusts noise power accordingly.
- **Accuracy**: Reflects real-world power calculations, though it assumes a normalized signal and ideal channel conditions.

### 4.3 Eb/N0 Calculation
- **Physics Basis**: Eb/N0 is the ratio of energy per bit to noise power spectral density, a key metric in digital communications:
  - `Eb = signalPower / bitRate`
  - `N0 = noisePower / bandwidth`
- **Implementation**: Computed in `SignalToNoiseRatio.cpp` using signal power, bit rate, and bandwidth.
- **Accuracy**: Aligns with communication theory, though the simulation assumes constant bit rate and bandwidth, ignoring dynamic channel effects.

### 4.4 Zero Crossing Rate
- **Physics Basis**: The zero-crossing rate of a noisy signal depends on the signal frequency, bandwidth, and SNR, derived from stochastic signal processing theory.
- **Implementation**: `Analyzer.cpp` uses the formula:
  - `frequency * sqrt((SNR_linear + 1 + (bandwidth^2)/(12*frequency^2)) / (SNR_linear + 1))`
- **Accuracy**: Captures the impact of noise and bandwidth on signal transitions, though it assumes a simplified signal model.

### 4.5 Phasor Statistics
- **Physics Basis**: Noise in the complex plane (for QPSK and 16-QAM) follows a bivariate Gaussian distribution, with magnitudes following a Rayleigh distribution.
- **Implementation**: `Analyzer.cpp` generates complex noise samples and computes the proportion within 1σ, 2σ, and 3σ circles.
- **Accuracy**: Reflects the statistical properties of AWGN in the complex domain, useful for visualizing noise distribution in modulation schemes.

## Screenshots

![](https://raw.githubusercontent.com/KMORaza/AWGN-Model/refs/heads/main/Additive%20White%20Gaussian%20Noise%20(AWGN)%20Model/002/screenshot.png)
