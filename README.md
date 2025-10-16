       _____                _____ __          __ 
      / ___/______________ / ___// /_  ____  / /_
      \__ \/ ___/ ___/ __ \\__ \/ __ \/ __ \/ __/
     ___/ / /__/ /  / / / /__/ / / / / /_/ / /_  
    /____/\___/_/  /_/ /_/____/_/ /_/\____/\__/  
                                             

Creating screenshots for ZX Spectrum Next (dot command)

This tool can be called from BASIC or the command line to take screenshots in BMP format.

~~The color palette is taken from Wikipedia ([Spectrum Video Modes](https://en.wikipedia.org/wiki/ZX_Spectrum_graphic_modes)) and used to create screenshots in "layer 0" (default video mode).~~

Not for productive use at the moment - seems to work fine for default Spectrum video mode (and LAYER 1,1, LAYER 1,2, LAYER 1,3, LAYER 2 ;-)). Playing around with z88dk functions to optimize pixel calculations ...

=> Version 0.1.0 will be the first complete release ;-)

---

### USAGE

![help.bmp](https://github.com/essszettt/scrnshot/blob/main/test/help.bmp)

---

### HISTORY

- 0.0.16  Usage of optimized function to calculate pixel address in video memory
- 0.0.17  Second LAYER 0-palette added
- 0.0.19  First version that creates basic LAYER 2 screenshots
- 0.0.20  First implementation of further LAYER 2 modes (bad palette handling)
- 0.0.21  First support for LAYER 1,1; handling of FLASH attribute in LAYER 0
- 0.0.23  First release that works reliably in LAYER 0, LAYER 1,1 and LAYER 2
- 0.0.24  Support for LAYER 1,3 added (Timex HiColor)
- 0.0.25  Support for LAYER 1,2 added (Timex HiRes) 

