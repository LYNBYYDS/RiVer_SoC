.section .text
.global main

main:
    ori x17,x0,10 # x1 = 10 // 1010 
    bne x17, x0, _bad
    add x1, x1, x2
    beq x17, x0, _good
    nop
    nop

