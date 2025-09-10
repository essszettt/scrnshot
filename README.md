# scrnshot

Creating screenshots for ZX Spectrum Next (dot command)

This tool can be called from BASIC to take screenshots. Screenshots are saved in BMP format.

The color palette is taken from Wikipedia ([Spectrum Video Modes](https://en.wikipedia.org/wiki/ZX_Spectrum_graphic_modes)) and used to create screenshots in "layer 0" (default video mode).



Not for productive use at the moment - seems to work fine for default Spectrum video mode. Playing around with z88dk functiones to optimize pixel calculations ...

=> Version 0.1.0 will be the first stable release ;-)

---

### USAGE

![help.bmp](https://github.com/essszettt/scrnshot/blob/main/test/help.bmp)

---

### HISTORY

0.0.16  Usage of optimized function to calculate pixel address in video memory
0.0.17  Second LAYER0-palette added
