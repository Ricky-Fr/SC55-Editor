#pragma once

#include <juce_core/juce_core.h>

// ============================================================================
//  Table des parametres exposes par des sliders (hors INSTRUMENT, traite a part).
//  Ajouter une ligne ici (et dans l'enum) suffit pour obtenir un slider de plus :
//  adresse SysEx, plage, valeur par defaut, commande globale et format d'affichage
//  viennent de cette table.
//
//  Adresse d'un parametre de part : 40 <partBase|n> <offset>   (n = bloc de la part)
//    partBase 0x10 -> 40 1n xx (parametres de part)
//    partBase 0x20 -> 40 2n xx (destinations des controleurs)
// ============================================================================
namespace SC55
{
    enum PartParamId
    {
        PartLevel = 0,
        PartPan,
        PartReverb,
        PartChorus,
        PartKeyShift,
        PartMidiChannel,
        PartBendRange,
        NumPartParams
    };

    constexpr int kDefaultIsPartNumber = -1;   // par defaut : canal MIDI = numero de la part

    // Que fait la commande "globale" (bouton ALL + bouton GLOBAL allumes) ?
    enum GlobalKind
    {
        GlobalNone,          // pas de commande globale
        GlobalMasterSysEx,   // SysEx "master" a l'adresse g1 g2 g3 : n'altere pas les parts
        GlobalAllParts,      // pas de SysEx master : la valeur est envoyee aux 16 parts
        GlobalDeviceId       // reglage local de l'editeur (Device ID), aucun message envoye
    };

    struct PartParamInfo
    {
        const char* label;        // nom affiche au-dessus du slider
        juce::uint8 offset;       // 3e octet de l'adresse (parametre de part)
        juce::uint8 partBase;     // 0x10 ou 0x20 (2e octet = partBase | bloc de la part)
        int rawMin;               // plage de la valeur envoyee au SC-55 (part)
        int rawMax;
        int rawDefault;           // valeur par defaut (part)
        int displayOffset;        // valeur affichee = valeur envoyee - displayOffset

        GlobalKind globalKind;
        juce::uint8 g1, g2, g3;   // adresse du SysEx master (GlobalMasterSysEx)
        int gRawMin;              // plage et defaut de la commande globale
        int gRawMax;
        int gRawDefault;
    };

    inline const PartParamInfo& getPartParamInfo (int id)
    {
        static const PartParamInfo table[NumPartParams] =
        {
            // label        offset base  rawMin rawMax rawDef                dispOff  globalKind          g1    g2    g3    gMin gMax gDef
            { "LEVEL",      0x19,  0x10, 0,     127,   100,                  0,       GlobalMasterSysEx,  0x40, 0x00, 0x04, 0,   127, 127 },  // Master Volume
            { "PAN",        0x1C,  0x10, 0,     127,   64,                   0,       GlobalMasterSysEx,  0x40, 0x00, 0x06, 1,   127, 64  },  // Master Pan
            { "REVERB",     0x22,  0x10, 0,     127,   40,                   0,       GlobalMasterSysEx,  0x40, 0x01, 0x33, 0,   127, 64  },  // Reverb Level (patch commun)
            { "CHORUS",     0x21,  0x10, 0,     127,   0,                    0,       GlobalMasterSysEx,  0x40, 0x01, 0x3A, 0,   127, 64  },  // Chorus Level (patch commun)
            { "KEY SHIFT",  0x16,  0x10, 0x28,  0x58,  64,                   64,      GlobalMasterSysEx,  0x40, 0x00, 0x05, 0x28, 0x58, 64 }, // Master Key-Shift
            { "MIDI CH",    0x02,  0x10, 0,     16,    kDefaultIsPartNumber, -1,      GlobalDeviceId,     0,    0,    0,    0,   31,  16  },  // Device ID 1..32
            { "BEND RANGE", 0x10,  0x20, 0x40,  0x58,  0x42,                 64,      GlobalAllParts,     0,    0,    0,    0x40, 0x58, 0x42 }, // 0..24 semitones
        };
        return table[juce::jlimit (0, (int) NumPartParams - 1, id)];
    }

    inline int getDefaultRaw (const PartParamInfo& info, int part)
    {
        return info.rawDefault == kDefaultIsPartNumber ? part : info.rawDefault;
    }

    // Texte affiche dans la zone de valeur du slider (valeur affichee, pas la valeur brute)
    inline juce::String formatPartParam (int id, int v, bool global = false)
    {
        switch (id)
        {
            case PartPan:
                if (v <= 0)  return "RND";
                if (v < 64)  return "L" + juce::String (64 - v);
                if (v == 64) return "0";
                return "R" + juce::String (v - 64);

            case PartKeyShift:
                return v > 0 ? "+" + juce::String (v) : juce::String (v);

            case PartMidiChannel:
                if (! global && v >= 17)
                    return "OFF";
                return juce::String (v);

            default:
                return juce::String (v);
        }
    }

    // Lecture d'une valeur saisie au clavier dans la zone de valeur
    inline int parsePartParam (int id, const juce::String& text)
    {
        const auto t = text.trim().toUpperCase();

        switch (id)
        {
            case PartPan:
                if (t.startsWith ("RND") || t.startsWith ("RAND")) return 0;
                if (t.startsWithChar ('L')) return 64 - t.substring (1).getIntValue();
                if (t.startsWithChar ('R')) return 64 + t.substring (1).getIntValue();
                return 64 + t.getIntValue();

            case PartMidiChannel:
                return t.startsWith ("OFF") ? 17 : t.getIntValue();

            default:
                return t.getIntValue();
        }
    }
}
