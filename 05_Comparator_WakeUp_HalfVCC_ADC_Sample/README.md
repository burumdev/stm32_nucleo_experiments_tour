# Comparator Wake-Up From Stop Mode ADC Sample On Demand

Comparator trigger voltage is internal VREF, which is about 1.2 volts. When the trigger voltage is exceeded
ADC steps in to sample continuously triggered by the timer 2, which runs at 10 hertz and then prints the
ADC value to the console. Once Comparator is triggered again, this time to the lower voltages, low power
stop mode will be initiated, which stops timers and ADC sampling which is timer triggered.

L152 hardware comparator doesn't have internal programmable hysteresis (unlike newer l4 series) and it also doesn't route its output to GPIO.
So external hysteresis with a resistor is also not an option. So to prevent unstable back-and-forth "chatter"
with the comparator output, a middle income man's solution with a software debounce logic is employed.

Middle income man is now the new poor man..

A potentiometer connected as voltage divider to both the comparator and ADC input in parallel
is an easy way to evaluate this setup.

