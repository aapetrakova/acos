  .intel_syntax noprefix

  .text
  .global add_scanf

add_scanf:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  lea rdi, [scanf_format_string]
  lea rsi, [rsp]
  lea rdx, [rsp + 8]
  call scanf
  mov rax, [rsp]
  add rax, [rsp + 8]
  add rsp, 16
  pop rbp
  ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"

