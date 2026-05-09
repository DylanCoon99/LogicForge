# Generating LogicForge Components with an LLM

Use this prompt template to have an LLM generate `.dccomp` files from IC datasheets. Save the output as `ComponentName.dccomp` in `~/Documents/digital_design_app/components/`, then click Tools > Refresh Components in LogicForge.

## Prompt Template

```
Generate a LogicForge .dccomp component file for the [CHIP PART NUMBER] based on this datasheet information:

[PASTE RELEVANT DATASHEET SECTIONS: pinout, truth table, function table, logic diagram]

The .dccomp format is a JSON file with this structure:

{
  "name": "CHIP NAME",
  "inputs": ["PIN_NAME_1", "PIN_NAME_2", ...],
  "outputs": ["PIN_NAME_1", "PIN_NAME_2", ...],
  "circuit": {
    "version": "1.0",
    "components": [ ...internal components... ],
    "wires": [ ...connections between internal components... ]
  }
}

The internal circuit must be built from these available component types:

COMBINATIONAL:
- {"type": "AND", "numInputs": N}       - AND gate with N inputs
- {"type": "OR", "numInputs": N}        - OR gate with N inputs
- {"type": "NOT"}                        - Inverter (1 input)
- {"type": "NAND", "numInputs": N}      - NAND gate
- {"type": "NOR", "numInputs": N}       - NOR gate
- {"type": "XOR", "numInputs": N}       - XOR gate
- {"type": "XNOR", "numInputs": N}      - XNOR gate

SEQUENTIAL:
- {"type": "D Flip-Flop"}               - Pins: D (input 0), CLK (input 1), Q (output 0), Q' (output 1)
- {"type": "SR Flip-Flop"}              - Pins: S (input 0), R (input 1), CLK (input 2), Q (output 0), Q' (output 1)
- {"type": "JK Flip-Flop"}              - Pins: J (input 0), K (input 1), CLK (input 2), Q (output 0), Q' (output 1)
- {"type": "T Flip-Flop"}               - Pins: T (input 0), CLK (input 1), Q (output 0), Q' (output 1)

ROUTING:
- {"type": "Mux 2:1"}                   - Pins: D0 (input 0), D1 (input 1), S0 (input 2), Y (output 0)
- {"type": "Mux 4:1"}                   - Pins: D0-D3 (inputs 0-3), S0-S1 (inputs 4-5), Y (output 0)

I/O (used to define the external interface):
- {"type": "Input Switch", "label": "PIN_NAME"}   - Becomes an external input pin
- {"type": "Output Probe", "label": "PIN_NAME"}   - Becomes an external output pin

Each component needs:
- "type": one of the types above
- "id": a unique UUID string (use any valid UUID format)
- "x" and "y": position coordinates (layout doesn't matter, use multiples of 20)
- "label": (required for Input Switch and Output Probe to map to external pins)

Wires connect components using:
- "srcId": UUID of the source component
- "srcPin": output pin index (0-based) on the source
- "dstId": UUID of the destination component
- "dstPin": input pin index (0-based) on the destination

IMPORTANT RULES:
- Every external input must be an Input Switch with a label matching the "inputs" array
- Every external output must be an Output Probe with a label matching the "outputs" array
- Pin indices are 0-based
- For gates: inputs are indexed 0..N-1, output is index 0
- For flip-flops: see pin order listed above
- All flip-flops are rising-edge triggered on their CLK input
- UUIDs MUST be valid format: 8-4-4-4-12 hex digits (e.g., "a1000000-0000-0000-0000-000000000001")
- UUIDs must use ONLY hexadecimal characters (0-9, a-f). Letters like g-z are NOT valid hex!
- Use unique UUIDs for every component

Return ONLY the JSON, no explanation.
```

## Example: 74LS08 (Quad 2-Input AND Gate)

```json
{
  "name": "74LS08",
  "inputs": ["1A", "1B", "2A", "2B", "3A", "3B", "4A", "4B"],
  "outputs": ["1Y", "2Y", "3Y", "4Y"],
  "circuit": {
    "version": "1.0",
    "components": [
      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000001", "x": 0, "y": 0, "label": "1A", "rotation": 0},
      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000002", "x": 0, "y": 40, "label": "1B", "rotation": 0},
      {"type": "AND", "id": "a0000001-0000-0000-0000-000000000003", "x": 100, "y": 20, "numInputs": 2, "rotation": 0},
      {"type": "Output Probe", "id": "a0000001-0000-0000-0000-000000000004", "x": 200, "y": 20, "label": "1Y", "rotation": 0},

      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000005", "x": 0, "y": 100, "label": "2A", "rotation": 0},
      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000006", "x": 0, "y": 140, "label": "2B", "rotation": 0},
      {"type": "AND", "id": "a0000001-0000-0000-0000-000000000007", "x": 100, "y": 120, "numInputs": 2, "rotation": 0},
      {"type": "Output Probe", "id": "a0000001-0000-0000-0000-000000000008", "x": 200, "y": 120, "label": "2Y", "rotation": 0},

      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000009", "x": 0, "y": 200, "label": "3A", "rotation": 0},
      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000010", "x": 0, "y": 240, "label": "3B", "rotation": 0},
      {"type": "AND", "id": "a0000001-0000-0000-0000-000000000011", "x": 100, "y": 220, "numInputs": 2, "rotation": 0},
      {"type": "Output Probe", "id": "a0000001-0000-0000-0000-000000000012", "x": 200, "y": 220, "label": "3Y", "rotation": 0},

      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000013", "x": 0, "y": 300, "label": "4A", "rotation": 0},
      {"type": "Input Switch", "id": "a0000001-0000-0000-0000-000000000014", "x": 0, "y": 340, "label": "4B", "rotation": 0},
      {"type": "AND", "id": "a0000001-0000-0000-0000-000000000015", "x": 100, "y": 320, "numInputs": 2, "rotation": 0},
      {"type": "Output Probe", "id": "a0000001-0000-0000-0000-000000000016", "x": 200, "y": 320, "label": "4Y", "rotation": 0}
    ],
    "wires": [
      {"srcId": "a0000001-0000-0000-0000-000000000001", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000003", "dstPin": 0},
      {"srcId": "a0000001-0000-0000-0000-000000000002", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000003", "dstPin": 1},
      {"srcId": "a0000001-0000-0000-0000-000000000003", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000004", "dstPin": 0},

      {"srcId": "a0000001-0000-0000-0000-000000000005", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000007", "dstPin": 0},
      {"srcId": "a0000001-0000-0000-0000-000000000006", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000007", "dstPin": 1},
      {"srcId": "a0000001-0000-0000-0000-000000000007", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000008", "dstPin": 0},

      {"srcId": "a0000001-0000-0000-0000-000000000009", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000011", "dstPin": 0},
      {"srcId": "a0000001-0000-0000-0000-000000000010", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000011", "dstPin": 1},
      {"srcId": "a0000001-0000-0000-0000-000000000011", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000012", "dstPin": 0},

      {"srcId": "a0000001-0000-0000-0000-000000000013", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000015", "dstPin": 0},
      {"srcId": "a0000001-0000-0000-0000-000000000014", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000015", "dstPin": 1},
      {"srcId": "a0000001-0000-0000-0000-000000000015", "srcPin": 0, "dstId": "a0000001-0000-0000-0000-000000000016", "dstPin": 0}
    ]
  }
}
```

## Tips

- For simple combinational ICs (74LS00, 74LS04, 74LS08, etc.), the LLM will get it right almost every time
- For sequential ICs (74LS161, 74LS374, etc.), provide the full function table from the datasheet
- Always test the generated component by wiring it up and verifying against the datasheet's truth/function table
- If the LLM gets the pin mapping wrong, the most common issue is mismatched pin indices - check that flip-flop CLK is at the correct input index
