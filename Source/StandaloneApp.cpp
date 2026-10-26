// ============================================================================
//  Application autonome personnalisee de SC55 Editor.
//
//  Adaptation de juce_audio_plugin_client_Standalone.cpp (JUCE) : elle fait la meme
//  chose que l'application autonome standard, avec en plus l'entree "About" dans le
//  menu Options. Elle n'est compilee que pour la cible Standalone (voir CMakeLists.txt,
//  JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1).
// ============================================================================

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

// Fonction de creation du plugin, definie dans PluginProcessor.cpp
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace
{
    constexpr int kAboutMenuItemId = 100;   // les identifiants 1 a 4 sont ceux du menu JUCE

    // Fenetre "About" : deux lignes de texte, un clic n'importe ou la ferme.
    // L'auteur et la version viennent du CMakeLists.txt (COMPANY_NAME et VERSION).
    class AboutBox : public juce::Component
    {
    public:
        explicit AboutBox (std::function<void()> onCloseRequested)
            : closeRequested (std::move (onCloseRequested))
        {
            setSize (320, 120);
            setOpaque (true);
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (juce::Colour (0xff23272a));
            g.setColour (juce::Colour (0xff5a6068));
            g.drawRect (getLocalBounds(), 1);

            auto area = getLocalBounds().reduced (12);

            g.setColour (juce::Colour (0xffe8e8e8));
            g.setFont (juce::Font (juce::FontOptions (22.0f, juce::Font::bold)));
            g.drawText ("Author " + juce::String (juce::CharPointer_UTF8 (JucePlugin_Manufacturer)),
                        area.removeFromTop (area.getHeight() / 2), juce::Justification::centred);

            g.setColour (juce::Colour (0xffff9a2e));
            g.setFont (juce::Font (juce::FontOptions (20.0f)));
            g.drawText ("Version " + juce::String (juce::CharPointer_UTF8 (JucePlugin_VersionString)),
                        area, juce::Justification::centred);
        }

        void mouseDown (const juce::MouseEvent&) override
        {
            // La fenetre est supprimee par son proprietaire, apres la fin de cet evenement
            if (closeRequested)
            {
                auto callback = closeRequested;
                juce::MessageManager::callAsync (std::move (callback));
            }
        }

    private:
        std::function<void()> closeRequested;
    };

    //==========================================================================
    // Fenetre standard JUCE, avec "About" ajoute au menu Options
    class SC55StandaloneWindow : public juce::StandaloneFilterWindow
    {
    public:
        using juce::StandaloneFilterWindow::StandaloneFilterWindow;

        ~SC55StandaloneWindow() override
        {
            aboutBox.reset();
        }

    private:
        // Meme menu que celui de JUCE, plus l'entree "About"
        void buttonClicked (juce::Button*) override
        {
            juce::PopupMenu menu;
            menu.addItem (1, TRANS ("Audio/MIDI Settings..."));
            menu.addSeparator();
            menu.addItem (2, TRANS ("Save current state..."));
            menu.addItem (3, TRANS ("Load a saved state..."));
            menu.addSeparator();
            menu.addItem (4, TRANS ("Reset to default state"));
            menu.addSeparator();
            menu.addItem (kAboutMenuItemId, "About");

            juce::Component::SafePointer<SC55StandaloneWindow> safeThis (this);

            menu.showMenuAsync (juce::PopupMenu::Options(), [safeThis] (int result)
            {
                if (safeThis == nullptr || result == 0)
                    return;

                if (result == kAboutMenuItemId)
                    safeThis->showAboutBox();
                else
                    safeThis->handleMenuResult (result);
            });
        }

        void showAboutBox()
        {
            juce::Component::SafePointer<SC55StandaloneWindow> safeThis (this);

            aboutBox = std::make_unique<AboutBox> ([safeThis]
            {
                if (safeThis != nullptr)
                    safeThis->aboutBox.reset();
            });

            aboutBox->setCentrePosition (getScreenBounds().getCentre());
            aboutBox->addToDesktop (juce::ComponentPeer::windowHasDropShadow);
            aboutBox->setAlwaysOnTop (true);
            aboutBox->setVisible (true);
            aboutBox->toFront (true);
        }

        std::unique_ptr<AboutBox> aboutBox;
    };
}

namespace juce
{

//==============================================================================
class SC55StandaloneApp final : public JUCEApplication
{
public:
    SC55StandaloneApp()
    {
        PropertiesFile::Options options;

        options.applicationName     = CharPointer_UTF8 (JucePlugin_Name);
        options.filenameSuffix      = ".settings";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX || JUCE_BSD
        options.folderName          = "~/.config";
       #else
        options.folderName          = "";
       #endif

        appProperties.setStorageParameters (options);
    }

    const String getApplicationName() override              { return CharPointer_UTF8 (JucePlugin_Name); }
    const String getApplicationVersion() override           { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override              { return true; }
    void anotherInstanceStarted (const String&) override    {}

    SC55StandaloneWindow* createWindow()
    {
        if (Desktop::getInstance().getDisplays().displays.isEmpty())
        {
            // Aucun ecran disponible : pas de fenetre
            jassertfalse;
            return nullptr;
        }

        return new SC55StandaloneWindow (getApplicationName(),
                                         LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                         createPluginHolder());
    }

    std::unique_ptr<StandalonePluginHolder> createPluginHolder()
    {
        const Array<StandalonePluginHolder::PluginInOuts> channelConfig;

        return std::make_unique<StandalonePluginHolder> (appProperties.getUserSettings(),
                                                         false,
                                                         String{},
                                                         nullptr,
                                                         channelConfig,
                                                         false);
    }

    //==============================================================================
    void initialise (const String&) override
    {
        mainWindow = rawToUniquePtr (createWindow());

        if (mainWindow != nullptr)
            mainWindow->setVisible (true);
        else
            pluginHolder = createPluginHolder();
    }

    void shutdown() override
    {
        pluginHolder = nullptr;
        mainWindow = nullptr;
        appProperties.saveIfNeeded();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        if (pluginHolder != nullptr)
            pluginHolder->savePluginState();

        if (mainWindow != nullptr)
            mainWindow->pluginHolder->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay (100, []()
            {
                if (auto app = JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

private:
    ApplicationProperties appProperties;
    std::unique_ptr<SC55StandaloneWindow> mainWindow;
    std::unique_ptr<StandalonePluginHolder> pluginHolder;
};

} // namespace juce

// Point d'entree : l'application personnalisee remplace celle de JUCE
JUCE_CREATE_APPLICATION_DEFINE (juce::SC55StandaloneApp)
