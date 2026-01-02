SECTION code_user
PUBLIC _zxn_reset_fastcall

; ==============================================================================
; void zxn_reset_fastcall(uint8_t uiMode) __z88dk_fastcall
; ------------------------------------------------------------------------------
; reset/reboot system
; ==============================================================================
_zxn_reset_fastcall:
  ld   a, l   ; __z88dk_fastcall: first (single) argument in L
  nreg $02,a  ; Write to reset-register
  ret         ; Should never be reached
