#pragma once

#include <juce_core/juce_core.h>

// Genere a partir de parameters.json (manuel du SC-55, pages 66-68 et 70-71).
// pc : numero d'instrument 1..128 ; cc0 : variation (0 = instrument de base).

namespace SC55
{
    struct InstrumentEntry { int pc; int cc0; const char* name; };
    struct DrumSetEntry    { int pc; const char* name; };

    inline constexpr const char* kInstrumentFamilies[16] =
    {
        "Piano",
        "Chromatic Percussion",
        "Organ",
        "Guitar",
        "Bass",
        "Strings / Orchestra",
        "Ensemble",
        "Brass",
        "Lead",
        "Pipe",
        "Synth Lead",
        "Synth Pad etc.",
        "Synth SFX",
        "Ethnic",
        "Percussive",
        "SFX",
    };

    // Tries par pc puis par cc0 : l'instrument de base (cc0 = 0) precede ses variations
    inline constexpr InstrumentEntry kInstruments[] =
    {
        { 1, 0, "Piano 1" },
        { 2, 0, "Piano 2" },
        { 3, 0, "Piano 3" },
        { 4, 0, "Honky-tonk" },
        { 5, 0, "E. Piano 1" },
        { 5, 8, "Detuned EP 1" },
        { 6, 0, "E. Piano 2" },
        { 6, 8, "Detuned EP 2" },
        { 7, 0, "Harpsichord" },
        { 7, 8, "Coupled Hps." },
        { 8, 0, "Clav." },
        { 9, 0, "Celesta" },
        { 10, 0, "Glockenspiel" },
        { 11, 0, "Music Box" },
        { 12, 0, "Vibraphone" },
        { 13, 0, "Marimba" },
        { 14, 0, "Xylophone" },
        { 15, 0, "Tubular-bell" },
        { 15, 8, "Church Bell" },
        { 16, 0, "Santur" },
        { 17, 0, "Organ 1" },
        { 17, 8, "Detuned Or. 1" },
        { 18, 0, "Organ 2" },
        { 18, 8, "Detuned Or. 2" },
        { 19, 0, "Organ 3" },
        { 20, 0, "Church Org. 1" },
        { 20, 8, "Church Org. 2" },
        { 21, 0, "Reed Organ" },
        { 22, 0, "Accordion Fr" },
        { 22, 8, "Accordion It" },
        { 23, 0, "Harmonica" },
        { 24, 0, "Bandneon" },
        { 25, 0, "Nylon-str. Gt" },
        { 25, 8, "Ukulele" },
        { 26, 0, "Steel-Str. Gt" },
        { 26, 8, "12-str. Gt" },
        { 26, 16, "Mandolin" },
        { 27, 0, "Jazz Gt." },
        { 27, 8, "Hawaiian Gt." },
        { 28, 0, "Clean Gt." },
        { 28, 8, "Chorus Gt." },
        { 29, 0, "Muted Gt." },
        { 29, 8, "Funk Gt." },
        { 30, 0, "Overdrive Gt" },
        { 31, 0, "DistortionGt" },
        { 31, 8, "Feedback Gt." },
        { 32, 0, "Gt. Harmonics" },
        { 32, 8, "Gt. Feedback" },
        { 33, 0, "Acoustic Bs." },
        { 34, 0, "Fingered Bs." },
        { 35, 0, "Picked Bs." },
        { 36, 0, "Fretless Bs." },
        { 37, 0, "Slap Bs. 1" },
        { 38, 0, "Slap Bs. 2" },
        { 39, 0, "Synth Bass 1" },
        { 39, 8, "Synth Bass 3" },
        { 40, 0, "Synth Bass 2" },
        { 40, 8, "Synth Bass 4" },
        { 41, 0, "Violin" },
        { 42, 0, "Viola" },
        { 43, 0, "Cello" },
        { 44, 0, "Contrabass" },
        { 45, 0, "Tremolo Str" },
        { 46, 0, "PizzicatoStr" },
        { 47, 0, "Harp" },
        { 48, 0, "Timpani" },
        { 49, 0, "Strings" },
        { 49, 8, "Orchestra" },
        { 50, 0, "Slow Strings" },
        { 51, 0, "Syn. Strings1" },
        { 51, 8, "Syn. Strings3" },
        { 52, 0, "Syn. Strings2" },
        { 53, 0, "Choir Aahs" },
        { 54, 0, "Voice Oohs" },
        { 55, 0, "SynVox" },
        { 56, 0, "OrchestraHit" },
        { 57, 0, "Trumpet" },
        { 58, 0, "Trombone" },
        { 59, 0, "Tuba" },
        { 60, 0, "MutedTrumpet" },
        { 61, 0, "French Horn" },
        { 62, 0, "Brass 1" },
        { 62, 8, "Brass 2" },
        { 63, 0, "Synth Brass1" },
        { 63, 8, "Synth Brass3" },
        { 64, 0, "Synth Brass2" },
        { 64, 8, "Synth Brass4" },
        { 65, 0, "Soprano sax" },
        { 66, 0, "Alto sax" },
        { 67, 0, "Tenor sax" },
        { 68, 0, "Baritone sax" },
        { 69, 0, "Oboe" },
        { 70, 0, "English Horn" },
        { 71, 0, "Bassoon" },
        { 72, 0, "Clarinet" },
        { 73, 0, "Piccolo" },
        { 74, 0, "Flute" },
        { 75, 0, "Recorder" },
        { 76, 0, "Pan flute" },
        { 77, 0, "Bottle Blow" },
        { 78, 0, "Shakuhachi" },
        { 79, 0, "Whistle" },
        { 80, 0, "Ocarina" },
        { 81, 0, "Square Wave" },
        { 82, 0, "Saw Wave" },
        { 83, 0, "Syn. Calliope" },
        { 84, 0, "Chiffer Lead" },
        { 85, 0, "Charang" },
        { 86, 0, "Solo Vox" },
        { 87, 0, "5th Saw Wave" },
        { 88, 0, "Bass&Lead" },
        { 89, 0, "Fantasia" },
        { 90, 0, "Warm Pad" },
        { 91, 0, "Polysynth" },
        { 92, 0, "Space Voice" },
        { 93, 0, "Bowed Glass" },
        { 94, 0, "Metal Pad" },
        { 95, 0, "Halo Pad" },
        { 96, 0, "Sweep Pad" },
        { 97, 0, "Ice Rain" },
        { 98, 0, "Soundtrack" },
        { 99, 0, "Crystal" },
        { 100, 0, "Atmosphere" },
        { 101, 0, "Brightness" },
        { 102, 0, "Goblin" },
        { 103, 0, "Echo Drops" },
        { 104, 0, "Star Theme" },
        { 105, 0, "Sitar" },
        { 106, 0, "Banjo" },
        { 107, 0, "Shamisen" },
        { 108, 0, "Koto" },
        { 108, 8, "Taisho Koto" },
        { 109, 0, "Kalimba" },
        { 110, 0, "Bag Pipe" },
        { 111, 0, "Fiddle" },
        { 112, 0, "Shanai" },
        { 113, 0, "Tinkle Bell" },
        { 114, 0, "Agogo" },
        { 115, 0, "Steel Drums" },
        { 116, 0, "Woodblock" },
        { 116, 8, "Castanets" },
        { 117, 0, "Taiko" },
        { 117, 8, "Concert BD" },
        { 118, 0, "Melo Tom 1" },
        { 118, 8, "Melo Tom 2" },
        { 119, 0, "Synth Drum" },
        { 119, 8, "808 Tom" },
        { 120, 0, "Reverse Cym." },
        { 121, 0, "Gt. FretNoise" },
        { 121, 1, "Gt. Cut Noise" },
        { 121, 2, "String Slap" },
        { 122, 0, "Fl. Keyclick" },
        { 123, 0, "Seashore" },
        { 123, 1, "Rain" },
        { 123, 2, "Thunder" },
        { 123, 3, "Wind" },
        { 123, 4, "Stream" },
        { 123, 5, "Bubble" },
        { 124, 0, "Bird" },
        { 124, 1, "Dog" },
        { 124, 2, "Horse" },
        { 125, 0, "Telephone 1" },
        { 125, 1, "Telephone 2" },
        { 125, 2, "DoorCreaking" },
        { 125, 3, "Door" },
        { 125, 4, "Scratch" },
        { 125, 5, "Windchime" },
        { 126, 0, "Helicopter" },
        { 126, 1, "Car-engine" },
        { 126, 2, "Car-stop" },
        { 126, 3, "Car-pass" },
        { 126, 4, "Car-crash" },
        { 126, 5, "Siren" },
        { 126, 6, "Train" },
        { 126, 7, "Jetplane" },
        { 126, 8, "Starship" },
        { 126, 9, "Burst Noise" },
        { 127, 0, "Applause" },
        { 127, 1, "Laughing" },
        { 127, 2, "Screaming" },
        { 127, 3, "Punch" },
        { 127, 4, "Heart Beat" },
        { 127, 5, "Footstep" },
        { 128, 0, "Gun Shot" },
        { 128, 1, "Machinegun" },
        { 128, 2, "Lasergun" },
        { 128, 3, "Explosion" },
    };
    inline constexpr int kNumInstruments = (int) (sizeof (kInstruments) / sizeof (kInstruments[0]));

    // Jeux de batterie (selectionnes par Program Change sur la part rythmique)
    inline constexpr DrumSetEntry kDrumSets[] =
    {
        { 1, "Standard Set" },
        { 9, "Room Set" },
        { 17, "Power Set" },
        { 25, "Electronic Set" },
        { 26, "TR-808 Set" },
        { 33, "Jazz Set" },
        { 41, "Brush Set" },
        { 49, "Orchestra Set" },
        { 57, "SFX Set" },
        { 128, "CM-64/32L Set" },
    };
    inline constexpr int kNumDrumSets = (int) (sizeof (kDrumSets) / sizeof (kDrumSets[0]));

    // Index dans kInstruments : correspondance exacte, sinon instrument de base, sinon -1
    inline int findInstrumentIndex (int pc, int cc0)
    {
        int capital = -1;
        for (int i = 0; i < kNumInstruments; ++i)
        {
            if (kInstruments[i].pc != pc)
                continue;
            if (kInstruments[i].cc0 == cc0)
                return i;
            if (kInstruments[i].cc0 == 0)
                capital = i;
        }
        return capital;
    }

    inline int findDrumSetIndex (int pc)
    {
        for (int i = 0; i < kNumDrumSets; ++i)
            if (kDrumSets[i].pc == pc)
                return i;
        return -1;
    }

    inline juce::String getInstrumentName (int pc, int cc0)
    {
        const int i = findInstrumentIndex (pc, cc0);
        return i >= 0 ? juce::String (kInstruments[i].name) : "Instrument " + juce::String (pc);
    }

    inline juce::String getDrumSetName (int pc)
    {
        const int i = findDrumSetIndex (pc);
        return i >= 0 ? juce::String (kDrumSets[i].name) : "Drum Set " + juce::String (pc);
    }
}
