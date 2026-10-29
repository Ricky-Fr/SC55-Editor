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

    // ========================================================================
    //  Parametres de l'ecran ADVANCED (hors Key Shift et Bend Range, deja dans la table
    //  principale). Tous sont des parametres de part : adresse 40 1n <offset>.
    // ========================================================================
    enum AdvParamId
    {
        // Tone Modify (valeur affichee = valeur envoyee - 64 : -50..+50)
        AdvVibratoRate = 0,
        AdvVibratoDepth,
        AdvVibratoDelay,
        AdvCutoff,
        AdvResonance,
        AdvAttack,
        AdvDecay,
        AdvRelease,

        // Part Setup
        AdvPartMode,        // 0 = Normal, 1 = Drum Map 1, 2 = Drum Map 2
        AdvVelocityDepth,
        AdvVelocityOffset,
        AdvKeyRangeLow,
        AdvKeyRangeHigh,
        AdvRxRpn,
        AdvRxNrpn,

        // Pitch
        AdvFineTune,        // 2 octets "nibblized" : valeur 8..248, 128 = 0 Hz, pas de 0,1 Hz
        AdvMonoPoly,        // 0 = Mono, 1 = Poly

        // Ajoute a la fin pour ne pas decaler les identifiants deja enregistres dans les presets
        AdvModulationDepth, // profondeur de la molette de modulation (40 2n 04 : MOD LFO1 PITCH DEPTH)

        NumAdvParams
    };

    constexpr int kDefaultRhythmRule = -3;   // par defaut : Drum Map 1 pour la part 10, Normal pour les autres

    struct AdvParamInfo
    {
        const char* label;
        juce::uint8 offset;       // 3e octet de l'adresse
        juce::uint8 partBase;     // 0x10 ou 0x20 (2e octet de l'adresse = partBase | bloc de la part)
        int bytes;                // 1, ou 2 (donnee "nibblized" : 4 bits par octet)
        int rawMin;
        int rawMax;
        int rawDefault;
        int displayOffset;        // valeur affichee = valeur envoyee - displayOffset
    };

    inline const AdvParamInfo& getAdvParamInfo (int id)
    {
        static const AdvParamInfo table[NumAdvParams] =
        {
            // label              offset base bytes rawMin rawMax rawDef              dispOff
            { "Vibrato Rate",     0x30, 0x10,  1,    14,    114,   64,                 64  },
            { "Vibrato Depth",    0x31, 0x10,  1,    14,    114,   64,                 64  },
            { "Vibrato Delay",    0x37, 0x10,  1,    14,    114,   64,                 64  },
            { "Cutoff",           0x32, 0x10,  1,    14,    80,    64,                 64  },   // facade : -50..+16
            { "Resonance",        0x33, 0x10,  1,    14,    114,   64,                 64  },
            { "Attack",           0x34, 0x10,  1,    14,    114,   64,                 64  },
            { "Decay",            0x35, 0x10,  1,    14,    114,   64,                 64  },
            { "Release",          0x36, 0x10,  1,    14,    114,   64,                 64  },
            { "Part Mode",        0x15, 0x10,  1,    0,     2,     kDefaultRhythmRule, 0   },
            { "Velocity Depth",   0x1A, 0x10,  1,    0,     127,   64,                 0   },
            { "Velocity Offset",  0x1B, 0x10,  1,    0,     127,   64,                 0   },
            { "Key Range Low",    0x1D, 0x10,  1,    0,     127,   0,                  0   },
            { "Key Range High",   0x1E, 0x10,  1,    0,     127,   127,                0   },
            { "Rx RPN",           0x09, 0x10,  1,    0,     1,     1,                  0   },
            { "Rx NRPN",          0x0A, 0x10,  1,    0,     1,     1,                  0   },
            { "Fine Tune",        0x17, 0x10,  2,    8,     248,   128,                128 },   // +-12,0 Hz
            { "Mono / Poly",      0x13, 0x10,  1,    0,     1,     1,                  0   },
            { "Modulation Depth", 0x04, 0x20, 1, 0,     127,   10,                 0   },   // facade : 0-10-127
        };
        return table[juce::jlimit (0, (int) NumAdvParams - 1, id)];
    }

    inline int getAdvDefaultRaw (const AdvParamInfo& info, int part)
    {
        if (info.rawDefault == kDefaultRhythmRule)
            return part == 9 ? 1 : 0;

        return info.rawDefault;
    }

    // ========================================================================
    //  Parametres de l'ecran SYSTEM : accord general et effets (reverb, chorus).
    //  Ce ne sont pas des parametres de part : adresses absolues sur 3 octets.
    //
    //  Le niveau de reverb (40 01 33), le niveau de chorus (40 01 3A), le pan general
    //  (40 00 06) et la transposition generale (40 00 05) sont deja geres comme
    //  commandes globales de la table principale (PartReverb, PartChorus, PartPan,
    //  PartKeyShift) : l'ecran SYSTEM les partage avec l'ecran de part.
    // ========================================================================
    enum SysParamId
    {
        SysMasterTune = 0,        // 40 00 00, 4 octets "nibblized" : 24..2024, 1024 = 0 cent, pas de 0,1 cent
        SysReverbType,            // 40 01 30 : macro 0..7
        SysReverbCharacter,       // 40 01 31
        SysReverbTime,            // 40 01 34
        SysReverbDelayFeedback,   // 40 01 35
        SysReverbSendToChorus,    // 40 01 36
        SysChorusType,            // 40 01 38 : macro 0..7
        SysChorusPreLpf,          // 40 01 39
        SysChorusFeedback,        // 40 01 3B
        SysChorusDelay,           // 40 01 3C
        SysChorusRate,            // 40 01 3D
        SysChorusDepth,           // 40 01 3E
        SysChorusSendToReverb,    // 40 01 3F
        NumSysParams
    };

    struct SysParamInfo
    {
        const char* label;
        juce::uint8 a1, a2, a3;   // adresse
        int bytes;                // 1, ou 4 (donnee "nibblized")
        int rawMin;
        int rawMax;
        int rawDefault;
        int displayOffset;        // valeur affichee = valeur envoyee - displayOffset
    };

    inline const SysParamInfo& getSysParamInfo (int id)
    {
        static const SysParamInfo table[NumSysParams] =
        {
            // label             a1    a2    a3    bytes rawMin rawMax rawDef  dispOff
            { "Tune",            0x40, 0x00, 0x00, 4,    0x018, 0x7E8, 0x400,  0x400 },   // -100,0..+100,0 cent
            { "Type",            0x40, 0x01, 0x30, 1,    0,     7,     4,      0     },   // Hall 2
            { "Character",       0x40, 0x01, 0x31, 1,    0,     7,     4,      0     },
            { "Time",            0x40, 0x01, 0x34, 1,    0,     127,   0x40,   0     },
            { "Delay Feedback",  0x40, 0x01, 0x35, 1,    0,     127,   0,      0     },
            { "Send to chorus",  0x40, 0x01, 0x36, 1,    0,     127,   0,      0     },
            { "Type",            0x40, 0x01, 0x38, 1,    0,     7,     2,      0     },   // Chorus 3
            { "Pre-LPF",         0x40, 0x01, 0x39, 1,    0,     7,     0,      0     },
            { "Feedback",        0x40, 0x01, 0x3B, 1,    0,     127,   0x08,   0     },
            { "Delay",           0x40, 0x01, 0x3C, 1,    0,     127,   0x50,   0     },
            { "Rate",            0x40, 0x01, 0x3D, 1,    0,     127,   0x03,   0     },
            { "Depth",           0x40, 0x01, 0x3E, 1,    0,     127,   0x13,   0     },
            { "Send to reverb",  0x40, 0x01, 0x3F, 1,    0,     127,   0,      0     },
        };
        return table[juce::jlimit (0, (int) NumSysParams - 1, id)];
    }

    // ========================================================================
    //  Reglages envoyes en Control Change (la documentation du SC-55 n'a pas d'adresse SysEx
    //  pour eux) : ils sont emis sur le canal MIDI de reception de la part.
    //  Attention : un Control Change vaut pour toutes les parts qui partagent ce canal.
    // ========================================================================
    enum CcParamId
    {
        CcPortamentoSwitch = 0,   // CC#65 : 0..63 = Off, 64..127 = On (l'editeur envoie 0 ou 127)
        CcPortamentoTime,         // CC#5  : 0..127
        NumCcParams
    };

    struct CcParamInfo
    {
        const char* label;
        int controller;           // numero de Control Change
        int rawMin;
        int rawMax;
        int rawDefault;
    };

    inline const CcParamInfo& getCcParamInfo (int id)
    {
        static const CcParamInfo table[NumCcParams] =
        {
            // label               CC  rawMin rawMax rawDef
            { "Portamento",        65, 0,     1,     0 },
            { "Portamento Time",   5,  0,     127,   0 },
        };
        return table[juce::jlimit (0, (int) NumCcParams - 1, id)];
    }

    // Valeur du Control Change envoyee pour une valeur de l'editeur (l'interrupteur envoie 0 ou 127)
    inline int getCcValue (int id, int raw)
    {
        return id == CcPortamentoSwitch ? (raw != 0 ? 127 : 0) : juce::jlimit (0, 127, raw);
    }
}
