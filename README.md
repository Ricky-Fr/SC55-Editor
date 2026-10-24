# Roland SC-55 Editor

An editor tailored for the Roland SC-55 hardware unit. It allows you to tweak and control hardware parameters directly from your PC. A dedicated VST plugin version (designed specifically for Ableton Live in my workflow) is planned as a future development phase.

The current version is fully functional and supports tweaking all main parameters.

---

## 🚀 Upcoming Features
- [ ] **Preset Management** (High priority)
- [ ] **Mute Button** integration
- [ ] **VST Plugin Version** (for DAW integration)

---

## 🛠️ Testing Setup & Environment

To test the editor without relying on physical hardware, I used a software emulator alongside virtual MIDI tools:

### 1. Emulator
- **Nuked-SC55 GUI**: Downloaded from [linoshkmalayil/Nuked-SC55-GUI-Float](https://github.com/linoshkmalayil/Nuked-SC55-GUI-Float) (conveniently provides a pre-compiled `.exe`).
- **ROM Requirements**: You will need to source the required ROM files independently. Although my physical unit runs ROM v1.20, I used **ROM v1.21**, which is properly recognized by the emulator executable.

### 2. MIDI Routing & Monitoring
- **loopMIDI**: Used to create a virtual MIDI port for interconnectivity.
- **MIDI-OX**: Used to monitor and inspect MIDI messages passing through for verification.

> ⚠️ **Important Note**: The emulator uses `loopMIDI` as its input device. Make sure to disable all other MIDI devices in your setup before launching the emulator to avoid conflicts.
