SECTION code_user
PUBLIC _zxn_pixelad_callee
PUBLIC _zxn_border_fastcall

; ==============================================================================
; uint8_t* zxn_pixelad_callee(uint8_t x, uint8_t y) __z88dk_callee
; ------------------------------------------------------------------------------
; calculates address of pixal at coordinate x, y
; ==============================================================================
_zxn_pixelad_callee:
  pop hl
  ex (sp),hl
  push de

  ld e, l    ; E = x
  ld d, h    ; D = y 

  ; INPUT:   DE (D = Y, E = X)
  ; OUTPUT:  HL (address of pixel byte)
  pixelad

  pop de
  ret

; ==============================================================================
; void zxn_border_fastcall(uint8_t uiColour) __z88dk_fastcall
; ------------------------------------------------------------------------------
; sets colour of border
; ==============================================================================
_zxn_border_fastcall:
  ld   a, l     ; __z88dk_fastcall: erstes (einziges) Arg liegt in L
  call 0x2294   ; BORDER-Routine im 48K-ROM
  ret
