#pragma once

#include <memory>
#include <juce_core/juce_core.h>

// Gestion des fichiers de presets.
//
//  - un fichier par preset : <AppData>/SC55Editor/Presets/<nom>.sc55preset (XML)
//  - le nom du dernier preset utilise : <AppData>/SC55Editor/settings.xml
//  - le preset "Default" n'a pas de fichier : il est construit par le code
//
// Les ecritures se font dans le dossier de donnees de l'utilisateur (jamais dans
// le dossier d'installation) et de facon atomique (fichier temporaire puis
// remplacement), pour rester sures si plusieurs instances tournent en meme temps.
class PresetManager
{
public:
    static constexpr const char* kDefaultName    = "Default";
    static constexpr const char* kFileExtension  = ".sc55preset";

    explicit PresetManager (const juce::File& rootFolder = getDefaultRootFolder());

    static juce::File getDefaultRootFolder();     // <AppData>/SC55Editor

    bool isDefault (const juce::String& name) const;
    bool exists (const juce::String& name) const;

    // "Default" en tete, puis les presets utilisateur par ordre alphabetique
    juce::StringArray getPresetNames() const;

    // nullptr si le fichier est introuvable ou invalide
    std::unique_ptr<juce::XmlElement> load (const juce::String& name) const;

    bool save (const juce::String& name, const juce::XmlElement& preset, juce::String& error) const;
    bool remove (const juce::String& name) const;

    juce::String getLastPresetName() const;
    void setLastPresetName (const juce::String& name) const;

private:
    juce::File presetsFolder() const   { return root.getChildFile ("Presets"); }
    juce::File settingsFile() const    { return root.getChildFile ("settings.xml"); }
    juce::File fileFor (const juce::String& name) const;

    juce::File root;
};
