#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LeopardSmashAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit LeopardSmashAudioProcessorEditor(LeopardSmashAudioProcessor&);
    ~LeopardSmashAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void setupKnob(Knob& knob, const juce::String& paramId, const juce::String& labelText);

    LeopardSmashAudioProcessor& audioProcessor;

    juce::Label title;
    juce::ComboBox modeBox;
    juce::ComboBox oversampleBox;
    juce::ComboBox collapseBox;
    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<ComboAttachment> oversampleAttachment;
    std::unique_ptr<ComboAttachment> collapseAttachment;

    juce::ToggleButton freezeEnvButton;
    juce::ToggleButton reverseReleaseButton;
    juce::ToggleButton gateButton;
    std::unique_ptr<ButtonAttachment> freezeAttachment;
    std::unique_ptr<ButtonAttachment> reverseAttachment;
    std::unique_ptr<ButtonAttachment> gateAttachment;

    Knob threshold;
    Knob ratio;
    Knob mix;
    Knob attack;
    Knob release;
    Knob chaos;
    Knob fold;
    Knob detectorRes;
    Knob aliasing;
    Knob spectral;
    Knob spectralMix;
    Knob fftBands;
    Knob warp;
    Knob feedback;
    Knob stereo;
    Knob memory;
    Knob hyst;
    Knob seed;
    Knob contain;
    Knob oscBias;
    Knob gateThresh;
    Knob gateRelease;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeopardSmashAudioProcessorEditor)
};
