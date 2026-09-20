cmake_minimum_required (VERSION 3.13)
include(pico_sdk_import.cmake)
project(computercard C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

macro (add_example _name)
  add_executable(${ARGV})
  if (TARGET ${_name})

	# Add warnings, in particular about float/double conversion, as it's very easy to end up
	# with slower double-precision calculations when we only mean to use single-precision float.
	target_compile_options(${_name} PRIVATE -Wdouble-promotion -Wfloat-conversion -Wall -Wextra)
	target_link_options(${_name} PRIVATE -Wl,--print-memory-usage)
	
	# Give oscillator more time to start - some boards won't run if this isn't included
	target_compile_definitions(${_name} PRIVATE PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)
	
	target_include_directories(${_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
	target_include_directories(${_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/examples/${_name}/)
    target_link_libraries(${_name} pico_unique_id pico_stdlib hardware_dma hardware_i2c hardware_pwm hardware_adc hardware_spi)
	pico_add_extra_outputs(${_name})
	target_sources(${_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/examples/${_name}/main.cpp)	  
	pico_enable_stdio_usb(${_name} 0)
  endif()
endmacro()

add_example(calibrated_cv_out)

# EightMU.h carries both the usb_midi_host driver and the host-mode TinyUSB
# configuration, so an 8mu card needs no extra sources - just the TinyUSB host
# libraries, and CFG_TUSB_CONFIG_FILE to point TinyUSB at EightMU.h for its
# configuration.
add_example(eightmu)
target_compile_definitions(eightmu PRIVATE CFG_TUSB_CONFIG_FILE="EightMU.h")
target_link_libraries(eightmu pico_multicore tinyusb_host tinyusb_board)

add_example(calibrated_input)

add_example(card_info)

add_example(midi_device)
target_link_libraries(midi_device pico_multicore tinyusb_device tinyusb_board )
target_sources(midi_device PUBLIC ${CMAKE_CURRENT_LIST_DIR}/examples/midi_device/usb_descriptors.c)

add_example(midi_device_host)
target_link_libraries(midi_device_host pico_multicore tinyusb_host tinyusb_board  tinyusb_device  )
target_sources(midi_device_host PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/examples/midi_device_host/usb_midi_host.c
  ${CMAKE_CURRENT_LIST_DIR}/examples/midi_device_host/usb_midi_host_app_driver.c
  ${CMAKE_CURRENT_LIST_DIR}/examples/midi_device_host/usb_descriptors.c
)

add_example(hid_keyboard_mouse)
target_link_libraries(hid_keyboard_mouse pico_multicore tinyusb_host tinyusb_board)

add_example(midi_host)
target_sources(midi_host PUBLIC ${CMAKE_CURRENT_LIST_DIR}/examples/midi_host/usb_midi_host.c  ${CMAKE_CURRENT_LIST_DIR}/examples/midi_host/usb_midi_host_app_driver.c)
target_link_libraries(midi_host pico_multicore tinyusb_host tinyusb_board  )

add_example(normalisation_probe)
  
add_example(passthrough)

add_example(sample_and_hold)

add_example(web_interface)
target_link_libraries(web_interface pico_multicore tinyusb_device tinyusb_board )
target_sources(web_interface PUBLIC ${CMAKE_CURRENT_LIST_DIR}/examples/web_interface/usb_descriptors.c)

add_example(sample_upload)

add_example(second_core)
target_link_libraries(second_core pico_multicore)

add_example(sine_wave_lookup)

add_example(sine_wave_float)

add_example(usb_detect)

add_example(usb_serial)
target_link_libraries(usb_serial pico_multicore)
pico_enable_stdio_usb(usb_serial 1)

add_example(telexio)
target_compile_definitions(telexio PRIVATE
  ENABLE_UART_DEBUGGING
  PICO_FLOAT_IN_RAM=1
  PICO_DOUBLE_IN_RAM=1
)
pico_generate_pio_header(telexio ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/i2c_multi.pio)
target_compile_options(telexio PRIVATE
    -O2
    -Wdouble-promotion
    -Wfloat-conversion
    -Wall
    -Wextra
    -Winline
)
target_sources(telexio PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/i2c_multi.c
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/lut.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXi/AnalogReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/Quantizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/TxHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/Oscillator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/CVOutput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/TriggerOutput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/TELEXo/Output.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/telexio/telexio.cpp
)
pico_add_extra_outputs(telexio)
target_link_libraries(telexio
    pico_stdlib
    hardware_irq
    hardware_pio
    hardware_i2c
	  pico_multicore
    pico_rand
)
pico_enable_stdio_usb(telexio 1)
pico_enable_stdio_uart(telexio 0)