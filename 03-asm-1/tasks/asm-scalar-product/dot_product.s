.intel_syntax noprefix

.text
.global dot_product

dot_product:
    push rbp
    mov rbp, rsp

    vzeroall

    cmp rdi, 8
    jb process_remainder

vector_loop:
    vmovups ymm0, [rsi]
    vmovups ymm1, [rdx]
    vmulps ymm0, ymm0, ymm1
    vaddps ymm2, ymm2, ymm0

    add rsi, 32
    add rdx, 32
    sub rdi, 8
    cmp rdi, 8
    jge vector_loop

    vperm2f128 ymm3, ymm2, ymm2, 1
    vaddps ymm2, ymm2, ymm3
    vhaddps ymm2, ymm2, ymm2
    vhaddps ymm2, ymm2, ymm2

process_remainder:
    test rdi, rdi
    jz load_result

scalar_loop:
    vmovss xmm0, [rsi]
    vmovss xmm1, [rdx]
    vmulss xmm0, xmm0, xmm1
    vaddss xmm2, xmm2, xmm0

    add rsi, 4
    add rdx, 4
    dec rdi
    jnz scalar_loop

load_result:
    vmovups xmm0, xmm2

    pop rbp
    ret

