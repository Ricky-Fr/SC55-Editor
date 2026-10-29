#include "ParamPanel.h"

namespace
{
    constexpr int kRowHeight = 24;
    constexpr int kFirstRowY = 36;     // sous le titre d'une zone
    constexpr int kLabelX    = 16;
    constexpr int kLabelW    = 124;

    const juce::Colour kTitleColour (0xffe8943a);   // titres de zones
    const juce::Colour kLineColour  (0xff3c3c3c);   // filets
    const juce::Colour kLabelColour (0xffd0d0d0);
    const juce::Colour kNoteColour  (0xff8f8f8f);

    juce::String noteName (int note)
    {
        static const char* names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        note = juce::jlimit (0, 127, note);
        return juce::String (names[note % 12]) + juce::String (note / 12 - 1);   // 60 = C4, 0 = C-1, 127 = G9
    }

    // Accepte un numero ("60") ou un nom de note ("C4", "F#2", "Bb3")
    int parseNote (const juce::String& text)
    {
        const auto t = text.trim().toUpperCase();
        if (t.isEmpty() || t.containsOnly ("0123456789-+"))
            return t.getIntValue();

        static const int semitoneOfLetter[7] = { 9, 11, 0, 2, 4, 5, 7 };   // A B C D E F G
        const auto letter = t[0];
        if (letter < 'A' || letter > 'G')
            return t.getIntValue();

        int semitone = semitoneOfLetter[letter - 'A'];
        int next = 1;

        if (t[next] == '#')      { ++semitone; ++next; }
        else if (t[next] == 'B') { --semitone; ++next; }

        const int octave = t.substring (next).getIntValue();
        return (octave + 1) * 12 + semitone;
    }
}

juce::String formatParamValue (ParamFormat format, int v)
{
    switch (format)
    {
        case ParamFormat::Signed:
            return v >= 0 ? "+" + juce::String (v) : juce::String (v);

        case ParamFormat::Note:
            return noteName (v);

        case ParamFormat::FineHz:
        {
            const float hz = (float) v / 10.0f;
            return (v >= 0 ? juce::String ("+") : juce::String()) + juce::String (hz, 1) + " Hz";
        }

        case ParamFormat::TuneCents:
        {
            const float cents = (float) v / 10.0f;
            return (v >= 0 ? juce::String ("+") : juce::String()) + juce::String (cents, 1) + " ct";
        }

        case ParamFormat::Semitones:
            return juce::String (v) + " st";

        case ParamFormat::Pan:
            if (v <= 0)  return "RND";
            if (v < 64)  return "L" + juce::String (64 - v);
            if (v == 64) return "0";
            return "R" + juce::String (v - 64);

        case ParamFormat::Plain:
        default:
            return juce::String (v);
    }
}

int parseParamValue (ParamFormat format, const juce::String& text)
{
    switch (format)
    {
        case ParamFormat::Note:
            return parseNote (text);

        case ParamFormat::FineHz:
        case ParamFormat::TuneCents:
            return juce::roundToInt (text.getFloatValue() * 10.0f);

        case ParamFormat::Pan:
        {
            const auto t = text.trim().toUpperCase();
            if (t.startsWith ("RND") || t.startsWith ("RAND")) return 0;
            if (t.startsWithChar ('L')) return 64 - t.substring (1).getIntValue();
            if (t.startsWithChar ('R')) return 64 + t.substring (1).getIntValue();
            return 64 + t.getIntValue();
        }

        default:
            return text.getIntValue();
    }
}

// ---------------------------------------------------------------------------

// Une ligne de l'ecran : un nom, puis un slider ou une liste
struct ParamPanel::Row
{
    juce::Label label;
    std::unique_ptr<SysExParamSlider> slider;
    std::unique_ptr<juce::ComboBox> combo;
    ParamRef ref { 0, 0 };
    ParamFormat format = ParamFormat::Plain;
    int zone = 0;
    int gapBefore = 0;
    juce::String statusName;
};

ParamPanel::ParamPanel (const juce::StringArray& zoneTitles, int splitYPosition)
    : titles (zoneTitles), splitY (splitYPosition)
{
    setOpaque (true);

    headerLabel.setColour (juce::Label::textColourId, kTitleColour);
    headerLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (headerLabel);

    closeButton.onClick = [this]
    {
        if (onCloseRequested)
            onCloseRequested();
    };
    addAndMakeVisible (closeButton);
}

ParamPanel::~ParamPanel() = default;

void ParamPanel::setHeaderText (const juce::String& text)
{
    headerLabel.setText (text, juce::dontSendNotification);
}

// ---------------------------------------------------------------------------
// Construction des lignes
// ---------------------------------------------------------------------------

void ParamPanel::addSliderRow (int zone, const juce::String& labelText, ParamRef ref, ParamFormat format,
                               int gapBefore, const juce::String& statusName)
{
    auto row = std::make_unique<Row>();
    row->ref = ref;
    row->format = format;
    row->zone = zone;
    row->gapBefore = gapBefore;
    row->statusName = statusName.isNotEmpty() ? statusName : labelText;

    const auto range = getRange (ref);
    const size_t index = rows.size();

    SysExParamSlider::Config config;
    config.minValue        = range.rawMin - range.displayOffset;
    config.maxValue        = range.rawMax - range.displayOffset;
    config.defaultValue    = getDefaultRaw (ref) - range.displayOffset;
    config.style           = juce::Slider::LinearHorizontal;
    config.textBoxPosition = juce::Slider::TextBoxRight;
    config.textBoxWidth    = 64;
    config.textBoxHeight   = 20;
    config.formatter       = [format] (int v) { return formatParamValue (format, v); };
    config.parser          = [format] (const juce::String& t) { return parseParamValue (format, t); };

    row->slider = std::make_unique<SysExParamSlider> (config,
        [this, index] (int value, bool immediate) { onSliderChanged (index, value, immediate); });

    // Valeur a droite, sans cadre
    row->slider->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    row->slider->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    row->slider->setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffbbbbbb));

    row->label.setText (labelText, juce::dontSendNotification);
    row->label.setColour (juce::Label::textColourId, kLabelColour);

    addAndMakeVisible (*row->slider);
    addAndMakeVisible (row->label);
    rows.push_back (std::move (row));
}

void ParamPanel::addComboRow (int zone, const juce::String& labelText, ParamRef ref,
                              const juce::StringArray& items, const juce::String& statusName)
{
    auto row = std::make_unique<Row>();
    row->ref = ref;
    row->zone = zone;
    row->statusName = statusName.isNotEmpty() ? statusName : labelText;

    const size_t index = rows.size();

    row->combo = std::make_unique<juce::ComboBox>();
    for (int i = 0; i < items.size(); ++i)
        row->combo->addItem (items[i], i + 1);          // identifiant = valeur envoyee + 1
    row->combo->onChange = [this, index] { onComboChanged (index); };

    row->label.setText (labelText, juce::dontSendNotification);
    row->label.setColour (juce::Label::textColourId, kLabelColour);

    addAndMakeVisible (*row->combo);
    addAndMakeVisible (row->label);
    rows.push_back (std::move (row));
}

void ParamPanel::addNote (int zone, const juce::String& text)
{
    notes.push_back ({ zone, text });
}

// ---------------------------------------------------------------------------
// Modification et rafraichissement
// ---------------------------------------------------------------------------

void ParamPanel::onSliderChanged (size_t rowIndex, int displayValue, bool immediate)
{
    auto& row = *rows[rowIndex];
    const auto range = getRange (row.ref);

    setRaw (row.ref, displayValue + range.displayOffset, immediate);

    if (onStatus)
        onStatus (getStatusPrefix() + row.statusName + " = " + formatParamValue (row.format, displayValue));
}

void ParamPanel::onComboChanged (size_t rowIndex)
{
    auto& row = *rows[rowIndex];
    const int raw = row.combo->getSelectedId() - 1;
    if (raw < 0)
        return;

    setRaw (row.ref, raw, true);

    if (onStatus)
        onStatus (getStatusPrefix() + row.statusName + " = " + row.combo->getText());
}

void ParamPanel::refresh()
{
    for (auto& rowPtr : rows)
    {
        auto& row = *rowPtr;
        const auto range = getRange (row.ref);
        const int raw = getRaw (row.ref);

        if (row.slider != nullptr)
        {
            row.slider->setValueSilently (raw - range.displayOffset);
            row.slider->setDefaultValue (getDefaultRaw (row.ref) - range.displayOffset);
            row.slider->refreshDisplayedText();
        }

        if (row.combo != nullptr)
            row.combo->setSelectedId (raw + 1, juce::dontSendNotification);
    }

    repaint();
}

// ---------------------------------------------------------------------------
// Dessin et disposition
// ---------------------------------------------------------------------------

juce::Rectangle<int> ParamPanel::zoneBounds (int zone) const
{
    const int w = getWidth();
    const int h = getHeight();
    const int vSplit = w / 2;

    const int x = (zone % 2 == 0) ? 0 : vSplit;
    const int width = (zone % 2 == 0) ? vSplit : w - vSplit;
    const int y = (zone < 2) ? 0 : splitY;
    const int height = (zone < 2) ? splitY : h - splitY;

    return { x, y, width, height };
}

void ParamPanel::paint (juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    g.fillAll (juce::Colour (0xff1e1e1e));
    g.setColour (juce::Colour (0xff6a6a6a));
    g.drawRect (getLocalBounds(), 1);

    // Filets entre les zones
    g.setColour (kLineColour);
    g.drawHorizontalLine (splitY, 4.0f, (float) w - 4.0f);
    g.drawVerticalLine (w / 2, 4.0f, (float) h - 4.0f);

    // Titres et traits discrets sous les titres
    for (int zone = 0; zone < titles.size() && zone < 4; ++zone)
    {
        if (titles[zone].isEmpty())
            continue;

        const auto b = zoneBounds (zone);

        g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)));
        g.setColour (kTitleColour);
        g.drawText (titles[zone], b.getX() + 16, b.getY() + 8, b.getWidth() - 32, 20, juce::Justification::centredLeft);

        g.setColour (kLineColour.withAlpha (0.6f));
        g.drawHorizontalLine (b.getY() + 30, (float) b.getX() + 12.0f, (float) b.getRight() - 12.0f);
    }

    // Textes d'information
    g.setFont (juce::Font (juce::FontOptions (12.5f)));
    g.setColour (kNoteColour);
    for (const auto& note : notes)
        g.drawFittedText (note.text, zoneBounds (note.zone).reduced (16, 0).withTrimmedTop (40).withTrimmedBottom (10),
                          juce::Justification::topLeft, 8);
}

void ParamPanel::resized()
{
    int y[4];
    for (int zone = 0; zone < 4; ++zone)
        y[zone] = zoneBounds (zone).getY() + kFirstRowY;

    for (auto& rowPtr : rows)
    {
        auto& row = *rowPtr;
        const auto zone = zoneBounds (row.zone);
        y[row.zone] += row.gapBefore;

        const int controlX = zone.getX() + kLabelX + kLabelW + 4;
        const int controlW = zone.getRight() - 14 - controlX;

        row.label.setBounds (zone.getX() + kLabelX, y[row.zone], kLabelW, kRowHeight);

        if (row.slider != nullptr)
            row.slider->setBounds (controlX, y[row.zone], controlW, kRowHeight);

        if (row.combo != nullptr)
            row.combo->setBounds (controlX, y[row.zone] + 1, controlW - 4, kRowHeight - 2);

        y[row.zone] += kRowHeight;
    }

    closeButton.setBounds (getWidth() - 28, 4, 22, 22);
    headerLabel.setBounds (getWidth() - 28 - 6 - 90, 6, 90, 20);
}
