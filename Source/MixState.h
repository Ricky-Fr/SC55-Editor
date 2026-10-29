#pragma once

#include <algorithm>
#include <array>

namespace SC55
{
    // Etat de mute / solo des 16 parts.
    //
    // Logique pure, sans JUCE ni MIDI : elle decide quelles parts doivent etre audibles
    // et ne signale que celles dont l'etat change par rapport a ce qui a deja ete applique
    // au SC-55. Elle peut donc etre testee seule.
    class MixState
    {
    public:
        static constexpr int kNumParts = 16;

        MixState()                                    { applied.fill (true); }

        // --- Mute ---
        void setMuted (int part, bool muted)          { muteFlags[(size_t) clampPart (part)] = muted; }
        bool isMuted (int part) const                 { return muteFlags[(size_t) clampPart (part)]; }
        void setAllMuted (bool muted)                 { muteFlags.fill (muted); }
        bool areAllMuted() const                      { return std::all_of (muteFlags.begin(), muteFlags.end(), [] (bool b) { return b; }); }

        // --- Solo ---
        bool isSoloed (int part) const                { return soloFlags[(size_t) clampPart (part)]; }
        bool anySolo() const                          { return std::any_of (soloFlags.begin(), soloFlags.end(), [] (bool b) { return b; }); }

        // additive = true  : la part est ajoutee aux solos, ou retiree si elle y etait
        // additive = false : solo exclusif de la part ; un second appui sur la seule part
        //                    en solo annule le solo
        void toggleSolo (int part, bool additive)
        {
            part = clampPart (part);

            if (additive)
            {
                soloFlags[(size_t) part] = ! soloFlags[(size_t) part];
            }
            else
            {
                const auto soloCount = std::count (soloFlags.begin(), soloFlags.end(), true);
                const bool wasOnlySolo = soloFlags[(size_t) part] && soloCount == 1;

                soloFlags.fill (false);
                soloFlags[(size_t) part] = ! wasOnlySolo;
            }
        }

        // --- Audibilite ---
        // Etat voulu : ni mute, et soit aucun solo, soit la part est en solo
        bool shouldBeAudible (int part) const
        {
            part = clampPart (part);
            return ! muteFlags[(size_t) part] && (! anySolo() || soloFlags[(size_t) part]);
        }

        // Etat actuellement applique au SC-55
        bool isAudible (int part) const               { return applied[(size_t) clampPart (part)]; }

        // Compare l'etat voulu a l'etat applique et appelle onChange (part, audible) pour chaque
        // part qui differe (ou pour les 16 si force), puis memorise le nouvel etat applique.
        template <typename Callback>
        void apply (bool force, Callback&& onChange)
        {
            for (int part = 0; part < kNumParts; ++part)
            {
                const bool audible = shouldBeAudible (part);

                if (force || audible != applied[(size_t) part])
                {
                    applied[(size_t) part] = audible;
                    onChange (part, audible);
                }
            }
        }

        // Indique que le SC-55 est dans un etat connu (ex. toutes les parts audibles apres un GS Reset)
        void markAllApplied (bool audible)            { applied.fill (audible); }

    private:
        static int clampPart (int part)               { return std::clamp (part, 0, kNumParts - 1); }

        std::array<bool, kNumParts> muteFlags {};
        std::array<bool, kNumParts> soloFlags {};
        std::array<bool, kNumParts> applied {};
    };
}
