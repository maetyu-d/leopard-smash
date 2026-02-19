#pragma once

#include <JuceHeader.h>

class LeopardSmashAudioProcessor final : public juce::AudioProcessor
{
public:
    LeopardSmashAudioProcessor();
    ~LeopardSmashAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS& getAPVTS() noexcept { return parameters; }

    static APVTS::ParameterLayout createParameterLayout();

private:
    static constexpr int kFftOrder = 10;
    static constexpr int kFftSize = 1 << kFftOrder;
    static constexpr int kNumFactoryPresets = 12;

    enum class Mode : int
    {
        fracture = 0,
        implode,
        shrapnel,
        riotBus,
        sentient
    };

    struct ChannelState
    {
        float envPeak = 0.0f;
        float envRms = 0.0f;
        float prevIn = 0.0f;
        float prevAbs = 0.0f;
        float prevHi = 0.0f;
        float prevOut = 0.0f;
        float heldSample = 0.0f;
        int holdCounter = 0;
        float memDetector = 0.0f;
        std::vector<float> delayLine;
        int delayWrite = 0;

        std::array<float, kFftSize> fftInput {};
        std::array<float, kFftSize> fftOutput {};
        std::array<float, kFftSize * 2> fftData {};
        std::array<float, (kFftSize / 2) + 1> magBuffer {};
        std::array<float, kFftSize> dryDelay {};
        std::array<float, kFftSize> hostDryDelay {};
        int fftIndex = 0;
        int dryDelayIndex = 0;
        int hostDryWrite = 0;
        float fftCollapseAccum = 0.0f;
        int fftCollapseCount = 0;
    };

    struct FactoryPreset
    {
        const char* name = "";
        float threshold = -24.0f;
        float ratio = 8.0f;
        float wetDryMix = 1.0f;
        float attack = 6.0f;
        float release = 180.0f;
        bool freezeEnv = false;
        bool reverseRel = false;
        float chaos = 0.35f;
        float fold = 0.4f;
        int detectorRes = 8;
        float alias = 0.25f;
        float spectral = 0.2f;
        float spectralMix = 0.7f;
        int fftBands = 12;
        float warp = 0.3f;
        float feedback = 0.3f;
        float stereo = 0.2f;
        float memory = 260.0f;
        float hyst = 0.2f;
        float seed = 0.65f;
        float contain = 0.55f;
        float oscBias = 0.0f;
        int oversampleQuality = 0;
        int collapseMode = 2;
        int mode = 0;
        bool gateEnabled = false;
        float gateThreshold = -52.0f;
        float gateReleaseMs = 120.0f;
    };

    float computeDetector(float inL, float inR, float prevL, float prevR, ChannelState& stL, ChannelState& stR);
    float mutateControl(float detector, float chaos, int bitDepth, float hyst, float memoryMs, float seedStability);
    float computeGainLinear(float detector,
                            float thresholdDb,
                            float ratio,
                            float attackMs,
                            float releaseMs,
                            bool freezeEnvelope,
                            bool reverseRelease);
    float applyWavefold(float x, float amount) const;
    float applyTemporalWarp(ChannelState& st, float sample, float warpAmount, float releaseMs, int channelIndex);
    float applySpectralCorruption(ChannelState& st, float sample, float amount);
    float applyFftSpectralCollapse(ChannelState& st, float sample, float amount, int bands);
    float delayDryForSpectralMix(ChannelState& st, float sample);
    static const std::array<FactoryPreset, kNumFactoryPresets>& getFactoryPresets();
    void loadFactoryPreset(int index, bool notifyHost);

    juce::AudioProcessorValueTreeState parameters;

    std::array<ChannelState, 2> channelStates;
    juce::Random rng;
    juce::dsp::FFT fft;
    double currentSampleRate = 44100.0;
    double processingSampleRate = 44100.0;
    float logisticState = 0.37f;
    float detectorMemory = 0.0f;
    float gainSmooth = 1.0f;
    float gateSmooth = 1.0f;
    float sentientLfoPhase = 0.0f;
    float correlationMemory = 0.0f;
    int currentLatencySamples = 0;
    int currentProgram = 0;
    std::array<juce::String, kNumFactoryPresets> programNames;
    std::array<float, 2> prevHostIn { 0.0f, 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeopardSmashAudioProcessor)
};
