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
    stp x29, x30, [sp, -16]!
    mov x29, sp
    ldr x0, =scanf_format_string
    sub sp, sp, 16
    add x1, sp, 8
    add x2, sp, 0
    bl scanf
    ldr x1, [sp]
    ldr x0, [sp, 8]
    add x0, x0, x1
    add sp, sp, 16
    ldp x29, x30, [sp], 16
    ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"
