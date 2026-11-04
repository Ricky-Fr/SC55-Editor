# Roland SC-55 Editor - Standalone and VST

![Status](https://img.shields.io/badge/status-in%20development-orange?style=flat-square)
![Version](https://img.shields.io/badge/version-1.8%20stable-brightgreen?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-blue?style=flat-square)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![JUCE](https://img.shields.io/badge/Framework-JUCE-black?style=flat-square)
![Format](https://img.shields.io/badge/format-Standalone%20%7C%20VST3-purple?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)

SC-55 Editor is a modern Windows application for managing and editing Roland SC-55 Sound Canvas devices through MIDI and SysEx communication.

An editor tailored for the Roland SC-55 **hardware** unit. It allows you to tweak and control hardware parameters directly from your PC.
Roland SoundCanvas SC-55 is a desktop synthesizer, also called sound expander.

A dedicated VST3 plugin version (designed specifically for Ableton Live in my workflow) is yet released.
Allows saving all settings as a preset and recalling them on the fly.
The vst version allows to do Key Switching: you put a note from C-2 to G8 in a clip then the correponding preset number is loaded. In ex : C#-2 = preset #1, D-2 = preset #2

Using Vibe Coding with Claude and Copilot. Done in C++ and JUCE.

The current version is fully functional and supports tweaking all main parameters.

Works also with Nuked-SC-55-GUI Float emulator that has a midi input, and succesfully tested with The Usual Suspect's one.

<img width="639" height="273" alt="sc55-hw" src="https://github.com/user-attachments/assets/a5a7b4c8-c366-4997-afbd-13a24d43b1e6" />

---

## 🚀 Upcoming Features
- [ ] **more Effects parameters**

---

<img width="722" height="632" alt="image" src="https://github.com/user-attachments/assets/a85b35a6-13a1-4c4d-ae3c-955cc833065a" />

*Main window*

<img width="722" height="632" alt="image" src="https://github.com/user-attachments/assets/df0bc743-97d4-44f9-b71b-4f3ec0603176" />

*View window*

<img width="722" height="632" alt="image" src="https://github.com/user-attachments/assets/9e361be0-a5b5-4a18-8b6e-83d11846fe42" />

*Advanced window*

<img width="722" height="632" alt="image" src="https://github.com/user-attachments/assets/227a5d1b-ddd0-4cff-95f9-53b0d1991c28" />

*System window*

---

## 🛠️ Testing Setup & Environment

To test the editor without relying on physical hardware, I used a software emulator alongside virtual MIDI tools:

### 1. Emulators
- **88EmuPlayer GUI** : Downloaded from Usuals Suspects https://theusualsuspects.io/downloads/88emuplayer
- **Nuked-SC55 GUI**: Downloaded from [linoshkmalayil/Nuked-SC55-GUI-Float](https://github.com/linoshkmalayil/Nuked-SC55-GUI-Float) (conveniently provides a pre-compiled `.exe`).
- **ROM Requirements**: You will need to source the required ROM files independently. Although my physical unit runs ROM v1.20, I used **ROM v1.21**, which is properly recognized by the emulator executable.

### 2. MIDI Routing & Monitoring
- **loopMIDI**: Used to create a virtual MIDI port for interconnectivity.
- **MIDI-OX**: Used to monitor and inspect MIDI messages passing through for verification.

> ⚠️ **Important Note**: The emulator uses `loopMIDI` as its input device. Make sure to disable all other MIDI devices in your setup before launching the emulator to avoid conflicts. When playing keyboard in Ableton, please don't select 'track' for the virtual port in input.

---

## 📄 License

This project is licensed under the **MIT License** - see the LICENSE file for details.


## Keywords

Roland SC-55, Roland Sound Canvas, SC-55 MkII, Sound Canvas,
MIDI Editor, SysEx Editor, GS MIDI, General MIDI,
Hardware Synthesizer, MIDI Device Manager,
Windows Application, JUCE Framework,
Retro Gaming, DOS Gaming, MIDI Programming
