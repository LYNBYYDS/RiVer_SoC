#include "ifetch.h"

void ifetch::fetch_method() {
    // Determine the value of adr_si
        if (!miss_ri.read() && cpt.ri.read() >= 2) {    // If the last instruction is a branchinstruction and the prediction is branch success
            adr_si.write((sc_bv_base)target_adr_ri.read()); // take the target address to search in icache also in the pred branch cache
        } else {
            adr_si.write((sc_bv_base)PC_RD.read());         // otherwise use nest-pc to search in the icache and the pred branch cache
        }
        ADR_SI.write((sc_bv_base)adr_si);

    // Let the pred cache search 
        PRED_BRANCH_CHECK_ADR_IN_SI.write((sc_bv_base)adr_si);

    // FIFO pred_branch concat & unconcat
        sc_bv<pb_if2dec_SIZE> pb_if2dec_in_var;
        pb_if2dec_in_var.range(69, 68)    =   (sc_bv_base)PRED_BRANCH_PNT_OUT_SP.read();                      // PNT
        pb_if2dec_in_var.range(67, 66)    =   (sc_bv_base)PRED_BRANCH_CPT_OUT_SP.read();                      // CPT
        pb_if2dec_in_var[65]              =   PRED_BRANCH_LRU_OUT_SP.read();                                  // LRU
        pb_if2dec_in_var.range(64, 33)    =   (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read();               // @ target
        pb_if2dec_in_var.range(32, 1)     =   (sc_bv_base)PC_RD.read();                                       // @ branch
        pb_if2dec_in_var[0]               =   PRED_BRANCH_MISS_OUT_SP.read();                                 // MISS/HIT
        pb_if2dec_in_si.write(pb_if2dec_in_var);

        sc_bv<pb_if2dec_SIZE> pb_if2dec_out_var = pb_if2dec_out_si;
        PRED_BRANCH_CPT_RI;.write((sc_bv_base)if2dec_out_var.range(69, 68));            // CPT
        PRED_BRANCH_LRU_RI.write((sc_bv_base)if2dec_out_var.range(67, 66));             // Less Recent Use
        PRED_BRANCH_PNT_RI.write((sc_bv_base)if2dec_out_var[65];                        // branch taken target address
        PRED_BRANCH_TARGET_ADR_RI.write((sc_bv_base)if2dec_out_var.range(64, 33));;     // branch target address
        PRED_BRANCH_ADR_RI.write((sc_bv_base)if2dec_out_var.range(32, 1));              // branch instruction address
        PRED_BRANCH_MISS_RI.write((sc_bv_base)if2dec_out_var[0]);                       // MISS/HIT for the cache MISS = 1 HIT = 0

    // fifo target_pc concat & unconcat
        // Input data
        sc_bv<TARGET_PC_SIZE> target_pc_in_var;
        target_pc_in_var.range(34, 33)  = (sc_bv_base)PRED_BRANCH_CPT_OUT_SP.read();
        target_pc_in_var.range(32, 1)   = (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read();
        target_pc_in_var[0]             = PRED_BRANCH_MISS_OUT_SP.read();
        target_pc_in_si.write(target_pc_in_var);

        // Output data
        sc_bv<IF2DEC_SIZE> target_pc_out_var = target_pc_out_si;
        cpt_ri.write(target_pc_out_var.range(34, 33));
        target_adr_ri.write(target_pc_out_var.range(32, 1));
        miss_ri.write(target_pc_out_var[0]);
    
    // FIFO if2dec concat & unconcat
        // Input data
        sc_bv<IF2DEC_SIZE> if2dec_in_var;
        if2dec_in_var.range(63, 32) = (sc_bv_base)IC_INST_SI.read();
        if2dec_in_var.range(31, 0)  = (sc_bv_base)adr_si;
        if2dec_in_si.write(if2dec_in_var);

        // Output data
        sc_bv<IF2DEC_SIZE> if2dec_out_var = if2dec_out_si;
        INSTR_RI.write((sc_bv_base)if2dec_out_var.range(63, 32));
        PC_RI.write((sc_bv_base)if2dec_out_var.range(31, 0));

    // FIFO IF2DEC gestion
    bool stall     = IC_STALL_SI || IF2DEC_FULL_SI || DEC2IF_EMPTY_SD;
    
    if (IF2DEC_FLUSH_SD.read()) {
        IF2DEC_PUSH_SI = false;
        DEC2IF_POP_SI  = true;
        ADR_VALID_SI   = false;
    } else {
        // stall if the memory stalls, if we can't push to dec, or have no value
        // of pc to pop from dec 
        IF2DEC_PUSH_SI = !stall;
        DEC2IF_POP_SI  = !stall;
        ADR_VALID_SI   = !DEC2IF_EMPTY_SD;
    }

    // FIFO pb_if2dec gestion
    if (PB_IF2DEC_FLUSH_SD.read()) {
        PB_IF2DEC_PUSH_SI = false;
        PB_IF2DEC_POP_SI  = true;     
    } else {
        PD_IF2DEC_PUSH_SI = !stall;
        PD_IF2DEC_POP_SI = !stall;
    }
}

void ifetch::trace(sc_trace_file* tf) {
    sc_trace(tf, ADR_SI, GET_NAME(ADR_SI));
    sc_trace(tf, ADR_VALID_SI, GET_NAME(ADR_VALID_SI));
    sc_trace(tf, IC_INST_SI, GET_NAME(IC_INST_SI));
    sc_trace(tf, IC_STALL_SI, GET_NAME(IC_STALL_SI));
    sc_trace(tf, DEC2IF_EMPTY_SD, GET_NAME(DEC2IF_EMPTY_SD));
    sc_trace(tf, DEC2IF_POP_SI, GET_NAME(DEC2IF_POP_SI));
    sc_trace(tf, IF2DEC_FLUSH_SD, GET_NAME(IF2DEC_FLUSH_SD));
    sc_trace(tf, IF2DEC_POP_SD, GET_NAME(IF2DEC_POP_SD));
    sc_trace(tf, IF2DEC_PUSH_SI, GET_NAME(IF2DEC_PUSH_SI));
    sc_trace(tf, IF2DEC_FULL_SI, GET_NAME(IF2DEC_FULL_SI));
    sc_trace(tf, IF2DEC_EMPTY_SI, GET_NAME(IF2DEC_EMPTY_SI));
    sc_trace(tf, PC_RD, GET_NAME(PC_RD));
    sc_trace(tf, INSTR_RI, GET_NAME(INSTR_RI));
    sc_trace(tf, PC_RI, GET_NAME(PC_RI));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET, GET_NAME(RESET));
    sc_trace(tf, if2dec_in_si, GET_NAME(if2dec_in_si));
    sc_trace(tf, if2dec_out_si, GET_NAME(if2dec_out_si));
    if2dec.trace(tf);
}