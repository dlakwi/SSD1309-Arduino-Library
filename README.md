# SSD1309-Arduino-Library
Arduino library for a bufferless SSD1309 128x64 parallel interface display

The SSD1309 display controller has graphics display memory (GDRAM) of 128x64 1-bit (black & white) pixels. The GDRAM stores eight 1-bit pixels in each addressable page byte. So, the GDRAM address range is 128x8.

The parallel interface permits the reading of GDRAM, so individual pixels can be read and written. Since GDRAM can be read, no display buffer is needed. Pixels are packed and unpacked when writing and reading.

The OLED display used for this development was from Aliexpress.
https://www.aliexpress.com/item/32919030007.html
