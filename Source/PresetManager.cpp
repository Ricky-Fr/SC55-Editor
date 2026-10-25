#include "PresetManager.h"

PresetManager::PresetManager (const juce::File& rootFolder)
    : root (rootFolder)
{
}

juce::File PresetManager::getDefaultRootFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("SC55Editor");
}

bool PresetManager::isDefault (const juce::String& name) const
{
    return name.trim().equalsIgnoreCase (kDefaultName);
}

juce::File PresetManager::fileFor (const juce::String& name) const
{
    return presetsFolder().getChildFile (juce::File::createLegalFileName (name.trim()) + kFileExtension);
}

bool PresetManager::exists (const juce::String& name) const
{
    return isDefault (name) || fileFor (name).existsAsFile();
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;

    const auto files = presetsFolder().findChildFiles (juce::File::findFiles, false,
                                                       juce::String ("*") + kFileExtension);
    for (const auto& file : files)
    {
        if (auto xml = juce::XmlDocument::parse (file))
            if (xml->hasTagName ("SC55Preset"))
                names.add (xml->getStringAttribute ("name", file.getFileNameWithoutExtension()));
    }

    names.removeString (kDefaultName, true);
    names.sortNatural();
    names.insert (0, kDefaultName);
    return names;
}

std::unique_ptr<juce::XmlElement> PresetManager::load (const juce::String& name) const
{
    const auto file = fileFor (name);
    if (! file.existsAsFile())
        return nullptr;

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("SC55Preset"))
        return nullptr;

    return xml;
}

bool PresetManager::save (const juce::String& name, const juce::XmlElement& preset, juce::String& error) const
{
    if (! presetsFolder().createDirectory().wasOk())
    {
        error = "Impossible de creer le dossier des presets";
        return false;
    }

    const auto target = fileFor (name);
    juce::TemporaryFile temp (target);

    if (! preset.writeTo (temp.getFile()))
    {
        error = "Ecriture du fichier impossible";
        return false;
    }

    if (! temp.overwriteTargetFileWithTemporary())
    {
        error = "Remplacement du fichier impossible";
        return false;
    }

    return true;
}

bool PresetManager::remove (const juce::String& name) const
{
    return ! isDefault (name) && fileFor (name).deleteFile();
}

juce::String PresetManager::getLastPresetName() const
{
    if (auto xml = juce::XmlDocument::parse (settingsFile()))
        if (xml->hasTagName ("SC55Settings"))
            return xml->getStringAttribute ("lastPreset");

    return {};
}

void PresetManager::setLastPresetName (const juce::String& name) const
{
    if (! root.createDirectory().wasOk())
        return;

    juce::XmlElement xml ("SC55Settings");
    xml.setAttribute ("lastPreset", name);

    const auto target = settingsFile();
    juce::TemporaryFile temp (target);

    if (xml.writeTo (temp.getFile()))
        temp.overwriteTargetFileWithTemporary();
}
