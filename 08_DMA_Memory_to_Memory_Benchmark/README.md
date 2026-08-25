# DMA Memory to Memory Transfer vs Main Thread Transfer Benchmark

A particularly large buffer of 32 Kbytes of word data will be copied from one buffer to the other on SRAM,
by first the regular memcpy() bitwise method in main thread and then using DMA M2M (Memory to Memory) transfer
functionality which can run in the background.

The cycles and time thereof that both operations are measured and printed to the console via UART.

A better example of DMA M2M usage would be that actually does something useful like framebuffer transfers to a TFT LCD color screen.
But for the purposes of this example a benchmark is more appropriate.

