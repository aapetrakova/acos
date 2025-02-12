  .text
  .global add_scanf

  .macro push Xn
    sub sp, sp, 16
    str \Xn, [sp]
  .endm

  .macro pop Xn
    ldr \Xn, [sp]
    add sp, sp, 16
  .endm

add_scanf:
  push x29
  mov x29, sp
  sub sp, sp, 16
  ldr x0, =scanf_format_string
  ldr x1, [sp]
  ldr x2, [sp, 8]
  bl scanf
  ldr x0, [sp]
  ldr x1, [sp, 8]
  add x0, x0, x1
  add sp, sp, 16
  pop x29

  ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"
