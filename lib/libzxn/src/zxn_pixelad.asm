SECTION code_user
PUBLIC _zxn_pixelad_callee

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
