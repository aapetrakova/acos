  .text
  .global longest_inc_subseq

longest_inc_subseq:
    cmp x2, 0
    beq return_zero

    mov x3, 1
    mov x4, 0

out_loop:
    cmp x4, x2
    bge exit_out

    mov x5, 0
    mov x7, 1
    str x7, [x1, x4, lsl 3]

in_loop:
    cmp x5, x4
    bge exit_in

    ldr x6, [x0, x4, lsl 3]
    ldr x7, [x0, x5, lsl 3]
    cmp x6, x7
    ble skip

    ldr x8, [x1, x5, lsl 3]
    add x8, x8, 1
    ldr x9, [x1, x4, lsl 3]
    cmp x8, x9
    ble skip

    str x8, [x1, x4, lsl 3]
    cmp x3, x8
    bge skip
    mov x3, x8

skip:
    add x5, x5, 1
    b in_loop

exit_in:
    add x4, x4, 1
    b out_loop

exit_out:
    mov x0, x3
    ret

return_zero:
    mov x0, 0
    ret

