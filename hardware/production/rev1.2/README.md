# Gamers rev 1.2 manufacturing exports

This folder contains the original Fabrication Toolkit exports from November 6, 2026.

| File | Purpose |
| --- | --- |
| `Gamers_rev1.2.zip` | Gerber and drill files |
| `Gamers_rev1.2_bom.csv` | Top-side assembly bill of materials |
| `Gamers_rev1.2_positions.csv` | Top-side component placement file |

## JLCPCB order review

The exported CSVs predate the JLCPCB placement review. Compare them with the final order data before using them for another assembly run.

- C7 must have its positive terminal on PCB pad 1, toward the LM386 output. The negative stripe faces the speaker side.
- U2 (LM66100) and U3 (TPS61023) must have package pin 1 aligned with the white triangle at PCB pad 1.
- The exported BOM lists C17407 for R6 and R7. C237121 was proposed as a 100 kΩ, 1%, 0805 substitute. Check the final order to confirm which part was selected.
