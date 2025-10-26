       _____                _____ __          __ 
      / ___/______________ / ___// /_  ____  / /_
      \__ \/ ___/ ___/ __ \\__ \/ __ \/ __ \/ __/
     ___/ / /__/ /  / / / /__/ / / / / /_/ / /_  
    /____/\___/_/  /_/ /_/____/_/ /_/\____/\__/  

Creating screenshots on ZX Spectrum Next (dot command)

This tool can be called from BASIC or the command line to take screenshots in BMP format.



Following layers are supported at the moment:

* LAYER 0 (256 x 192 x 16 colours)
* LAYER 1,0 (Timex LoRes: 128 x 92 x 16/256 colours)
* LAYER 1,1 (256 x 192 x 16 colours)
* LAYER 1,2 (Timex HiRes: 512 x 192 x 2 colours)
* LAYER 1,3 (Timex HiColor: 256 x 192 x 16 colours)
* LAYER 2,0 (256 x 192 x 256 colours)


Mixing of different active layers is not supported at the moment (will be added in future releases).

Development is done using z88dk (version 2.4) and Visual Studio Code on Windows. 


---

### USAGE

![help.bmp](https://github.com/essszettt/scrnshot/blob/main/doc/help.bmp)

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
- 0.0.26  Support for LAYER 1,0 added (Timex LoRes)
- 0.1.0   First official release
