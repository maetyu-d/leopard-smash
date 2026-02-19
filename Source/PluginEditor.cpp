#include "PluginEditor.h"

LeopardSmashAudioProcessorEditor::LeopardSmashAudioProcessorEditor(LeopardSmashAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(1080, 620);

    title.setText("LEOPARD SMASH!", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::FontOptions("Menlo", 34.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colour(0xff111111));
    addAndMakeVisible(title);

    addAndMakeVisible(modeBox);
    modeBox.addItemList({ "Fracture", "Implode", "Shrapnel", "Riot Bus", "Sentient" }, 1);
    modeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xffb8b8b2));
    modeBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xff101010));
    modeBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff101010));
    modeBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff101010));
    modeAttachment = std::make_unique<ComboAttachment>(audioProcessor.getAPVTS(), "mode", modeBox);

    addAndMakeVisible(oversampleBox);
    oversampleBox.addItemList({ "Off", "2x", "4x" }, 1);
    oversampleBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xffb8b8b2));
    oversampleBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xff101010));
    oversampleBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff101010));
    oversampleBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff101010));
    oversampleAttachment = std::make_unique<ComboAttachment>(audioProcessor.getAPVTS(), "oversampleQuality", oversampleBox);

    addAndMakeVisible(collapseBox);
    collapseBox.addItemList({ "Hold", "Rate Drop", "Hybrid" }, 1);
    collapseBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xffb8b8b2));
    collapseBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xff101010));
    collapseBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff101010));
    collapseBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff101010));
    collapseAttachment = std::make_unique<ComboAttachment>(audioProcessor.getAPVTS(), "collapseMode", collapseBox);

    freezeEnvButton.setButtonText("Freeze Envelope");
    freezeEnvButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff1a1a1a));
    addAndMakeVisible(freezeEnvButton);
    freezeAttachment = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "freezeEnv", freezeEnvButton);

    reverseReleaseButton.setButtonText("Reverse Release");
    reverseReleaseButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff1a1a1a));
    addAndMakeVisible(reverseReleaseButton);
    reverseAttachment = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "reverseRel", reverseReleaseButton);

    gateButton.setButtonText("Noise Gate");
    gateButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff1a1a1a));
    addAndMakeVisible(gateButton);
    gateAttachment = std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), "gateEnable", gateButton);

    setupKnob(threshold, "threshold", "Threshold");
    setupKnob(ratio, "ratio", "Ratio");
    setupKnob(mix, "mix", "Mix");
    setupKnob(attack, "attack", "Attack");
    setupKnob(release, "release", "Release");

    setupKnob(chaos, "chaos", "Chaos");
    setupKnob(fold, "fold", "Fold");
    setupKnob(detectorRes, "detectorRes", "Detector Res");
    setupKnob(aliasing, "alias", "Aliasing");
    setupKnob(spectral, "spectral", "Spectral");
    setupKnob(spectralMix, "spectralMix", "Spectral Mix");
    setupKnob(fftBands, "fftBands", "FFT Bands");
    setupKnob(warp, "warp", "Temporal Warp");
    setupKnob(feedback, "feedback", "Feedback Instability");
    setupKnob(stereo, "stereo", "Stereo Divergence");
    setupKnob(memory, "memory", "Memory");

    setupKnob(hyst, "hyst", "Hysteresis");
    setupKnob(seed, "seed", "Seed Stability");
    setupKnob(contain, "contain", "Containment");
    setupKnob(oscBias, "oscBias", "Osc Bias");
    setupKnob(gateThresh, "gateThresh", "Gate Thresh");
    setupKnob(gateRelease, "gateRelease", "Gate Release");
}

LeopardSmashAudioProcessorEditor::~LeopardSmashAudioProcessorEditor() = default;

void LeopardSmashAudioProcessorEditor::setupKnob(Knob& knob, const juce::String& paramId, const juce::String& labelText)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    knob.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff303030));
    knob.slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff0f0f0f));
    knob.slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd24e12));
    knob.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffadada5));
    knob.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff101010));
    knob.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff111111));
    addAndMakeVisible(knob.slider);

    knob.label.setText(labelText, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setFont(juce::FontOptions("Menlo", 11.0f, juce::Font::bold));
    knob.label.setColour(juce::Label::textColourId, juce::Colour(0xff141414));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), paramId, knob.slider);
}

void LeopardSmashAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff8f9187));

    for (int y = 0; y < getHeight(); y += 3)
    {
        const auto shade = juce::jlimit(0, 255, 138 + ((y / 3) % 5) - 2);
        g.setColour(juce::Colour::fromRGB(static_cast<juce::uint8>(shade),
                                          static_cast<juce::uint8>(shade),
                                          static_cast<juce::uint8>(shade - 2)));
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
    }

    const auto outer = getLocalBounds().reduced(10);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(outer);
    g.setColour(juce::Colour(0xffb5b7ae));
    g.fillRect(outer.reduced(5));
    g.setColour(juce::Colour(0xff202020));
    g.drawRect(outer, 3);

    const auto panel = juce::Rectangle<int>(18, 100, getWidth() - 36, getHeight() - 116);
    g.setColour(juce::Colour(0xffa4a59c));
    g.fillRect(panel);
    g.setColour(juce::Colour(0xff111111));
    g.drawRect(panel, 4);

    for (int x = panel.getX(); x < panel.getRight(); x += 52)
    {
        g.setColour(juce::Colour(0x15000000));
        g.drawVerticalLine(x, static_cast<float>(panel.getY()), static_cast<float>(panel.getBottom()));
    }

    g.setColour(juce::Colour(0xffd24e12));
    g.fillRect(22, 18, getWidth() - 44, 42);
    g.setColour(juce::Colour(0xff131313));
    g.drawRect(22, 18, getWidth() - 44, 42, 3);

    const int gutterX = 146;
    g.setColour(juce::Colour(0x33000000));
    g.drawLine(static_cast<float>(gutterX), 100.0f, static_cast<float>(gutterX), static_cast<float>(getHeight() - 18), 2.0f);

    g.setColour(juce::Colour(0xff1b1b1b));
    g.setFont(juce::FontOptions("Menlo", 12.0f, juce::Font::bold));
    g.drawText("MODE", 24, 67, 70, 16, juce::Justification::left);
    g.drawText("QUALITY", 210, 67, 84, 16, juce::Justification::left);
    g.drawText("COLLAPSE", 372, 67, 90, 16, juce::Justification::left);

    g.setColour(juce::Colour(0xff1b1b1b));
    auto drawVerticalLabel = [&](const juce::String& text, float cx, float cy)
    {
        juce::Graphics::ScopedSaveState state(g);
        g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, cx, cy));
        g.drawText(text, static_cast<int>(cx - 70.0f), static_cast<int>(cy - 10.0f), 140, 20, juce::Justification::centred);
    };
    drawVerticalLabel("CORE", 50.0f, 186.0f);
    drawVerticalLabel("SPECTRAL", 50.0f, 326.0f);
    drawVerticalLabel("MUTATION", 50.0f, 466.0f);

    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawLine(18.0f, 260.0f, static_cast<float>(getWidth() - 18), 260.0f, 2.0f);
    g.drawLine(18.0f, 400.0f, static_cast<float>(getWidth() - 18), 400.0f, 2.0f);
}

void LeopardSmashAudioProcessorEditor::resized()
{
    title.setBounds(22, 19, getWidth() - 44, 38);

    modeBox.setBounds(64, 68, 132, 24);
    oversampleBox.setBounds(260, 68, 100, 24);
    collapseBox.setBounds(430, 68, 122, 24);

    gateButton.setBounds(getWidth() - 396, 68, 120, 24);
    freezeEnvButton.setBounds(getWidth() - 270, 68, 120, 24);
    reverseReleaseButton.setBounds(getWidth() - 144, 68, 120, 24);

    auto placeRow = [this](int y, std::initializer_list<Knob*> knobs)
    {
        const int x0 = 158;
        const int gap = 6;
        const int count = static_cast<int>(knobs.size());
        const int w = (getWidth() - x0 - 22 - gap * (count - 1)) / count;

        int idx = 0;
        for (auto* k : knobs)
        {
            const int x = x0 + idx * (w + gap);
            k->slider.setBounds(x, y, w, 72);
            k->label.setBounds(x, y - 14, w, 12);
            ++idx;
        }
    };

    placeRow(148, { &threshold, &ratio, &mix, &attack, &release, &memory, &fold });
    placeRow(288, { &chaos, &detectorRes, &aliasing, &spectral, &spectralMix, &fftBands, &warp, &stereo });
    placeRow(428, { &feedback, &contain, &oscBias, &hyst, &seed, &gateThresh, &gateRelease });
}
