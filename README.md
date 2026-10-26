# Roland SC-55 Editor - Standalone and VST

An editor tailored for the Roland SC-55 **hardware** unit. It allows you to tweak and control hardware parameters directly from your PC. 
A dedicated VST plugin version (designed specifically for Ableton Live in my workflow) is planned as a future development phase.
Allows saving all settings as a preset and recalling them on the fly.
Using Vibe Coding with Claude.

The current version is fully functional and supports tweaking all main parameters.

Works also with Nuked-SC-55-GUI Float emulator that has a midi input, but not with The Usual Suspect's one which has not.

<img width="639" height="273" alt="sc55-hw" src="https://github.com/user-attachments/assets/a5a7b4c8-c366-4997-afbd-13a24d43b1e6" />

---

## 🚀 Upcoming Features
- [ ] **effects parameters**
- [ ] **VST Plugin Version** (for DAW integration)

---

<img width="722" height="628" alt="sc55-1" src="https://github.com/user-attachments/assets/a770e78a-2293-462e-b3f7-d5a9064b2f61" />

Main windows

<img width="722" height="628" alt="sc55-2" src="https://github.com/user-attachments/assets/53df4b6f-9797-45de-a124-15c1cc1c27da" />

View windows

## 🛠️ Testing Setup & Environment

To test the editor without relying on physical hardware, I used a software emulator alongside virtual MIDI tools:

### 1. Emulator
- **Nuked-SC55 GUI**: Downloaded from [linoshkmalayil/Nuked-SC55-GUI-Float](https://github.com/linoshkmalayil/Nuked-SC55-GUI-Float) (conveniently provides a pre-compiled `.exe`).
- **ROM Requirements**: You will need to source the required ROM files independently. Although my physical unit runs ROM v1.20, I used **ROM v1.21**, which is properly recognized by the emulator executable.

### 2. MIDI Routing & Monitoring
- **loopMIDI**: Used to create a virtual MIDI port for interconnectivity.
- **MIDI-OX**: Used to monitor and inspect MIDI messages passing through for verification.

> ⚠️ **Important Note**: The emulator uses `loopMIDI` as its input device. Make sure to disable all other MIDI devices in your setup before launching the emulator to avoid conflicts.
