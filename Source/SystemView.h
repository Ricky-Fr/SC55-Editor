#pragma once

#include "ParamPanel.h"
#include "PluginProcessor.h"

// Ecran SYSTEM : reglages generaux du SC-55 (pas ceux d'une part).
//
//   zone 0 (haut gauche)  : System       pan general, transposition generale, accord general
//   zone 1 (haut droite)  : Reverb
//   zone 2 (bas gauche)   : Chorus
//   zone 3 (bas droite)   : Transmission (reglages de facade : aucun SysEx dans la documentation)
//
// Il se superpose a l'ecran de reglage d'une part, comme VIEW et ADVANCED.
class SystemView : public ParamPanel
{
public:
    explicit SystemView (SC55Processor& processorToUse);
    ~SystemView() override;

protected:
    Range getRange (ParamRef ref) const override;
    int   getDefaultRaw (ParamRef ref) const override;
    int   getRaw (ParamRef ref) const override;
    void  setRaw (ParamRef ref, int rawValue, bool immediate) override;

private:
    // Origine d'un parametre : commande globale de la table principale (PartParamId)
    // ou parametre systeme / effet (SysParamId)
    enum Source { GlobalSource = 0, SysSource = 1 };

    SC55Processor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemView)
};
