  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
  push rbp
  mov rbp, rsp
  push rdi
  cmp edx, 8
  jb finish_fast

  fast:
  sub edx, 8
  mov rax, [rsi]
  mov [rdi], rax
  add rdi, 8
  add rsi, 8
  cmp edx, 8
  jae fast

  finish_fast:
  test edx, edx
  jz finish

  slow:
  dec edx
  mov al, [rsi]
  mov [rdi], al
  add rdi, 1
  add rsi, 1
  test edx, edx
  jnz slow

  finish:
  pop rax
  pop rbp
  ret
