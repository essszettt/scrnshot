SECTION code_user
PUBLIC _zxn_border_fastcall

; ==============================================================================
; void zxn_border_fastcall(uint8_t uiColour) __z88dk_fastcall
; ------------------------------------------------------------------------------
; sets colour of border
; ==============================================================================
_zxn_border_fastcall:
  ld   a, l     ; __z88dk_fastcall: erstes (einziges) Arg liegt in L
  call 0x2294   ; BORDER-Routine im 48K-ROM
  ret
