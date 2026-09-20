#include "ComputerCard.h"
#include "telexio.h"
#include "lut.h"

// FIXMEs
// use of random in the oscillator
// oscillators seem sharp??
// rework header includes and inline of oracle functions
// consider LUT for v/oct
// move any TO operands that take too long to oracle

TelexIO telex; // not on the stack

int main()
{
	set_sys_clock_khz(192000, true);

	stdio_init_all(); // for printf to serial
    
    // TelexI
    telex.InitTelexI(); 

    // TelexO
    reciprocal_init();
    init_clz_lut();
    telex.InitTelexO();

    // Shared
    multicore_launch_core1(TelexIO::core1);

    telex.Run();
}


  
