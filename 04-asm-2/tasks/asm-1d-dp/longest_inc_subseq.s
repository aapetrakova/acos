  .text
  .global longest_inc_subseq

longest_inc_subseq:
    mov x3, x2
    cbz x3, return_zero

    mov x4, 0
init_loop:
    cmp x4, x3
    bge fill_done
    mov x5, 1
    str x5, [x1, x4, lsl #3]
    add x4, x4, 1
    b init_loop
fill_done:

    mov x6, 1
    mov x4, 1
outer_loop:
    cmp x4, x3
    bge outer_done
    mov x7, 0
inner_loop:
    cmp x7, x4
    bge inner_done
    ldr x8, [x0, x7, lsl #3]
    ldr x9, [x0, x4, lsl #3]
    cmp x8, x9
    ble inner_next
    ldr x10, [x1, x4, lsl #3]
    ldr x11, [x1, x7, lsl #3]
    add x11, x11, 1
    cmp x10, x11
    csel x10, x11, x10, gt
    str x10, [x1, x4, lsl #3]
inner_next:
    add x7, x7, 1
    b inner_loop
inner_done:

    ldr x10, [x1, x4, lsl #3]
    cmp x6, x10
    csel x6, x10, x6, gt
    add x4, x4, 1
    b outer_loop
outer_done:

return_zero:
    mov x0, 0
    ret

return_result:
    mov x0, x6
    ret

