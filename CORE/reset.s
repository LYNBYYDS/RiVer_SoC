.section .reset
.global _reset

# What's need to be initialized :
# CPU starts in kernel mode
# It must load :
# * main adress
# * exception handler adress
# * 

_reset:
    la x28,0x10054       #loading main adress
    la x29,_exception    #loading exception handler adress
    csrrw x0, 0x341,x28  # writting main adress in mepc
    csrrw x0, 0x305,x29  # writting exception handler adress in mtvec
    li x2,0x10000        # sp initialization

    # loading isr adresses
    la x9, _isr_vector
    la x10, _instruction_address_fault
    la x11, _illegal_instruction
    la x12, _instruction_address_misagligned
    la x13, _env_call_u_mode
    la x14, _env_call_s_mode
    la x15, _env_call_m_mode
    la x16, _load_adress_missaligned
    la x17, _store_adress_missaligned
    la x18, _load_access_fault
    la x19, _store_access_fault
    la x20, _env_call_wrong_mode
    
    #storing isr adresses in isr vector
    
    sw x12, 0(x9)
    sw x10, 4(x9)
    sw x11, 8(x9)
    sw x16, 16(x9)
    sw x18, 20(x9)
    sw x17, 24(x9)
    sw x19, 28(x9)
    sw x13, 32(x9)
    sw x14, 36(x9)
    sw x15, 40(x9)
    sw x20, 96(x9) #_env_call_wrong mode set in custom use 24

    # reseting register value

    la x3, 0
    la x4, 0
    la x5, 0
    la x6, 0
    la x7, 0
    la x8, 0
    la x9, 0
    la x10, 0
    la x11, 0
    la x12, 0
    la x13, 0
    la x14, 0
    la x15, 0
    la x16, 0
    la x17, 0
    la x18, 0
    la x19, 0
    la x20, 0
    la x21, 0

    mret