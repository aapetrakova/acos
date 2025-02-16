.text
.global longest_inc_subseq

longest_inc_subseq:
    cmp x2, 0
    beq return_zero

    mov x3, 1
    mov x4, 0

outer_loop:
    cmp x4, x2
    bge exit_outer_loop

    mov x5, 0
    str wzr, [x1, x4, lsl #3]

inner_loop:
    cmp x5, x4
    bge exit_inner_loop

    ldr x6, [x0, x4, lsl #3]
    ldr x7, [x0, x5, lsl #3]
    cmp x6, x7
    ble skip_assignment

    ldr x8, [x1, x5, lsl #3]
    add x8, x8, 1
    ldr x9, [x1, x4, lsl #3]
    cmp x8, x9
    ble skip_assignment

    str x8, [x1, x4, lsl #3]
    cmp x3, x8
    bge skip_assignment
    mov x3, x8

skip_assignment:
    add x5, x5, 1
    b inner_loop

exit_inner_loop:
    add x4, x4, 1
    b outer_loop

exit_outer_loop:
    mov x0, x3
    ret

return_zero:
    mov x0, 0
    ret

