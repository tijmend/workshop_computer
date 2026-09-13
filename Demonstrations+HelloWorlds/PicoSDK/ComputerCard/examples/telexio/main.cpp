#include "ComputerCard.h"

#include "i2c_multi.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdio.h>


// TELEXi port strategy

// 1. AnalogReader
//    - new hardware-facing AnalogReader
//    - map address -> ComputerCard KnobVal / CVIn / AudioIn
//    - reproduce TELEXi's value range, calibration and mapping
//    - no ResponsiveAnalogRead unless testing shows it is needed
//
// 2. Test / visualize
//    - printf raw and processed values
//    - sweep knobs/CV
//    - verify ranges, polarity, calibration and mapping
//
// 3. TELEXi processing
//    - add quantization
//    - add scale/top/bottom handling
//    - run this on core 1 at ~1 kHz
//
// 4. I2C
//    - add RP2040 I²C slave
//    - connect existing TELEXi command/request protocol
//    - keep callbacks lightweight
//	  - potentially add ring buffer handler (address, length, command ..)


/*
I would therefore revise the evenings to:
1TELEXi AnalogReader + input verification
2TELEXi processing + quantization
3Both TELEXi and TELEXo I²C
4TELEXo CVOutput + ComputerCard outputs ()
5Oscillator, integration, testing, optimization

Core 0 — hard real-time
48 kHz ProcessSample()
    └── TELEXo
        ├── CVOutput::Update() * 4 [attach timer loop to process_sampe]
        └── ComputerCard outputs

Core 1 — communications / control
    ├── TELEXi
    │   ├── input scanning
    │   ├── quantization
    │   └── state
    │
    └── TELEXo I²C
        ├── receive commands
        ├── TxHelper::Parse()
        └── actOnCommand()

*/


/// Triple sample and hold

/// Updates audio output to take the value of the audio input
/// only when the corresponding pulse input has a rising edge.

/// If audio input not connected, sample random noise instead.

/// If pulse input not connected, update output every sample,
/// (tracking audio input if connected, or producing white noise
/// if audio input not connected.)

/// CV 1 output is controlled by the switch.
/// If switch is up, CV 1 output tracks CV 1 input
/// If switch is in middle position, CV 1 output is held constant
/// CV 1 output is set to CV 1 input when the switch is moved from middle to down


PIO pio = pio0;
uint pin = 0;
uint8_t tx_buffer[64] = {0};
uint8_t rx_buffer[64] = {0};
static volatile bool stop_pending = false;
static volatile uint8_t stop_bytes = 0;
static volatile uint8_t address = 0;
static volatile uint index = 0;
static volatile bool is_read = false;


// I2C handlers run in interrupt context.
// Keep them short and non-blocking. Avoid printf/Serial, delays,
// or other slow operations.

void i2c_receive_handler(uint8_t data, bool is_address) {
    if (is_address) {
        address = data;
        index = 0;
        is_read = false;
    } else {
        is_read = true;
        rx_buffer[index++] = data;
    }
}

void i2c_request_handler(uint8_t address) {
    is_read = false;
    switch (address) {
        case 0x70:
            tx_buffer[0] = 0x10;
            tx_buffer[1] = 0x11;
            tx_buffer[2] = 0x12;
            break;
        case 0x71:
            sprintf((char *)tx_buffer, "Hello, I'm %X", address);
            break;
    }
}

void i2c_stop_handler(uint8_t length) {
    stop_bytes = length;
    stop_pending = true;
}


class TelexIO : public ComputerCard
{
public:

    TelexIO()
    {
        // Runs on Core 0.
        multicore_launch_core1(core1);
    }

    static void core1()
    {
        TelexIO* self = (TelexIO*)ThisPtr();
        self->SlowProcessingCore();
    }

	void StartupAnimation()
    {
        for (int i = 0; i < 6; i++)
        {
            LedOn(i);
            sleep_ms(50);
        }

        for (int i = 0; i < 6; i++)
        {
            LedOff(i);
            sleep_ms(50);
        }
    }

    void SlowProcessingCore()
    {
        // This code runs on Core 1.

		// initialise the PIO
        i2c_multi_init(pio, pin);
        i2c_multi_enable_address(0x70);
        i2c_multi_enable_address(0x71);

        i2c_multi_set_receive_handler(i2c_receive_handler);
        i2c_multi_set_request_handler(i2c_request_handler);
        i2c_multi_set_stop_handler(i2c_stop_handler);
        i2c_multi_set_write_buffer(tx_buffer);

        while (true)  // Core 1 processing
        {
			if (stop_pending)
			{
				uint8_t bytes = stop_bytes;

				stop_pending = false;

				if (bytes > 0)
				{
					uint8_t last_byte = rx_buffer[bytes - 1];

					LedOn(0, last_byte & 0x01);
					LedOn(1, last_byte & 0x02);
					LedOn(2, last_byte & 0x04);
					LedOn(3, last_byte & 0x08);
					LedOn(4, last_byte & 0x10);
					LedOn(5, last_byte & 0x20);

					printf("command %d \n",last_byte);
				}
			}

        	tight_loop_contents();
        }
    }

    void ProcessSample() override
    {
        // Runs on Core 0 at 48 kHz.
    }
};

TelexIO telex;

int main()
{
	//set_sys_clock_khz(144000, true);
	set_sys_clock_khz(192000, true);

	stdio_init_all(); // for printf to serial

	telex.StartupAnimation();
	
	telex.Run();
}


  
