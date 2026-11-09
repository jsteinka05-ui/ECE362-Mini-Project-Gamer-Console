# Cup Pong console hardware references

The reference designators below follow the Gamers rev 1.2 carrier PCB and assembly BOM. Manufacturer series datasheets are shared across parts where applicable. The LCSC or JLCPCB links identify the exact assembly variants.

## Processor, display, and hand-installed parts

| PCB reference | Part | Datasheet or product reference |
| --- | --- | --- |
| J1–J4 | Purdue Proton RP2350B board and debugger | [Proton user guide](Purdue-Proton-User-Guide.pdf), [pinout](Purdue-Proton-Pinout.pdf), [RP2350 datasheet](Raspberry-Pi-RP2350-Datasheet.pdf), [Purdue schematic and PCB files](https://ece362-purdue.github.io/proton-labs/datasheets/) |
| J1–J5 | 2.54 mm single-row female sockets | [Adafruit #598 product reference](https://www.adafruit.com/product/598). Use two 28-pin strips and one each of 3, 4, and 14 pins. |
| J5 | 3.2-inch, 240 × 320 SPI TFT, 14-pin header | [ILI9341 controller datasheet](Ilitek-ILI9341.pdf), [MSP3218-family module manual](LCDWiki-MSP3218-3.2in-User-Manual.pdf), [module schematic](LCDWiki-MSP3218-3.2in-Schematic.pdf). Confirm the module revision against the physical display before using the module-level documentation. |
| J7 | Phoenix Contact MKDS 1,5/2-5,08 (1715721) | [Terminal block datasheet and drawing](Phoenix-Contact-1715721.pdf), [manufacturer product page](https://www.phoenixcontact.com/en-us/products/printed-circuit-board-terminal-mkds-15-2-508-1715721). This is the part specified by the PCB footprint. |
| LS1 | Treedix 1 W, 8 Ω mini speaker | [Treedix product specifications for STRX5-0660](https://treedix.com/products/treedix-10pcs-1-watt-8-ohm-mini-speaker-full-range-cavity-advertising-machine-small-speaker-audio-speaker-stereo). Confirm the SKU on the purchased speaker before using these mechanical dimensions. |
| RV1 | Adafruit #4271, 10 kΩ slide potentiometer | [Mechanical drawing and specifications](Adafruit-4271-10k-Slider-Drawing.pdf), [Adafruit product page](https://www.adafruit.com/product/4271) |
| SW1, SW2 | Adafruit #1119, 12 mm tactile switches | [Omron B3F family reference](Omron-B3F-Tactile-Switches.pdf), [Adafruit product page](https://www.adafruit.com/product/1119). Adafruit identifies the B3F-40XX footprint and provides the Omron sheet as an example. |
| SW3 | SS12D00-G3 SPDT slide switch | [SOFNG product drawing](https://www.sofng.com/Slideswitch/2023/0325/1831.html) for comparison. The switch in hand is EGS CST branded. Confirm its pin pitch and mechanical fit on the PCB. |
| U1 | Texas Instruments LM386N-1, DIP-8 audio amplifier | [LM386 datasheet](TI-LM386.pdf) |
| U4 | Bare joystick and cap from Adafruit #512 | [Adafruit product sheet](Adafruit-512-Joystick-Product-Sheet.pdf), [official PCB and schematic reference](https://github.com/adafruit/2-axis-joystick-breakout-board-with-mounting-holes). The carrier accepts the bare joystick. The product-sheet dimensions describe the assembled breakout. |
| U7 | Microchip 24AA32AF, DIP-8 I²C EEPROM | [24AA32AF/24LC32AF datasheet](Microchip-24AA32AF-24LC32AF.pdf), [Purdue lab kit listing](https://ece362-purdue.github.io/proton-labs/lab-kit/) |
| External to J7 | 3×AA holder and AA cells | [Adafruit #771 example holder specifications](https://www.adafruit.com/product/771), [Energizer E91 example alkaline AA datasheet](Energizer-E91-AA-Alkaline.pdf) |

The **2.2-inch MSP2202 lab-kit display** referenced in the project overview has a separate [module drawing](LCDWiki-MSP2202-2.2in-Drawing.pdf) and [Purdue lab-kit documentation](https://ece362-purdue.github.io/proton-labs/lab-kit/#22-inch-spi-module-msp2202-tft-lcd-display-sd-card-reader).

## Top-side assembly parts

| PCB reference | Value and manufacturer part | Assembly listing | Datasheet |
| --- | --- | --- | --- |
| C1, C5 | 100 nF, Yageo CC0805KRX7R9BB104 | [C49678](https://www.lcsc.com/product-detail/C49678.html) | [Yageo CC series](Yageo-CC-Series-MLCC.pdf) |
| C2, C8 | 10 µF, Samsung CL31A106KBHNNNE | [C13585](https://www.lcsc.com/product-detail/C13585.html) | [Samsung CL series](Samsung-CL-Series-MLCC.pdf) |
| C3 | 10 nF, Samsung CL21B103KBANNNC | [C1710](https://www.lcsc.com/product-detail/C1710.html) | [Samsung CL series](Samsung-CL-Series-MLCC.pdf) |
| C4, C11 | 1 µF, Samsung CL21B105KBFNNNE | [C28323](https://www.lcsc.com/product-detail/C28323.html) | [Samsung CL series](Samsung-CL-Series-MLCC.pdf) |
| C6 | 47 nF, Samsung CL21B473KBCNNNC | [C53134](https://www.lcsc.com/product-detail/C53134.html) | [Samsung CL series](Samsung-CL-Series-MLCC.pdf) |
| C7 | 220 µF, 16 V, SamYoung MVG, 6.3 × 7.7 mm | [C2827940](https://jlcpcb.com/partdetail/2975110-MVG_16V_220UF_6_3_77/C2827940) | [SamYoung MVG series](SamYoung-MVG-Series.pdf) |
| C9, C10 | 22 µF, Samsung CL31A226KAHNNNE | [C12891](https://www.lcsc.com/product-detail/C12891.html) | [Samsung CL series](Samsung-CL-Series-MLCC.pdf) |
| L1 | 1 µH, Sunlord MWSA0402S-1R0MT | [C408332](https://jlcpcb.com/partdetail/Sunlord-MWSA0402S1R0MT/C408332) | [Sunlord MWSA series](Sunlord-MWSA-Series.pdf) |
| R1, R2 | 4.7 kΩ, Uniroyal 0805W8F4701T5E | [C17673](https://www.lcsc.com/product-detail/C17673.html) | [Uniroyal thick-film chip resistors](Uniroyal-Thick-Film-Chip-Resistors.pdf) |
| R3 | 330 kΩ, Uniroyal 0805W8F3303T5E | [C17629](https://www.lcsc.com/product-detail/C17629.html) | [Uniroyal thick-film chip resistors](Uniroyal-Thick-Film-Chip-Resistors.pdf) |
| R4 | 10 Ω, Uniroyal 1206W4F100JT5E | [C17903](https://www.lcsc.com/product-detail/C17903.html) | [Uniroyal thick-film chip resistors](Uniroyal-Thick-Film-Chip-Resistors.pdf) |
| R5 | 732 kΩ, Uniroyal 0805W8F7323T5E | [C17816](https://www.lcsc.com/product-detail/C17816.html) | [Uniroyal thick-film chip resistors](Uniroyal-Thick-Film-Chip-Resistors.pdf) |
| R6, R7 | 100 kΩ, Uniroyal 0805W8F1003T5E in the export. Stackpole RMCF0805FT100K is the suggested assembly substitute. | [C17407](https://www.lcsc.com/product-detail/C17407.html), [C237121 substitute](https://jlcpcb.com/partdetail/C237121) | [Uniroyal thick-film chip resistors](Uniroyal-Thick-Film-Chip-Resistors.pdf), [Stackpole RMCF series](Stackpole-RMCF-Series.pdf) |
| U2 | Texas Instruments LM66100DCKR ideal diode | [C2869734](https://www.lcsc.com/product-detail/C2869734.html) | [LM66100 datasheet](TI-LM66100.pdf) |
| U3 | Texas Instruments TPS61023DRLR boost converter | [C919459](https://www.lcsc.com/product-detail/C919459.html) | [TPS61023 datasheet](TI-TPS61023.pdf) |
