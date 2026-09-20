## TelexIO

**TelexIO** is a port of the open-source **TelexI** and **TelexO** Teletype expander modules to the **Music Thing Modular Workshop Computer**.
It emulates both modules together on a single workshop computer and responds to the same TI and TO commands from an i2c-connected Teletype as the original modules do. 

The Workshop Computer’s knobs, inputs, and outputs are mapped to corresponding TelexI and TelexO functionality, with a few differences due to different hardware.

This Workshop Computer card was developed as a proof of concept to investigate whether **i2c communication** through the Workshop Computer’s UART pins is possible. See electrical notes below. 

# Input and output mapping
Mapping between Workshop Computer and TelexI/TelexO functionality:
* The three knobs (main, x, y) correspond to the first three TelexI knob inputs.
* The switch is mapped to the fourth TelexI input, generating values 0,1,2.
* Audio and CV inputs are mapped to the corresponding TelexI inputs.
* Audio, CV and trigger outputs are mapped to the corresponding TelexO outputs. The Workshop Computer has two trigger outputs instead of TelexO’s four, so two are unavailable.
* There are two additional trigger inputs. These are internally normalised to trigger envelopes configured on the first two output channels of TelexIO.

# Electrical notes
TelexIO needs an i2c connection. The Workshop Computer’s UART pins are used for i2c. Solder a header to the UART pads (typically unpopulated):
* Bottom -> **SCL**
* Top -> **SDA**
Make sure Workshop Computer and Teletype share **GND**. A dedicated **GND** wire between the Workshop Computer and the Teletype i2c bus is recommended. Without it, I sometimes experienced data corruption during i2c communication.

By default the computercard.h framework appears to configure the UART pins as outputs during startup. Therefore, when using other cards based on computercard.h, disconnect the i2c connection to avoid the UART outputs fighting with the i2c-bus, or recompile those cards with the appropriate UART configuration flag - see my CMake file.

# Notes on implementation
On the RP2040, Core 1 handles the i2c interrupts and the TelexI code, while Core 0 runs most of the TelexO code with a sample rate of 48 kHz.

As the RP2040 has no FPU, some floating-point code from TelexO was rewritten to use integer arithmetic (particularly the polyblep code) and some float calculations are passed from Core 0 to Core 1 for less time critical processing.

# LLM disclosure
AI was used for code review, safety checks and optimisation suggestions. No unsupervised coding or unreviewed code was used in this project.

# License
**TelexIO** is **MIT licensed**, in line with the code it is derived from.

# History
V0.1 first release
Future work:
- Calibration is disabled and needs to be adapted to the workshop computer framework. 
