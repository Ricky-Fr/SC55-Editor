#pragma once

#include "ParamPanel.h"
#include "PluginProcessor.h"

// Ecran ADVANCED : reglages avances de la part courante.
//
//   zone 0 (haut gauche)  : Tone Modify
//   zone 1 (haut droite)  : Part Setup
//   zone 2 (bas gauche)   : libre (reservee)
//   zone 3 (bas droite)   : Pitch
//
// Il se superpose a l'ecran de reglage d'une part, comme l'ecran VIEW.
class AdvancedView : public ParamPanel
{
public:
    explicit AdvancedView (SC55Processor& processorToUse);
    ~AdvancedView() override;

    // Choisit la part affichee (0..15) et relit son etat
    void setPart (int newPart);

protected:
    Range getRange (ParamRef ref) const override;
    int   getDefaultRaw (ParamRef ref) const override;
    int   getRaw (ParamRef ref) const override;
    void  setRaw (ParamRef ref, int rawValue, bool immediate) override;
    juce::String getStatusPrefix() const override;

private:
    // Origine d'un parametre : table principale (PartParamId), table des parametres avances,
    // ou reglage envoye en Control Change (portamento)
    enum Source { MainSource = 0, AdvSource = 1, CcSource = 2 };

    SC55Processor& processor;
    int part = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedView)
};
