# DAC Wavetable Tone Generator with ADC Frequency Control

DAC is configured to stream 128 element wave lookup tables via DMA with its frequency controlled by
ADC (such as a potentiometer connected as voltage divider). Square, Saw, Sine and Triangle waves are supported
and can be cycled with the B1 button. Maximum frequency is set at 10khz while minimum is around 6hz.
DAC's internal output buffer is enabled to prevent influence of low impedance loads such as a speaker.

Noise floor of the DAC device appears to be low and can be used for simple tone generation and playing tunes.
