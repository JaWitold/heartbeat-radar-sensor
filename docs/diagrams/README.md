# Diagram Conventions & Repository Guide

This folder contains all architectural, protocol, and hardware wiring diagrams for the Heartbeat Radar Sensor project.

## Diagram Strategy

| Domain | Tool | Source Format | Presentation |
|--------|------|---------------|--------------|
| **Software Architecture & Flows** | [Mermaid](https://mermaid.js.org/) | Markdown (`.md`) | Rendered natively on GitHub |
| **Hardware Schematics & Wiring** | [Fritzing](https://fritzing.org/) | `.fzz` (Binary XML) | Exported `.svg` committed side-by-side |

---

## Directory Organization

```
docs/diagrams/
├── README.md                 # This guide
├── hardware/                 # Fritzing circuit diagrams
│   ├── README.md
│   ├── esp32-ld2450-wiring.fzz
│   ├── esp32-ld2450-wiring.svg
│   ├── esp32-st7789-wiring.fzz
│   ├── esp32-st7789-wiring.svg
│   ├── full-assembly.fzz
│   └── full-assembly.svg
└── software/                 # Mermaid diagrams rendered in Markdown
    ├── architecture-overview.md
    ├── class-hierarchy.md
    ├── radar-engine-states.md
    └── sensor-protocol.md
```

---

## Fritzing Export Workflow

1. Open or create the circuit in Fritzing (`.fzz`).
2. Go to **File > Export > as Image > SVG...**.
3. Save the exported `.svg` in `docs/diagrams/hardware/` using the exact same basename as the `.fzz` file.
4. Commit both the `.fzz` source and the `.svg` export in the same commit:
   ```bash
   git add docs/diagrams/hardware/esp32-ld2450-wiring.fzz docs/diagrams/hardware/esp32-ld2450-wiring.svg
   git commit -s -m "[Docs] Update LD2450 wiring diagram"
   ```
