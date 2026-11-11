# Diagram Conventions

Architecture, protocol, wiring diagrams.

## Tools

| Domain | Tool | Source Format | Presentation |
|--------|------|---------------|--------------|
| **Software Architecture & Flows** | [Mermaid](https://mermaid.js.org/) | Markdown (`.md`) | Rendered natively on GitHub |
| **Hardware Schematics & Wiring** | [Fritzing](https://fritzing.org/) | `.fzz` (Binary XML) | Exported `.svg` committed side-by-side |

---

## Layout

```
docs/diagrams/
├── README.md                 # This guide
├── hardware/                 # Fritzing circuit diagrams (.fzz/.svg planned, not committed yet)
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

## Fritzing Export

1. Open/create circuit in Fritzing (`.fzz`).
2. **File > Export > as Image > SVG...**.
3. Save `.svg` in `docs/diagrams/hardware/`, same basename as `.fzz`.
4. Commit `.fzz` and `.svg` together:
   ```bash
   git add docs/diagrams/hardware/esp32-ld2450-wiring.fzz docs/diagrams/hardware/esp32-ld2450-wiring.svg
   git commit -s -m "[Docs] Update LD2450 wiring diagram"
   ```
