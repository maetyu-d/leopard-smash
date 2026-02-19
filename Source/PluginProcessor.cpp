#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int kDelayBufferSize = 8192;

float dbToLinear(float db)
{
    return juce::Decibels::decibelsToGain(db, -120.0f);
}

float linearToDb(float linear)
{
    return juce::Decibels::gainToDecibels(juce::jmax(1.0e-9f, linear), -120.0f);
}

float fastSign(float x)
{
    return x < 0.0f ? -1.0f : 1.0f;
}
} // namespace

const std::array<LeopardSmashAudioProcessor::FactoryPreset, LeopardSmashAudioProcessor::kNumFactoryPresets>&
LeopardSmashAudioProcessor::getFactoryPresets()
{
    static const std::array<FactoryPreset, kNumFactoryPresets> presets { {
        { "Concrete Control", -22.0f, 4.0f, 0.68f, 12.0f, 220.0f, false, false, 0.20f, 0.18f, 14, 0.08f, 0.10f, 0.20f, 8, 0.10f, 0.12f, 0.06f, 180.0f, 0.10f, 0.92f, 0.85f, 0.0f, 2, 0, 0 },
        { "Subquake Imploder", -34.0f, 18.0f, 0.92f, -2.5f, 920.0f, false, true, 0.72f, 0.80f, 4, 0.62f, 0.24f, 0.14f, 6, 0.24f, 0.74f, 0.10f, 720.0f, 0.65f, 0.36f, 0.28f, 0.40f, 1, 2, 1 },
        { "Cymbal Shredder", -28.0f, 12.0f, 0.88f, 0.5f, 520.0f, false, false, 0.62f, 0.34f, 5, 0.88f, 0.96f, 0.95f, 22, 0.56f, 0.48f, 0.72f, 410.0f, 0.46f, 0.35f, 0.44f, -0.22f, 0, 1, 2 },
        { "Snare Riot Bus", -26.0f, 15.0f, 0.82f, -4.0f, 160.0f, false, false, 0.70f, 0.75f, 6, 0.55f, 0.66f, 0.75f, 10, 0.72f, 0.58f, 0.48f, 330.0f, 0.38f, 0.50f, 0.35f, 0.10f, 1, 2, 3 },
        { "Fracture Steps", -14.0f, 7.0f, 0.75f, 3.0f, 70.0f, false, false, 0.92f, 0.48f, 2, 0.32f, 0.30f, 0.40f, 12, 0.22f, 0.18f, 0.30f, 110.0f, 0.95f, 0.12f, 0.82f, 0.0f, 2, 0, 0 },
        { "Sentient Drift", -36.0f, 2.0f, 0.80f, 95.0f, 3600.0f, true, true, 0.82f, 0.16f, 12, 0.34f, 0.82f, 0.54f, 24, 0.66f, 0.26f, 0.88f, 1400.0f, 0.38f, 0.90f, 0.66f, -0.50f, 0, 1, 4 },
        { "Reverse Vacuum", -30.0f, -3.2f, 0.58f, 8.0f, 2100.0f, false, true, 0.40f, 0.24f, 10, 0.20f, 0.22f, 0.15f, 5, 0.14f, 0.41f, 0.12f, 870.0f, 0.62f, 0.70f, 0.64f, 0.0f, 2, 0, 1 },
        { "Glitch Mono Crusher", -18.0f, 20.0f, 0.90f, -5.0f, 45.0f, false, false, 0.96f, 0.92f, 3, 0.92f, 0.12f, 0.18f, 4, 0.78f, 0.54f, 0.00f, 80.0f, 0.82f, 0.10f, 0.32f, 0.22f, 0, 1, 0 },
        { "Phase Predator", -33.0f, 16.0f, 0.97f, -1.5f, 980.0f, false, true, 0.88f, 0.62f, 4, 0.72f, 0.98f, 1.00f, 24, 0.68f, 0.70f, 1.00f, 880.0f, 0.58f, 0.22f, 0.22f, 0.35f, 0, 1, 2 },
        { "Dry Punch / Wet Wreck", -20.0f, 11.0f, 0.42f, 1.2f, 190.0f, false, false, 0.58f, 0.60f, 7, 0.58f, 0.72f, 0.38f, 16, 0.40f, 0.52f, 0.25f, 220.0f, 0.33f, 0.55f, 0.52f, -0.08f, 2, 2, 3 },
        { "Pump Tunnel", -42.0f, 5.0f, 0.70f, 52.0f, 2800.0f, true, false, 0.44f, 0.28f, 9, 0.56f, 0.50f, 0.46f, 14, 0.54f, 0.60f, 0.34f, 1500.0f, 0.52f, 0.78f, 0.36f, 0.58f, 0, 2, 4 },
        { "Zero Containment", -38.0f, 16.0f, 0.98f, -1.0f, 700.0f, false, true, 0.96f, 1.00f, 4, 0.78f, 0.70f, 0.84f, 24, 0.76f, 0.84f, 0.88f, 900.0f, 0.80f, 0.06f, 0.16f, 0.78f, 0, 1, 1 },
    } };

    return presets;
}

LeopardSmashAudioProcessor::LeopardSmashAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
      fft(kFftOrder)
{
    const auto& presets = getFactoryPresets();
    for (size_t i = 0; i < presets.size(); ++i)
        programNames[i] = presets[i].name;

    loadFactoryPreset(0, false);
}

LeopardSmashAudioProcessor::~LeopardSmashAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout LeopardSmashAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -24.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", juce::NormalisableRange<float>(-4.0f, 20.0f, 0.01f), 8.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("gateEnable", "Noise Gate", false));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gateThresh", "Gate Threshold", juce::NormalisableRange<float>(-80.0f, -10.0f, 0.01f), -52.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gateRelease", "Gate Release", juce::NormalisableRange<float>(5.0f, 500.0f, 0.01f), 120.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(-5.0f, 200.0f, 0.01f), 6.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(1.0f, 5000.0f, 0.01f), 180.0f));

    p.push_back(std::make_unique<juce::AudioParameterBool>("freezeEnv", "Freeze Envelope", false));
    p.push_back(std::make_unique<juce::AudioParameterBool>("reverseRel", "Reverse Release", false));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("chaos", "Chaos", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.35f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("fold", "Fold Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.4f));
    p.push_back(std::make_unique<juce::AudioParameterInt>("detectorRes", "Detector Resolution", 2, 16, 8));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("alias", "Aliasing Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.25f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("spectral", "Spectral Corruption", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("spectralMix", "Spectral Collapse Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.7f));
    p.push_back(std::make_unique<juce::AudioParameterInt>("fftBands", "FFT Bands", 4, 24, 12));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("warp", "Temporal Warp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.3f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback Instability", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.3f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("stereo", "Stereo Divergence", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("memory", "Memory", juce::NormalisableRange<float>(5.0f, 2000.0f, 0.01f), 260.0f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("hyst", "Hysteresis", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("seed", "Random Seed Stability", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.65f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("contain", "Containment", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.55f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("oscBias", "Oscillation Bias", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0001f), 0.0f));

    p.push_back(std::make_unique<juce::AudioParameterChoice>("oversampleQuality", "Oversampling Quality", juce::StringArray { "Off", "2x", "4x" }, 0));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("collapseMode", "Collapse Behavior", juce::StringArray { "Hold", "Rate Drop", "Hybrid" }, 2));

    p.push_back(std::make_unique<juce::AudioParameterChoice>("mode", "Mode",
                                                             juce::StringArray { "Fracture", "Implode", "Shrapnel", "Riot Bus", "Sentient" },
                                                             0));

    return { p.begin(), p.end() };
}

const juce::String LeopardSmashAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LeopardSmashAudioProcessor::acceptsMidi() const
{
    return false;
}

bool LeopardSmashAudioProcessor::producesMidi() const
{
    return false;
}

bool LeopardSmashAudioProcessor::isMidiEffect() const
{
    return false;
}

double LeopardSmashAudioProcessor::getTailLengthSeconds() const
{
    return 0.35;
}

int LeopardSmashAudioProcessor::getNumPrograms()
{
    return kNumFactoryPresets;
}

int LeopardSmashAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void LeopardSmashAudioProcessor::setCurrentProgram(int index)
{
    const int clamped = juce::jlimit(0, kNumFactoryPresets - 1, index);
    currentProgram = clamped;
    loadFactoryPreset(clamped, true);
}

const juce::String LeopardSmashAudioProcessor::getProgramName(int index)
{
    if (juce::isPositiveAndBelow(index, kNumFactoryPresets))
        return programNames[static_cast<size_t>(index)];

    return {};
}

void LeopardSmashAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (juce::isPositiveAndBelow(index, kNumFactoryPresets) && newName.isNotEmpty())
        programNames[static_cast<size_t>(index)] = newName;
}

void LeopardSmashAudioProcessor::loadFactoryPreset(int index, bool notifyHost)
{
    const auto& presets = getFactoryPresets();
    const auto& p = presets[static_cast<size_t>(juce::jlimit(0, kNumFactoryPresets - 1, index))];

    const auto applyValue = [this, notifyHost](const juce::String& id, float value)
    {
        if (auto* param = parameters.getParameter(id))
        {
            const float normalized = param->convertTo0to1(value);
            if (notifyHost)
                param->setValueNotifyingHost(normalized);
            else
                param->setValue(normalized);
        }
    };

    applyValue("threshold", p.threshold);
    applyValue("ratio", p.ratio);
    applyValue("mix", p.wetDryMix);
    applyValue("gateEnable", p.gateEnabled ? 1.0f : 0.0f);
    applyValue("gateThresh", p.gateThreshold);
    applyValue("gateRelease", p.gateReleaseMs);
    applyValue("attack", p.attack);
    applyValue("release", p.release);
    applyValue("freezeEnv", p.freezeEnv ? 1.0f : 0.0f);
    applyValue("reverseRel", p.reverseRel ? 1.0f : 0.0f);
    applyValue("chaos", p.chaos);
    applyValue("fold", p.fold);
    applyValue("detectorRes", static_cast<float>(p.detectorRes));
    applyValue("alias", p.alias);
    applyValue("spectral", p.spectral);
    applyValue("spectralMix", p.spectralMix);
    applyValue("fftBands", static_cast<float>(p.fftBands));
    applyValue("warp", p.warp);
    applyValue("feedback", p.feedback);
    applyValue("stereo", p.stereo);
    applyValue("memory", p.memory);
    applyValue("hyst", p.hyst);
    applyValue("seed", p.seed);
    applyValue("contain", p.contain);
    applyValue("oscBias", p.oscBias);
    applyValue("oversampleQuality", static_cast<float>(p.oversampleQuality));
    applyValue("collapseMode", static_cast<float>(p.collapseMode));
    applyValue("mode", static_cast<float>(p.mode));
}

void LeopardSmashAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    processingSampleRate = sampleRate;

    for (auto& st : channelStates)
    {
        st = {};
        st.delayLine.assign(kDelayBufferSize, 0.0f);
    }

    logisticState = rng.nextFloat() * 0.8f + 0.1f;
    detectorMemory = 0.0f;
    gainSmooth = 1.0f;
    gateSmooth = 1.0f;
    sentientLfoPhase = 0.0f;
    correlationMemory = 0.0f;
    currentLatencySamples = kFftSize;
    setLatencySamples(currentLatencySamples);
    prevHostIn = { 0.0f, 0.0f };
}

void LeopardSmashAudioProcessor::releaseResources()
{
}

bool LeopardSmashAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

float LeopardSmashAudioProcessor::computeDetector(float inL, float inR, float prevL, float prevR, ChannelState& stL, ChannelState& stR)
{
    const float absL = std::abs(inL);
    const float absR = std::abs(inR);
    const float peak = juce::jmax(absL, absR);

    const float rmsCoeff = 0.995f;
    stL.envRms = rmsCoeff * stL.envRms + (1.0f - rmsCoeff) * inL * inL;
    stR.envRms = rmsCoeff * stR.envRms + (1.0f - rmsCoeff) * inR * inR;
    const float rms = std::sqrt(0.5f * (stL.envRms + stR.envRms));

    const float deriv = 0.5f * (std::abs(inL - prevL) + std::abs(inR - prevR));

    const float fluxL = std::abs(absL - stL.prevAbs);
    const float fluxR = std::abs(absR - stR.prevAbs);
    const float flux = 0.5f * (fluxL + fluxR);

    const float crest = peak / (rms + 1.0e-6f);

    const float corrNum = inL * inR;
    const float corrDen = (std::sqrt(inL * inL + 1.0e-9f) * std::sqrt(inR * inR + 1.0e-9f) + 1.0e-9f);
    const float corr = corrNum / corrDen;
    correlationMemory = 0.995f * correlationMemory + 0.005f * corr;

    stL.prevAbs = absL;
    stR.prevAbs = absR;

    const float corrChaos = 1.0f - std::abs(correlationMemory);
    return 0.29f * rms + 0.22f * peak + 0.18f * deriv + 0.14f * flux + 0.1f * juce::jlimit(0.0f, 4.0f, crest) + 0.07f * corrChaos;
}

float LeopardSmashAudioProcessor::mutateControl(float detector, float chaos, int bitDepth, float hyst, float memoryMs, float seedStability)
{
    const float memorySamples = juce::jmax(1.0f, memoryMs * 0.001f * static_cast<float>(processingSampleRate));
    const float memCoeff = std::exp(-1.0f / memorySamples);
    detectorMemory = memCoeff * detectorMemory + (1.0f - memCoeff) * detector;

    const float stableNoise = (rng.nextFloat() * 2.0f - 1.0f) * (1.0f - seedStability);
    detector += stableNoise * chaos * 0.15f;

    const float r = 3.55f + 0.44f * chaos;
    logisticState = juce::jlimit(0.001f, 0.999f, r * logisticState * (1.0f - logisticState));
    detector *= juce::jmap(logisticState, 0.25f, 1.65f);

    const int levels = 1 << juce::jlimit(2, 16, bitDepth);
    detector = std::round(detector * static_cast<float>(levels)) / static_cast<float>(levels);

    const float hysteresisBlend = 0.35f + hyst * 0.6f;
    detector = juce::jmap(hysteresisBlend, detector, detectorMemory);
    return juce::jmax(0.0f, detector);
}

float LeopardSmashAudioProcessor::computeGainLinear(float detector,
                                                    float thresholdDb,
                                                    float ratio,
                                                    float attackMs,
                                                    float releaseMs,
                                                    bool freezeEnvelope,
                                                    bool reverseRelease)
{
    const float inDb = linearToDb(detector + 1.0e-6f);
    float gainDb = 0.0f;

    if (ratio >= 0.0f)
    {
        if (inDb > thresholdDb)
        {
            const float safeRatio = juce::jmax(0.5f, ratio);
            const float outDb = thresholdDb + (inDb - thresholdDb) / safeRatio;
            gainDb = outDb - inDb;
        }
    }
    else
    {
        if (inDb > thresholdDb)
        {
            const float inverted = thresholdDb - (inDb - thresholdDb) * std::abs(ratio);
            gainDb = inverted - inDb;
        }
    }

    const float atkSamples = juce::jmax(1.0f, std::abs(attackMs) * 0.001f * static_cast<float>(processingSampleRate));
    const float relSamples = juce::jmax(1.0f, releaseMs * 0.001f * static_cast<float>(processingSampleRate));

    float target = dbToLinear(gainDb);
    target = juce::jlimit(0.0f, 2.0f, target);

    if (freezeEnvelope && inDb > thresholdDb)
        target = gainSmooth;

    if (reverseRelease && target > gainSmooth)
    {
        const float recovery = target - gainSmooth;
        target = juce::jmax(0.0f, gainSmooth - recovery * 0.75f);
    }

    const float a = std::exp(-1.0f / atkSamples);
    const float r = std::exp(-1.0f / relSamples);
    const float coeff = target < gainSmooth ? a : r;

    const float preTriggerBoost = attackMs < 0.0f ? juce::jmap(-attackMs, 0.0f, 5.0f, 1.0f, 1.35f) : 1.0f;
    gainSmooth = coeff * gainSmooth + (1.0f - coeff) * target;

    return juce::jlimit(0.0f, 2.0f, gainSmooth * preTriggerBoost);
}

float LeopardSmashAudioProcessor::applyWavefold(float x, float amount) const
{
    if (amount <= 0.0001f)
        return x;

    const float drive = 1.0f + 12.0f * amount;
    float y = x * drive;

    for (int i = 0; i < 3; ++i)
    {
        if (y > 1.0f)
            y = 2.0f - y;
        else if (y < -1.0f)
            y = -2.0f - y;
        else
            break;
    }

    return std::tanh(y * (1.0f + amount * 3.0f));
}

float LeopardSmashAudioProcessor::applyTemporalWarp(ChannelState& st, float sample, float warpAmount, float releaseMs, int channelIndex)
{
    if (st.delayLine.empty())
        return sample;

    st.delayLine[static_cast<size_t>(st.delayWrite)] = sample;

    const float maxMs = juce::jmap(warpAmount, 0.0f, 1.0f, 0.2f, 10.0f);
    const float releaseInfluence = juce::jlimit(0.0f, 1.0f, releaseMs / 5000.0f);
    const float jitter = (rng.nextFloat() * 2.0f - 1.0f) * maxMs * (0.4f + 0.6f * releaseInfluence);
    const float stereoOffset = channelIndex == 1 ? warpAmount * 1.7f : 0.0f;

    const int delaySamples = juce::jlimit(1, kDelayBufferSize - 2,
                                          static_cast<int>((maxMs + jitter + stereoOffset) * 0.001f * processingSampleRate));

    int read = st.delayWrite - delaySamples;
    while (read < 0)
        read += static_cast<int>(st.delayLine.size());

    const int repeatTap = (read - juce::jlimit(1, 96, static_cast<int>(warpAmount * 48.0f)) + kDelayBufferSize) % kDelayBufferSize;
    const float delayed = st.delayLine[static_cast<size_t>(read)];
    const float repeat = st.delayLine[static_cast<size_t>(repeatTap)] * (0.15f + warpAmount * 0.45f);

    st.delayWrite = (st.delayWrite + 1) % static_cast<int>(st.delayLine.size());
    return sample * (1.0f - warpAmount * 0.55f) + delayed * (warpAmount * 0.55f) + repeat;
}

float LeopardSmashAudioProcessor::applySpectralCorruption(ChannelState& st, float sample, float amount)
{
    if (amount <= 0.0001f)
        return sample;

    const float hp = sample - st.prevHi + 0.965f * st.prevOut;
    st.prevHi = sample;

    const float bins = juce::jmap(amount, 0.0f, 1.0f, 32.0f, 4.0f);
    const float quantHi = std::round(hp * bins) / bins;

    const float harmonic = std::sin(sample * juce::MathConstants<float>::twoPi * (2.0f + amount * 6.0f));
    const float rotated = 0.75f * quantHi + 0.25f * fastSign(quantHi) * std::abs(harmonic);

    const float out = sample * (1.0f - amount * 0.5f) + rotated * (amount * 0.7f);
    st.prevOut = out;
    return out;
}

float LeopardSmashAudioProcessor::applyFftSpectralCollapse(ChannelState& st, float sample, float amount, int bands)
{
    const int idx = st.fftIndex;
    const float out = st.fftOutput[static_cast<size_t>(idx)];

    st.fftInput[static_cast<size_t>(idx)] = sample;
    st.fftOutput[static_cast<size_t>(idx)] = 0.0f;
    st.fftCollapseAccum += amount;
    ++st.fftCollapseCount;

    st.fftIndex = (st.fftIndex + 1) % kFftSize;
    if (st.fftIndex != 0)
        return out;

    const float frameAmount = st.fftCollapseCount > 0
        ? juce::jlimit(0.0f, 1.0f, st.fftCollapseAccum / static_cast<float>(st.fftCollapseCount))
        : 0.0f;

    st.fftCollapseAccum = 0.0f;
    st.fftCollapseCount = 0;

    if (frameAmount <= 0.0001f)
    {
        st.fftOutput = st.fftInput;
        return out;
    }

    for (int i = 0; i < kFftSize; ++i)
    {
        st.fftData[static_cast<size_t>(i)] = st.fftInput[static_cast<size_t>(i)];
        st.fftData[static_cast<size_t>(i + kFftSize)] = 0.0f;
    }

    fft.performRealOnlyForwardTransform(st.fftData.data());

    const int maxBin = kFftSize / 2;
    const int safeBands = juce::jlimit(4, 24, bands);
    const float harmonicBase = juce::jmap(frameAmount, 2.0f, 14.0f);

    for (int bin = 0; bin <= maxBin; ++bin)
    {
        const float re = st.fftData[static_cast<size_t>(2 * bin)];
        const float im = st.fftData[static_cast<size_t>(2 * bin + 1)];
        st.magBuffer[static_cast<size_t>(bin)] = std::sqrt(re * re + im * im);
    }

    for (int bin = 1; bin < maxBin; ++bin)
    {
        float re = st.fftData[static_cast<size_t>(2 * bin)];
        float im = st.fftData[static_cast<size_t>(2 * bin + 1)];

        float mag = std::sqrt(re * re + im * im);
        float phase = std::atan2(im, re);

        const int band = juce::jlimit(0, safeBands - 1, (bin * safeBands) / juce::jmax(1, maxBin));
        const float bandNorm = static_cast<float>(band) / static_cast<float>(juce::jmax(1, safeBands - 1));

        const float magSteps = juce::jmap(frameAmount * (0.5f + bandNorm), 256.0f, 10.0f);
        mag = std::round(mag * magSteps) / juce::jmax(1.0f, magSteps);

        if (band >= safeBands / 2)
        {
            const float phaseJitter = (rng.nextFloat() * 2.0f - 1.0f)
                * frameAmount
                * juce::MathConstants<float>::pi
                * (0.35f + bandNorm);
            phase += phaseJitter;
        }

        const int targetBin = juce::jlimit(1, maxBin - 1,
                                           static_cast<int>(std::round(static_cast<float>(bin) / harmonicBase) * harmonicBase));
        const float targetMag = st.magBuffer[static_cast<size_t>(targetBin)];
        mag = juce::jmap(frameAmount * 0.6f, mag, targetMag);

        st.fftData[static_cast<size_t>(2 * bin)] = mag * std::cos(phase);
        st.fftData[static_cast<size_t>(2 * bin + 1)] = mag * std::sin(phase);
    }

    fft.performRealOnlyInverseTransform(st.fftData.data());

    const float norm = 1.0f / static_cast<float>(kFftSize);
    for (int i = 0; i < kFftSize; ++i)
        st.fftOutput[static_cast<size_t>(i)] = st.fftData[static_cast<size_t>(i)] * norm;

    return out;
}

float LeopardSmashAudioProcessor::delayDryForSpectralMix(ChannelState& st, float sample)
{
    const int idx = st.dryDelayIndex;
    const float delayed = st.dryDelay[static_cast<size_t>(idx)];
    st.dryDelay[static_cast<size_t>(idx)] = sample;
    st.dryDelayIndex = (st.dryDelayIndex + 1) % kFftSize;
    return delayed;
}

void LeopardSmashAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto* l = buffer.getWritePointer(0);
    auto* r = buffer.getWritePointer(1);

    const float threshold = parameters.getRawParameterValue("threshold")->load();
    const float ratio = parameters.getRawParameterValue("ratio")->load();
    const float wetDryMix = parameters.getRawParameterValue("mix")->load();
    const bool gateEnabled = parameters.getRawParameterValue("gateEnable")->load() > 0.5f;
    const float gateThresholdDb = parameters.getRawParameterValue("gateThresh")->load();
    const float gateReleaseMs = parameters.getRawParameterValue("gateRelease")->load();
    const float attack = parameters.getRawParameterValue("attack")->load();
    const float release = parameters.getRawParameterValue("release")->load();

    const bool freezeEnv = parameters.getRawParameterValue("freezeEnv")->load() > 0.5f;
    const bool reverseRel = parameters.getRawParameterValue("reverseRel")->load() > 0.5f;

    float chaos = parameters.getRawParameterValue("chaos")->load();
    float foldDepth = parameters.getRawParameterValue("fold")->load();
    int detectorRes = juce::roundToInt(parameters.getRawParameterValue("detectorRes")->load());
    float aliasing = parameters.getRawParameterValue("alias")->load();
    float spectral = parameters.getRawParameterValue("spectral")->load();
    const float spectralMix = parameters.getRawParameterValue("spectralMix")->load();
    const int fftBands = juce::roundToInt(parameters.getRawParameterValue("fftBands")->load());
    float warp = parameters.getRawParameterValue("warp")->load();
    float feedback = parameters.getRawParameterValue("feedback")->load();
    float stereoDiv = parameters.getRawParameterValue("stereo")->load();
    float memory = parameters.getRawParameterValue("memory")->load();

    float hyst = parameters.getRawParameterValue("hyst")->load();
    float seedStability = parameters.getRawParameterValue("seed")->load();
    float containment = parameters.getRawParameterValue("contain")->load();
    float oscBias = parameters.getRawParameterValue("oscBias")->load();

    const int oversampleChoice = juce::roundToInt(parameters.getRawParameterValue("oversampleQuality")->load());
    const int collapseMode = juce::roundToInt(parameters.getRawParameterValue("collapseMode")->load());

    const auto mode = static_cast<Mode>(juce::roundToInt(parameters.getRawParameterValue("mode")->load()));

    if (mode == Mode::fracture)
    {
        detectorRes = juce::jmin(detectorRes, 6);
        hyst += 0.2f;
    }
    else if (mode == Mode::implode)
    {
        feedback = juce::jmin(1.0f, feedback + 0.35f);
        containment *= 0.65f;
    }
    else if (mode == Mode::shrapnel)
    {
        spectral = juce::jmin(1.0f, spectral + 0.35f);
        aliasing = juce::jmin(1.0f, aliasing + 0.25f);
    }
    else if (mode == Mode::riotBus)
    {
        foldDepth = juce::jmin(1.0f, foldDepth + 0.25f);
        feedback = juce::jmin(1.0f, feedback + 0.2f);
        warp = juce::jmin(1.0f, warp + 0.2f);
    }
    else if (mode == Mode::sentient)
    {
        const float hz = 0.03f;
        const float phaseInc = hz / static_cast<float>(currentSampleRate);
        sentientLfoPhase += phaseInc;
        if (sentientLfoPhase > 1.0f)
            sentientLfoPhase -= 1.0f;

        const float evolve = 0.5f + 0.5f * std::sin(sentientLfoPhase * juce::MathConstants<float>::twoPi);
        chaos = juce::jlimit(0.0f, 1.0f, chaos * (0.8f + 0.4f * evolve));
        spectral = juce::jlimit(0.0f, 1.0f, spectral * (0.6f + 0.8f * (1.0f - evolve)));
        warp = juce::jlimit(0.0f, 1.0f, warp * (0.6f + 0.8f * evolve));
    }

    const int oversampleFactor = oversampleChoice == 2 ? 4 : (oversampleChoice == 1 ? 2 : 1);
    processingSampleRate = currentSampleRate * static_cast<double>(oversampleFactor);
    const float gateThresholdLin = dbToLinear(gateThresholdDb + 8.0f);
    const float gateAttackCoeff = std::exp(-1.0f / juce::jmax(1.0f, 0.0002f * static_cast<float>(currentSampleRate)));
    const float gateReleaseCoeff = std::exp(-1.0f / juce::jmax(1.0f, gateReleaseMs * 0.00025f * static_cast<float>(currentSampleRate)));
    const int latencySamples = juce::jmax(0, kFftSize / oversampleFactor);
    if (latencySamples != currentLatencySamples)
    {
        currentLatencySamples = latencySamples;
        setLatencySamples(currentLatencySamples);
    }

    auto processInternalSample = [&](float inL, float inR, bool freezeEnabled, bool reverseEnabled)
    {
        const float transientEnergy = juce::jlimit(0.0f, 1.0f,
                                                   0.5f * (std::abs(inL - channelStates[0].prevIn)
                                                           + std::abs(inR - channelStates[1].prevIn)) * 24.0f);

        const float detectorPreview = 0.5f * (std::abs(inL) + std::abs(inR));
        const float silenceFactor = juce::jlimit(0.0f, 1.0f, detectorPreview * 10.0f);
        const float rhythmicFeedback = feedback * (0.20f + 0.80f * silenceFactor);

        const float fbL = std::tanh(channelStates[0].prevOut * (1.0f + 3.0f * rhythmicFeedback));
        const float fbR = std::tanh(channelStates[1].prevOut * (1.0f + 3.0f * rhythmicFeedback));

        const float containmentComp = juce::jmap(containment, 0.0f, 1.0f, 1.5f, 0.25f);
        inL += fbL * rhythmicFeedback * containmentComp * (1.0f + oscBias * 0.4f);
        inR += fbR * rhythmicFeedback * containmentComp * (1.0f - oscBias * 0.4f);

        const float detector = computeDetector(inL, inR,
                                               channelStates[0].prevIn,
                                               channelStates[1].prevIn,
                                               channelStates[0],
                                               channelStates[1]);

        float control = mutateControl(detector, chaos, detectorRes, hyst, memory, seedStability);
        control *= 1.0f + (rng.nextFloat() * 2.0f - 1.0f) * chaos * 0.1f;

        const float gain = computeGainLinear(control, threshold, ratio, attack, release, freezeEnabled, reverseEnabled);
        const float gr = juce::jlimit(0.0f, 1.0f, 1.0f - gain);

        float yL = inL * gain * (1.0f + stereoDiv * 0.5f);
        float yR = inR * gain * (1.0f - stereoDiv * 0.5f);

        const float bias = (control - 0.1f) * (0.8f + chaos * 1.2f);
        yL += bias * 0.12f;
        yR -= bias * 0.12f;

        const float foldAmt = foldDepth * (0.4f + 1.4f * gr);
        yL = applyWavefold(yL, foldAmt * (1.0f + chaos * 0.3f));
        yR = applyWavefold(yR, foldAmt * (1.0f - chaos * 0.2f));

        const float asym = 1.0f + chaos * 0.9f;
        yL = yL > 0.0f ? yL * asym : yL * (2.0f - asym);
        yR = yR > 0.0f ? yR * (2.0f - asym * 0.7f) : yR * asym * 0.9f;

        const float transientSafe = 1.0f - transientEnergy * 0.68f;
        const float collapse = aliasing * juce::jlimit(0.0f, 1.0f, gr * 1.4f) * transientSafe;
        for (int ch = 0; ch < 2; ++ch)
        {
            auto& st = channelStates[static_cast<size_t>(ch)];
            float& s = ch == 0 ? yL : yR;
            const int holdLen = juce::jlimit(1, 64, static_cast<int>(1.0f + collapse * 63.0f));

            if (collapseMode == 0)
            {
                if (st.holdCounter <= 0)
                {
                    st.heldSample = s;
                    st.holdCounter = holdLen;
                }
                else
                {
                    s = juce::jmap(collapse, s, st.heldSample);
                    --st.holdCounter;
                }
            }
            else if (collapseMode == 1)
            {
                if (st.holdCounter <= 0)
                {
                    st.heldSample = s;
                    st.holdCounter = holdLen;
                }
                else
                {
                    s = st.heldSample;
                    --st.holdCounter;
                }
            }
            else
            {
                if (st.holdCounter <= 0)
                {
                    st.heldSample = s;
                    st.holdCounter = holdLen;
                }
                else
                {
                    const float hard = st.heldSample;
                    const float soft = juce::jmap(collapse, s, st.heldSample);
                    s = juce::jmap(collapse, soft, hard);
                    --st.holdCounter;
                }
            }
        }

        const float warpSafe = warp * (0.22f + 0.78f * (1.0f - transientEnergy * 0.72f));
        yL = applyTemporalWarp(channelStates[0], yL, warpSafe * (0.9f + 0.2f * stereoDiv), release, 0);
        yR = applyTemporalWarp(channelStates[1], yR, warpSafe * (0.9f - 0.2f * stereoDiv), release, 1);

        yL = applySpectralCorruption(channelStates[0], yL, spectral * 0.5f);
        yR = applySpectralCorruption(channelStates[1], yR, spectral * (0.5f + stereoDiv * 0.2f));

        const float preFftL = yL;
        const float preFftR = yR;

        const float fftDrive = spectral * juce::jlimit(0.0f, 1.0f, 0.4f + gr * 1.2f) * transientSafe;
        const float wetL = applyFftSpectralCollapse(channelStates[0], yL, fftDrive, fftBands);
        const float wetR = applyFftSpectralCollapse(channelStates[1], yR, fftDrive * (1.0f + stereoDiv * 0.15f), fftBands);
        const float dryDelayedL = delayDryForSpectralMix(channelStates[0], preFftL);
        const float dryDelayedR = delayDryForSpectralMix(channelStates[1], preFftR);

        yL = juce::jmap(spectralMix, dryDelayedL, wetL);
        yR = juce::jmap(spectralMix, dryDelayedR, wetR);

        // Preserve per-hit definition by reinjecting a bit of dry transient.
        const float transientBlend = 0.30f * transientEnergy;
        yL = juce::jmap(transientBlend, yL, inL);
        yR = juce::jmap(transientBlend, yR, inR);

        // Anti-drone behavior: when input energy collapses, rapidly damp self-oscillation memory.
        if (silenceFactor < 0.16f)
        {
            for (auto& st : channelStates)
            {
                st.prevOut *= 0.64f;
                st.heldSample *= 0.54f;
            }
            gainSmooth = juce::jmap(0.18f, gainSmooth, 1.0f);
        }

        const float safeContain = juce::jmap(containment, 0.0f, 1.0f, 0.45f, 1.0f);
        yL = juce::jlimit(-1.2f, 1.2f, yL * safeContain);
        yR = juce::jlimit(-1.2f, 1.2f, yR * safeContain);

        channelStates[0].prevIn = inL;
        channelStates[1].prevIn = inR;
        channelStates[0].prevOut = yL;
        channelStates[1].prevOut = yR;

        return std::pair<float, float> { yL, yR };
    };

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float hostL = l[i];
        const float hostR = r[i];

        float outL = 0.0f;
        float outR = 0.0f;

        for (int os = 0; os < oversampleFactor; ++os)
        {
            const float alpha = static_cast<float>(os + 1) / static_cast<float>(oversampleFactor);
            const float inL = juce::jmap(alpha, prevHostIn[0], hostL);
            const float inR = juce::jmap(alpha, prevHostIn[1], hostR);

            const auto y = processInternalSample(inL, inR, freezeEnv, reverseRel);
            outL += y.first;
            outR += y.second;
        }

        l[i] = outL / static_cast<float>(oversampleFactor);
        r[i] = outR / static_cast<float>(oversampleFactor);

        auto blendWithLatencyAlignedDry = [&](ChannelState& st, float dryNow, float wetNow)
        {
            st.hostDryDelay[static_cast<size_t>(st.hostDryWrite)] = dryNow;
            int read = st.hostDryWrite - currentLatencySamples;
            while (read < 0)
                read += kFftSize;
            const float dryAligned = st.hostDryDelay[static_cast<size_t>(read)];
            return juce::jmap(wetDryMix, dryAligned, wetNow);
        };

        const float wetL = l[i];
        const float wetR = r[i];
        l[i] = blendWithLatencyAlignedDry(channelStates[0], hostL, wetL);
        r[i] = blendWithLatencyAlignedDry(channelStates[1], hostR, wetR);

        if (gateEnabled)
        {
            const float gateDetector = juce::jmax(std::abs(hostL), std::abs(hostR));
            const float target = gateDetector > gateThresholdLin ? 1.0f : 0.0f;
            const float coeff = target > gateSmooth ? gateAttackCoeff : gateReleaseCoeff;
            gateSmooth = coeff * gateSmooth + (1.0f - coeff) * target;
            float shaped = gateSmooth * gateSmooth * gateSmooth;
            if (shaped < 0.02f)
                shaped = 0.0f;
            l[i] *= shaped;
            r[i] *= shaped;
        }
        else
        {
            gateSmooth = 0.97f * gateSmooth + 0.03f;
        }

        channelStates[0].hostDryWrite = (channelStates[0].hostDryWrite + 1) % kFftSize;
        channelStates[1].hostDryWrite = (channelStates[1].hostDryWrite + 1) % kFftSize;

        prevHostIn[0] = hostL;
        prevHostIn[1] = hostR;
    }
}

bool LeopardSmashAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LeopardSmashAudioProcessor::createEditor()
{
    return new LeopardSmashAudioProcessorEditor(*this);
}

void LeopardSmashAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.state.createXml())
        copyXmlToBinary(*xml, destData);
}

void LeopardSmashAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LeopardSmashAudioProcessor();
}
