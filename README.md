# Stepdad Ultra
**Stepdad Ultra** is an NTR-032 emulator for Nintendo 64, primarily created for testing the `H8_BIG_ENDIAN` and `H8_REVERSE_BITFIELDS` compile-time options of **[libh8300h](https://github.com/celerizer/libh8300h)**.

<img width="683" height="512" alt="image" src="https://github.com/user-attachments/assets/160f9120-ffc7-493e-bc75-48ec3ffdd740" />

## Controls

| | Nintendo 64 | NTR-032 |
|-|-|-|
| ![d](https://github.com/celerizer/Press-F-Ultra/assets/33245078/ce131dfb-45ac-42d0-8182-c9a89062795d) | Control Pad | ◁ / ▷ Control Buttons |
| ![a](https://github.com/celerizer/Press-F-Ultra/assets/33245078/f634cf7e-4705-42b0-a607-14b9e057ea8a) | A Button | ◯ Control Button |
| ![z](https://github.com/celerizer/Press-F-Ultra/assets/33245078/8ac5415c-8cfd-4d43-9dd3-0c278163eafc) | Z Trigger | Take a step (pulse the accelerometer) |

## Unsupported features

Currently, sound, IR, and saving data are all unsupported. RTC may or may not be supported depending on your flashcart.

## Building
- Set up a [libdragon environment](https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon) on the preview branch.
- Clone the project and the core emulation submodule:
```sh
git clone https://github.com/celerizer/Stepdad-Ultra.git --recurse-submodules
```
- Place the 48KB ROM and your 64KB EEPROM in the `roms` directory as `ntr032.bin` and `ntr032-eep.bin`.
- Run `make`.

## License

- **Stepdad Ultra** and **libh8300h** are distributed under the MIT license. See LICENSE for information.
