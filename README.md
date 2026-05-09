# LogicForge

A desktop application for designing and simulating digital logic circuits. Built for engineers, students, and hobbyists who want to prototype and verify digital designs before breadboarding.

LogicForge focuses on **logic correctness** - it simulates gate-level behavior, signal propagation, and sequential logic without modeling electrical characteristics.

## Features

### Component Library

**Logic Gates:** AND, OR, NOT, NAND, NOR, XOR, XNOR - rendered as classical IEEE schematic symbols with configurable input count (2-64 inputs)

**Sequential Logic:**
- Flip-Flops (D, SR, JK, T) - edge-triggered with clock triangle indicators
- Latches (D, SR) - level-triggered with enable pin
- Registers - parallel load with hex value display, configurable bit width (1-32)
- Counters - binary up-counter with synchronous reset, configurable bit width (1-16)

**Routing:**
- Multiplexers (2:1 up to 64:1) and Demultiplexers (1:2 up to 1:64) - power-of-2 sizes with selector pins on top edge
- Bus Splitters and Joiners - configurable bit width (2-32)

**I/O:**
- Input Switches - toggle with double-click
- Output Probes - LED-style state indicator
- Clock Source - configurable frequency, single-step mode, start/stop via right-click menu

**Custom Components:** Encapsulate any sub-circuit as a reusable block with named inputs and outputs. Supports both combinational and sequential logic internally.

**IC Import:** Generate `.dccomp` component files from IC datasheets using an LLM. See `docs/llm_component_prompt.md` for the prompt template. Pre-built components included for 74LS04, 74LS74, 74LS138, 74LS161, and 74LS173.

### Simulation

- Iterative settling engine for combinational logic
- Edge detection for sequential components (rising/falling clock edges)
- Three-phase simulation for custom components (settle combinational, fire edges, settle again)
- Oscillation detection with warnings
- Real-time signal propagation with color-coded wires (green = high, gray = low, red = undefined)
- Clock source with adjustable period and manual single-step

### Schematic Symbols

All components render as classical schematic symbols:
- Gates use standard IEEE shapes (AND D-shape, OR curved, NOT triangle, with inversion bubbles)
- Flip-flops and latches show clock triangles and pin labels
- Multiplexers render as trapezoids with selector pins on the top edge
- Custom components have a distinctive teal border

### Tools

- **Truth Table Generator** - automatically enumerates all input combinations and displays outputs
- **Timing Diagram** - dockable panel that records and displays signal waveforms over time
- **Image Export** - PNG (raster at 2x resolution) and SVG (vector) export of circuit diagrams

### Usability

- **Select Mode** - pointer button above the canvas to switch from placement to selection mode
- Save/load circuits as `.ddc` JSON files (defaults to `~/Documents/digital_design_app/circuits/`)
- Copy, paste, and cut components with internal wiring preserved
- Undo/redo support
- Component labels and annotations (right-click > Edit Label)
- Component rotation (90-degree increments)
- Component resizing - drag the corner handle to resize any component
- Configurable component sizes - right-click to set gate input count, mux/demux size, register/counter bit width, or bus width
- Wire bend points - double-click a wire to add a bend point, drag to reposition, double-click a bend point to remove it
- Right-click context menus on all components with type-specific options
- Pan (right-click drag, spacebar + drag, middle-click drag, Alt + drag)
- Zoom (scroll wheel)
- Snap-to-grid placement
- Organized sidebar palette with section headers (Logic Gates, I/O, Flip-Flops, Latches, Mux/Demux, Registers/Counters, Bus, Custom ICs)

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
| Escape | Cancel wiring or placement (return to select mode) |
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
./run.sh
```

### Linux

```bash
git clone https://github.com/DylanCoon99/LogicForge.git
cd LogicForge
qmake digital_design_app.pro
make -j$(nproc)
./run.sh
```

### Windows

Open `digital_design_app.pro` in Qt Creator and build, or from a Qt command prompt:

```cmd
qmake digital_design_app.pro
nmake
digital_design_app.exe
```

## Usage

1. **Place components** - Click a component in the sidebar palette, then click on the canvas to place it. Click the pointer icon (top-left of canvas) or press Escape to return to select mode.
2. **Wire components** - Click an output pin (green), then click an input pin (blue) to connect them. Double-click a wire to add bend points for custom routing.
3. **Simulate** - Toggle input switches (double-click) to see signals propagate through the circuit.
4. **Clock circuits** - Double-click a Clock Source to start/stop it, or right-click for single-step and frequency controls.
5. **Configure components** - Right-click a gate to set input count, a mux to set size, a register to set bit width, etc.
6. **Resize components** - Select a component, then drag the blue triangle at the bottom-right corner.
7. **Create custom components** - Select a group of components (must include InputSwitches and/or OutputProbes), right-click, and choose "Create Custom Component..." to save it as a reusable block.
8. **Import ICs** - Generate `.dccomp` files from datasheets using an LLM (see `docs/llm_component_prompt.md`), save to `~/Documents/digital_design_app/components/`, and click Tools > Refresh Components.
9. **Save your work** - Ctrl+S saves to a `.ddc` file (JSON format).

## File Formats

- **`.ddc`** - Circuit project files (JSON). Self-contained, including any embedded custom components.
- **`.dccomp`** - Custom component definitions (JSON). Stored in `~/Documents/digital_design_app/components/` and loaded into the palette automatically.

## Project Structure

```
src/
  main.cpp                  Entry point
  mainwindow.h/cpp          Main window, menus, toolbar, palette
  canvas/
    circuitscene.h/cpp      Circuit canvas (placement, wiring, simulation triggers)
    circuitview.h/cpp       Viewport (pan, zoom)
  model/
    component.h/cpp         Base component class with serialization
    pin.h/cpp               Pin with state and edge detection
    wire.h/cpp              Wire connections
    circuit.h/cpp           Circuit container with JSON serialization
    simulationengine.h/cpp  Iterative settling and single-edge simulation
  components/
    gatecomponent.h/cpp     AND, OR, NOT, NAND, NOR, XOR, XNOR (configurable inputs)
    flipflop.h/cpp          D, SR, JK, T flip-flops
    latch.h/cpp             D, SR latches
    inputswitch.h/cpp       Toggleable input
    outputprobe.h/cpp       LED output indicator
    clocksource.h/cpp       Clock oscillator with step/frequency control
    mux.h/cpp               Multiplexer (configurable size)
    demux.h/cpp             Demultiplexer (configurable size)
    registercomponent.h/cpp Register (configurable bit width)
    counter.h/cpp           Counter (configurable bit width)
    bussplitter.h/cpp       Bus splitter (configurable width)
    busjoiner.h/cpp         Bus joiner (configurable width)
    customcomponent.h/cpp   User-defined reusable components
  graphics/
    componentgraphicsitem.h/cpp  Schematic symbol rendering with resize handles
    pingraphicsitem.h/cpp        Pin visualization and interaction
    wiregraphicsitem.h/cpp       Wire routing with draggable bend points
  widgets/
    truthtabledialog.h/cpp       Truth table generator
    timingdiagramwidget.h/cpp    Waveform viewer
  dialogs/
    createcustomdialog.h/cpp     Custom component creation dialog
docs/
  llm_component_prompt.md       Prompt template for generating IC components
components/
  *.dccomp                      Pre-built IC component definitions
```

## License

This project is open source. See [LICENSE](LICENSE) for details.
