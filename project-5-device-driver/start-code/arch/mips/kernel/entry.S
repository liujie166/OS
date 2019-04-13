#include "asm.h"


.equ    KERNEL, 0
.equ    USER,   156

.macro STI
    mfc0 k0, CP0_STATUS 
    or  k0, 0x00008001
    mtc0 k0, CP0_STATUS
.endm

.macro CLI
    mfc0 k0, CP0_STATUS
    and k0, 0xfffffffe
    mtc0 k0, CP0_STATUS
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI
    mtc0    zero, CP0_INDEX
    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
END(asm_start)


.macro SAVE_CONTEXT offset
    // TODO save context
    lw  k0, current_running
    addi k0,k0,\offset
    sw  $2, OFFSET_REG2(k0)
    sw  $3, OFFSET_REG3(k0)
    sw  $4, OFFSET_REG4(k0)
    sw  $5, OFFSET_REG5(k0)
    sw  $6, OFFSET_REG6(k0)
    sw  $7, OFFSET_REG7(k0)
    sw  $8, OFFSET_REG8(k0)
    sw  $9, OFFSET_REG9(k0)
    sw  $10, OFFSET_REG10(k0)
    sw  $11, OFFSET_REG11(k0)
    sw  $12, OFFSET_REG12(k0)
    sw  $13, OFFSET_REG13(k0)
    sw  $14, OFFSET_REG14(k0)
    sw  $15, OFFSET_REG15(k0)
    sw  $16, OFFSET_REG16(k0)
    sw  $17, OFFSET_REG17(k0)
    sw  $18, OFFSET_REG18(k0)
    sw  $19, OFFSET_REG19(k0)
    sw  $20, OFFSET_REG20(k0)
    sw  $21, OFFSET_REG21(k0)
    sw  $22, OFFSET_REG22(k0)
    sw  $23, OFFSET_REG23(k0)
    sw  $24, OFFSET_REG24(k0)
    sw  $25, OFFSET_REG25(k0)
    sw  $28, OFFSET_REG28(k0)
    sw  $29, OFFSET_REG29(k0)
    sw  $30, OFFSET_REG30(k0)
    sw  $31, OFFSET_REG31(k0)
    mfc0 k1, CP0_STATUS
    sw   k1, OFFSET_STATUS(k0)
    mfhi k1
    sw   k1, OFFSET_HI(k0)
    mflo k1
    sw   k1, OFFSET_LO(k0)
    mfc0 k1, CP0_BADVADDR
    sw   k1, OFFSET_BADVADDR(k0)
    mfc0 k1, CP0_CAUSE
    sw   k1, OFFSET_CAUSE(k0)
    mfc0 k1, CP0_EPC
    sw   k1, OFFSET_EPC(k0)
    mfc0 k1, CP0_ENTRYHI
    sw   k1, OFFSET_ENTRYHI(k0)
.endm

.macro RESTORE_CONTEXT offset
    // TODO restore context
    lw   k0,current_running
    addi k0,k0,\offset
    lw  $2, OFFSET_REG2(k0)
    lw  $3, OFFSET_REG3(k0)
    lw  $4, OFFSET_REG4(k0)
    lw  $5, OFFSET_REG5(k0)
    lw  $6, OFFSET_REG6(k0)
    lw  $7, OFFSET_REG7(k0)
    lw  $8, OFFSET_REG8(k0)
    lw  $9, OFFSET_REG9(k0)
    lw  $10, OFFSET_REG10(k0)
    lw  $11, OFFSET_REG11(k0)
    lw  $12, OFFSET_REG12(k0)
    lw  $13, OFFSET_REG13(k0)
    lw  $14, OFFSET_REG14(k0)
    lw  $15, OFFSET_REG15(k0)
    lw  $16, OFFSET_REG16(k0)
    lw  $17, OFFSET_REG17(k0)
    lw  $18, OFFSET_REG18(k0)
    lw  $19, OFFSET_REG19(k0)
    lw  $20, OFFSET_REG20(k0)
    lw  $21, OFFSET_REG21(k0)
    lw  $22, OFFSET_REG22(k0)
    lw  $23, OFFSET_REG23(k0)
    lw  $24, OFFSET_REG24(k0)
    lw  $25, OFFSET_REG25(k0)
    lw  $28, OFFSET_REG28(k0)
    lw  $29, OFFSET_REG29(k0)
    lw  $30, OFFSET_REG30(k0)
    lw  $31, OFFSET_REG31(k0)

    lw   k1, OFFSET_HI(k0)
    mthi k1
    lw   k1, OFFSET_LO(k0)
    mtlo k1
    lw   k1, OFFSET_STATUS(k0)
    mtc0 k1, CP0_STATUS
    lw   k1, OFFSET_BADVADDR(k0)
    mtc0 k1, CP0_BADVADDR
    lw   k1, OFFSET_EPC(k0)
    mtc0 k1, CP0_EPC
    lw   k1, OFFSET_ENTRYHI(k0)
    mtc0 k1, CP0_ENTRYHI
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal   scheduler
    RESTORE_CONTEXT(KERNEL)
    jr    ra
END(do_scheduler)


LEAF(enable_interrupt)
    li  k0, 0x00008001
    mtc0 k0, CP0_STATUS
    jr ra
END(enable_interrupt)

LEAF(disable_interrupt)
    mtc0 zero, CP0_STATUS
    jr ra
END(disable_interrupt)

LEAF(reset_com_and_cnt)
    mtc0  zero, CP0_COUNT
    li    k1, 0x00020000
    mtc0  k1, CP0_COMPARE
    jr ra
END(reset_com_and_cnt)

.global exception_handler_begin
.global exception_handler_end
.global TLBexception_handler_begin
.global TLBexception_handler_end

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:
    //TODO: TLB exception entry
    CLI
    // save user context
    SAVE_CONTEXT(USER)
    // switch to kernel stack
    lw  k0, current_running
    lw  sp, OFFSET_REG29(k0)
    //jmp exception_handler[i] which decided by CP0_CAUSE
    mfc0 a0, CP0_CAUSE
    jal exception_helper
TLBexception_handler_end:
END(TLBexception_handler_entry)

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // TODO close interrupt
    CLI
    // save user context
    SAVE_CONTEXT(USER)
    // switch to kernel stack
    lw  k0, current_running
    lw  sp, OFFSET_REG29(k0)
    // jmp exception_handler[i] which decided by CP0_CAUSE
    mfc0 a0, CP0_CAUSE
    mfc0 a1, CP0_BADVADDR
    jal exception_helper
    // Leve2 exception Handler.
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    mfc0 a0, CP0_STATUS
    mfc0 a1, CP0_CAUSE
    jal interrupt_helper
    // Leve3 exception Handler.
    RESTORE_CONTEXT(USER)
    //enable interrupt
    STI
    eret
    // Leve2 exception Handler.
END(handle_int)

NESTED(handle_TLB, 0, sp)
    // interrupt handler
    mfc0 a0, CP0_BADVADDR
    jal do_TLB_Refill
    // Leve3 exception Handler.
    RESTORE_CONTEXT(USER)
    //enable interrupt
    STI
    eret
    // Leve2 exception Handler.
END(handle_TLB)

NESTED(handle_syscall, 0, sp)
    // system call handler
    lw k0,current_running
    addi k0,k0,USER
    lw  a0, OFFSET_REG4(k0)
    lw  a1, OFFSET_REG5(k0)
    lw  a2, OFFSET_REG6(k0)
    lw  a3, OFFSET_REG7(k0)
    jal system_call_helper
    RESTORE_CONTEXT(USER)
    mfc0 k0, CP0_EPC
    addu k0, 0x4
    mtc0 k0, CP0_EPC
    STI
    // Leve3 exception Handler.
    eret
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
    RESTORE_CONTEXT(USER)
    //enable interrupt
    mtc0  zero, CP0_COUNT
    li    k1, 0x00020000
    mtc0  k1, CP0_COMPARE
    STI
    eret
END(handle_other)

LEAF(get_cp0_count)
    mfc0 v0,CP0_COUNT
    jr ra
END(get_cp0_count)

LEAF(task_start)
    RESTORE_CONTEXT(USER)
    mtc0  zero, CP0_COUNT
    li    k1, 0x00020000
    mtc0  k1, CP0_COMPARE
    STI
    eret
END(task_start)

LEAF(tlb_fill)
    mtc0    a0, CP0_ENTRYHI                                                                         
    mtc0    a1, CP0_ENTRYLO0                                                                             
    mtc0    a2, CP0_ENTRYLO1                                                                                                                                                                                           
    mtc0    zero,CP0_PAGEMASK
    mfc0    k0, CP0_INDEX
    addu    k0, 0x1                                           
    mtc0    k0, CP0_INDEX
    //tlbp
    tlbwr
    jr ra
END(tlb_fill)

