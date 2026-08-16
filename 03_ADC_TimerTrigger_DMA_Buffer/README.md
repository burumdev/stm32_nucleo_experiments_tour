# ADC Timer Triggered Sampler with DMA and Double Buffering

ADC in this example samples triggered by the overflow of Timer 3 (16 bit). Which is around
10 hertz frequency sampling rate for the purposes of this example.

UART output to computer is also added, which means frequency is kept very low to avoid race conditions.

DMA is configured for double buffer technique in which the writing and reading halves of the buffer
are swapped between DMA (writer) and user code (reader) to ease synchronization and avoid race conditions.

