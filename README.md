# LogicForge

A desktop application for designing and simulating digital logic circuits. Built for engineers, students, and hobbyists who want to prototype and verify digital designs before breadboarding.

LogicForge focuses on **logic correctness** — it simulates gate-level behavior, signal propagation, and sequential logic without modeling electrical characteristics.

## Features

### Component Library

**Logic Gates:** AND, OR, NOT, NAND, NOR, XOR, XNOR — rendered as classical IEEE schematic symbols

**Sequential Logic:**
- Flip-Flops (D, SR, JK, T) — edge-triggered with clock triangle indicators
- Latches (D, SR) — level-triggered with enable pin
- Registers (8-bit) — parallel load with hex value display
- Counters (4-bit) — binary up-counter with synchronous reset

**Routing:**
- Multiplexers (2:1, 4:1) and Demultiplexers (1:2, 1:4)
- Bus Splitters and Joiners (8-bit)

**I/O:**
- Input Switches — toggle with double-click
- Output Probes — LED-style state indicator
- Clock Source — configurable frequency, single-step mode

**Custom Components:** Encapsulate any sub-circuit as a reusable block with named inputs and outputs

### Simulation

- Iterative settling engine for combinational logic
- Edge detection for sequential components (rising/falling clock edges)
- Oscillation detection with warnings
- Real-time signal propagation with color-coded wires (green = high, gray = low, red = undefined)
- Clock source with adjustable period and manual single-step

### Tools

- **Truth Table Generator** — automatically enumerates all input combinations and displays outputs
- **Timing Diagram** — records and displays signal waveforms over time
- **Image Export** — PNG (raster) and SVG (vector) export of circuit diagrams

### Usability

- Save/load circuits as `.ddc` JSON files
- Copy, paste, and cut components with internal wiring preserved
- Undo/redo support
- Component labels and annotations
- Component rotation (90-degree increments)
- Right-click context menus on all components
- Pan (right-click drag, spacebar + drag, middle-click drag, Alt + drag)
- Zoom (scroll wheel)
- Snap-to-grid placement

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New circuit |
| Ctrl+O | Open circuit |
| Ctrl+S | Save |
| Ctrl+Shift+S | Save As |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+C | Copy |
| Ctrl+V | Paste |
| Ctrl+X | Cut |
| Ctrl+A | Select All |
| R | Rotate selected 90 degrees |
| Delete / Backspace | Delete selected |
| Escape | Cancel wiring or placement |
| Space + Drag | Pan canvas |

## Building from Source

### Requirements

- **Qt 5.15+** (Widgets, Svg, Core, GUI modules)
- **C++17** compiler (Clang, GCC, or MSVC)
- **qmake** (included with Qt)

### macOS

```bash
git clone https://github.com/DylanCoon99/LogicForge.git
cd LogicForge
qmake digital_design_app.pro
make -j$(sysctl -n hw.ncpu)
open digital_design_app.app
```

### Linux

```bash
git clone https://github.com/DylanCoon99/LogicForge.git
cd LogicForge
qmake digital_design_app.pro
make -j$(nproc)
./digital_design_app
```

### Windows

Open `digital_design_app.pro` in Qt Creator and build, or from a Qt command prompt:

```cmd
qmake digital_design_app.pro
nmake
digital_design_app.exe
```

## Usage

1. **Place components** — Click a component in the sidebar palette, then click on the canvas to place it
2. **Wire components** — Click an output pin (green), then click an input pin (blue) to connect them
3. **Simulate** — Toggle input switches (double-click) to see signals propagate through the circuit
4. **Clock circuits** — Double-click a Clock Source to start/stop it, or right-click for single-step and frequency controls
5. **Create custom components** — Select a group of components, right-click, and choose "Create Custom Component..." to save it as a reusable block
6. **Save your work** — Ctrl+S saves to a `.ddc` file (JSON format)

## File Formats

- **`.ddc`** — Circuit project files (JSON). Self-contained, including any embedded custom components.
- **`.dccomp`** — Custom component definitions (JSON). Stored in `~/Documents/digital_design_app/components/` and loaded into the palette automatically.

## Project Structure

```
src/
  main.cpp                  Entry point
  mainwindow.h/cpp          Main window, menus, palette
  canvas/
    circuitscene.h/cpp      Circuit canvas (placement, wiring, simulation triggers)
    circuitview.h/cpp       Viewport (pan, zoom)
  model/
    component.h/cpp         Base component class with serialization
    pin.h/cpp               Pin with state and edge detection
    wire.h/cpp              Wire connections
    circuit.h/cpp           Circuit container with JSON serialization
    simulationengine.h/cpp  Iterative settling simulation
  components/
    gatecomponent.h/cpp     AND, OR, NOT, NAND, NOR, XOR, XNOR
    flipflop.h/cpp          D, SR, JK, T flip-flops
    latch.h/cpp             D, SR latches
    inputswitch.h/cpp       Toggleable input
    outputprobe.h/cpp       LED output indicator
    clocksource.h/cpp       Clock oscillator
    mux.h/cpp               Multiplexer
    demux.h/cpp             Demultiplexer
    registercomponent.h/cpp 8-bit register
    counter.h/cpp           4-bit counter
    bussplitter.h/cpp       Bus splitter
    busjoiner.h/cpp         Bus joiner
    customcomponent.h/cpp   User-defined reusable components
  graphics/
    componentgraphicsitem.h/cpp  Schematic symbol rendering
    pingraphicsitem.h/cpp        Pin visualization and interaction
    wiregraphicsitem.h/cpp       Wire routing and state coloring
  widgets/
    truthtabledialog.h/cpp       Truth table generator
    timingdiagramwidget.h/cpp    Waveform viewer
  dialogs/
    createcustomdialog.h/cpp     Custom component creation dialog
```

## License

This project is open source. See [LICENSE](LICENSE) for details.
