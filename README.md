# AVR Temperature-Controlled Fan

ATmega328P firmware that samples an analogue temperature signal, displays a converted reading on a multiplexed three-digit display, and switches a fan output using separate on/off thresholds.

**Stack:** Embedded C · AVR registers and interrupts · ATmega328P · Atmel/Microchip Studio project

## How it works

The main loop reads ADC5, converts the reading using the original integer scaling, updates the display value, and applies the fan thresholds. Timer0's overflow interrupt refreshes one display digit at a time. Separating display refresh from the main loop keeps the multiplexing logic independent of the temperature-control decisions.

| Converted reading | Fan output |
| --- | --- |
| `>= 260` | Set PD7 HIGH |
| `<= 250` | Clear PD7 LOW |
| Between 250 and 260 | Keep the previous output state |

This gap implements hysteresis, reducing repeated switching around a single threshold. The thresholds are **raw converted integer values** in the source; verify the sensor's transfer function and decimal-point convention before interpreting them as degrees Celsius.

## Hardware mapping from the code

| Pins/peripheral | Use |
| --- | --- |
| PC5 / ADC5 | Analogue temperature input |
| PC0–PC3 | Four-bit BCD display output |
| PB0–PB2 | Multiplexed digit selection |
| PD7 | Fan-control output |
| Timer0 | Display-refresh overflow interrupt |
| ADC reference | Internal 1.1 V setting in `ADMUX` |

The source comments describe a 5 ms timer overflow using preload `0xB2` and prescaler 1024. The actual interval depends on the MCU clock. Match the clock, display circuitry, analogue input, and external fan driver to the original hardware before programming a board.

## Reproduce the firmware build

**Prerequisites:** Git, a compatible Atmel/Microchip Studio installation with AVR GCC and ATmega328P device support. Physical execution additionally needs an ATmega328P board, programmer, matching clock/fuses, analogue sensor circuit, BCD/display driver and digit multiplexing hardware, and a suitable external fan driver.

```sh
git clone https://github.com/ChuaJiaSheng/avr-temperature-controlled-fan.git
cd avr-temperature-controlled-fan
```

1. Open [Finial Project.atsln](Finial%20Project.atsln) in Studio.
2. Verify the selected MCU is **ATmega328P** and the AVR GCC toolchain is selected. Keep the solution/project paths unchanged.
3. Select Debug or Release and choose **Build → Build Solution**. **Expected build result:** an ELF is produced in that configuration's output directory under `Finial Project/`.
4. For a board, verify the pin mapping above, clock/fuses, reference voltage, sensor scaling, and fan driver against the actual circuitry before flashing. Use Studio's device-programming function with the correct programmer and generated firmware.
5. Observe display refresh and compare the converted reading with a known input. The intended control check is PD7 HIGH at `>=260`, LOW at `<=250`, and unchanged between those thresholds.

**Reproduction limits:** the repository provides the firmware project, but not a complete physical-board specification, calibrated sensor transfer function, or confirmed fuse/programmer configuration. The source comments' 5 ms refresh interval depends on the clock. Building the firmware and reproducing the complete physical system are separate tasks; do not treat the numbers 260/250 as calibrated Celsius values without confirming the conversion.

## Source guide

Main source: [222257U_Project_1-02-2024.c](Finial%20Project/222257U_Project_1-02-2024.c).

| Function | Responsibility |
| --- | --- |
| `ioInit` | Configure digital ports |
| `initADC`, `readADC` | Configure and sample ADC5 |
| `convert` | Scale the ADC reading using integer arithmetic |
| `displayNow` | Output one display digit and advance the multiplexer |
| `interruptInit`, `TIMER0_OVF_vect` | Configure and service display refresh |
| `main` | Initialise peripherals and apply hysteresis |

## Author

Chua Jia Sheng.
