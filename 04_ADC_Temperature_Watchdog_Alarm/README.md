# ADC Temperature Hysteresis Window Watchdog Alarm with DMA

Given a temperature upper threshold of 32 C, a high temperature alarm will
be turned on if the ADC temperature sensor value exceeds the threshold.
User LED will blink fast and UART output will print the temperature value to the console.

ADC is used in analog watchdog mode with both DMA and watchdog interrupt modes and triggers a window interrupt when temperature readings sway out of limits.
Normally the main spin loop should be run in low power mode to take advantage of low power nature of this kind of setup
but for the purposes of this demo, spin loop runs normally.

Both Temperature and voltage reference channels are used for accurate temperature readings.

DMA is used in continuous request mode to fill in a tuple array of (temperature, voltageRef)

System returns back to "normal operation" if the temperature returns to normal levels (31 degrees or lower).
These high and low thresholds create a hysteresis window.

Easiest way to test is to light a lighter or match near the chip to increase the temperature. Be careful not to burn the chip :)
