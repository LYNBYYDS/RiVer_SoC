#include "ifetch.h"

void ifetch::fetch_method() {
    sc_bv<if2dec_size> if2dec_in_var;
    sc_bv<if2dec_size> if2dec_out_var = if2dec_out.read();
    if (EXCEPTION_SM.read() == 0) {

        // if the branch is taken and branch is in the pred branch cache then give the icache the target instruction address otherwise give the next instruction address
        if (PRED_BRANCH_MISS_OUT_SP.read() && pb_taken.read()) { 
            PC_SI.write(PRED_BRANCH_TARGET_ADR_OUT_SP.read());
        } else {
            PC_SI.write(PC_RD.read());
        }

        //Input of IF2DEC fifo
            // variable internal to charge all information need to pass through IF2DEC fifo
            if2dec_in_var.range(101, 100)   =   (sc_bv_base)PRED_BRANCH_PNT_OUT_SP.read();
            if2dec_in_var.range(99, 98)     =   (sc_bv_base)PRED_BRANCH_CPT_OUT_SP.read();
            if2dec_in_var[97]               =   PRED_BRANCH_LRU_OUT_SP.read();
            if2dec_in_var.range(96, 65)     =   (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read();
            if2dec_in_var[64]               =   PRED_BRANCH_MISS_OUT_SP.read();
            if2dec_in_var.range(63, 32)     =   (sc_bv_base)INST_SIC.read();
            if2dec_in_var.range(31, 0)      =   (pb_taken.read() && PRED_BRANCH_MISS_OUT_SP.read()) ? (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read() : (sc_bv_base)PC_RD.read();

            // charge the value to type sc_signal
            if2dec_in.write(if2dec_in_var);

        // Output of IF2DEC fifo
            PRED_TAKEN_RI.write((bool)if2dec_out_var[96]);
            PRED_BRANCH_ADR_RI.write((sc_bv_base)if2dec_out_var.range(95, 64));
            INSTR_RI.write((sc_bv_base)if2dec_out_var.range(63, 32));
            PC_IF2DEC_RI.write((sc_bv_base)if2dec_out_var.range(31, 0));
            
            if (IF2DEC_FLUSH_SD.read()) 
            {
                if2dec_push.write(false);
                DEC2IF_POP_SI.write(true);
                PC_VALID_SI.write(false);
            } 
            else 
            {
                /* stall if :
                    the memory stalls
                    can't push to if2dec
                    dont have PC to pop from dec2if
                */
                bool stall = STALL_SIC.read() || if2dec_full.read() || DEC2IF_EMPTY_SI.read();
                if2dec_push.write(!stall);
                DEC2IF_POP_SI.write(!stall);
                PC_VALID_SI.write(!DEC2IF_EMPTY_SI.read());
            }

    } else {
        // data sent in if2dec
        // If an exception is detected
        // Pipeline pass in M-mode
        // Fifo send nop instruction
        if2dec_in_var.range(101, 64) = 0;
        if2dec_in_var.range(63, 32) = nop_encoding;
        if2dec_in_var.range(31, 0) = (pb_taken.read() && PRED_BRANCH_MISS_OUT_SP.read()) ? (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read() : (sc_bv_base)PC_RD.read();

        if (pb_taken.read() && PRED_BRANCH_MISS_OUT_SP.read()) {
            PC_SI.write(PRED_BRANCH_TARGET_ADR_OUT_SP.read());
        } else {
            PC_SI.write(PC_RD.read());
        }

        if2dec_in.write(if2dec_in_var);

        // data coming out from if2dec :
        if2dec_in_var.range(101, 100)   =   (sc_bv_base)PRED_BRANCH_PNT_OUT_SP.read();
        if2dec_in_var.range(99, 98)     =   (sc_bv_base)PRED_BRANCH_CPT_OUT_SP.read();
        if2dec_in_var[97]               =   PRED_BRANCH_LRU_OUT_SP.read();
        if2dec_in_var.range(96, 65)     =   (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read();
        if2dec_in_var[64]               =   PRED_BRANCH_MISS_OUT_SP.read();
        if2dec_in_var.range(63, 32)     =   (sc_bv_base)INST_SIC.read();
        if2dec_in_var.range(31, 0)      =   (pb_taken.read() && PRED_BRANCH_MISS_OUT_SP.read()) ? (sc_bv_base)PRED_BRANCH_TARGET_ADR_OUT_SP.read() : (sc_bv_base)PC_RD.read();
        if2dec_push.write(true);

        DEC2IF_POP_SI.write(true);
        PC_VALID_SI.write(false);
    }
}

void ifetch::exception()
// No exception in IFECTH
{
    EXCEPTION_RI.write(0);
}

void ifetch::trace(sc_trace_file* tf) {
    sc_trace(tf, INST_SIC, GET_NAME(INST_SIC));
    sc_trace(tf, STALL_SIC, GET_NAME(STALL_SIC));
    sc_trace(tf, PC_SI, GET_NAME(PC_SI));
    sc_trace(tf, PC_VALID_SI, GET_NAME(PC_VALID_SI));
    sc_trace(tf, DEC2IF_EMPTY_SI, GET_NAME(DEC2IF_EMPTY_SI));
    sc_trace(tf, DEC2IF_POP_SI, GET_NAME(DEC2IF_POP_SI));
    sc_trace(tf, PC_RD, GET_NAME(PC_RD));
    sc_trace(tf, IF2DEC_FLUSH_SD, GET_NAME(IF2DEC_FLUSH_SD));
    sc_trace(tf, IF2DEC_POP_SD, GET_NAME(IF2DEC_POP_SD));
    sc_trace(tf, IF2DEC_EMPTY_SI, GET_NAME(IF2DEC_EMPTY_SI));
    sc_trace(tf, INSTR_RI, GET_NAME(INSTR_RI));
    sc_trace(tf, if2dec_push, GET_NAME(if2dec_push));
    sc_trace(tf, if2dec_full, GET_NAME(if2dec_full));
    sc_trace(tf, IF2DEC_EMPTY_SI, GET_NAME(IF2DEC_EMPTY_SI));
    sc_trace(tf, INSTR_RI, GET_NAME(INSTR_RI));
    sc_trace(tf, PC_IF2DEC_RI, GET_NAME(PC_IF2DEC_RI));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET, GET_NAME(RESET));
    sc_trace(tf, if2dec_in, GET_NAME(if2dec_in));
    sc_trace(tf, if2dec_out, GET_NAME(if2dec_out));
    sc_trace(tf, EXCEPTION_RI, GET_NAME(EXCEPTION_RI));
    sc_trace(tf, EXCEPTION_SM, GET_NAME(EXCEPTION_SM));
    sc_trace(tf, INTERRUPTION_SE, GET_NAME(INTERRUPTION_SE));
    sc_trace(tf, MRET_SM, GET_NAME(MRET_SM));
    sc_trace(tf, INSTR_IS_BRANCH_RD, GET_NAME(INSTR_IS_BRANCH_RD));
    sc_trace(tf, BRANCH_INST_ADR_RD, GET_NAME(BRANCH_INST_ADR_RD));
    sc_trace(tf, BRANCH_TARGET_ADR_RD, GET_NAME(BRANCH_TARGET_ADR_RD));
    sc_trace(tf, PRED_BRANCH_ADR_RI, GET_NAME(PRED_BRANCH_ADR_RI));
    sc_trace(tf, PRED_TAKEN_RI, GET_NAME(PRED_TAKEN_RI));
    sc_trace(tf, PRED_NEXT_ADR_SI, GET_NAME(PRED_NEXT_ADR_SI));
    sc_trace(tf, PRED_ADR_TAKEN_SI, GET_NAME(PRED_ADR_TAKEN_SI));
    sc_trace(tf, pred_write_pointer_si, GET_NAME(pred_write_pointer_si));
    sc_trace(tf, PRED_SUCCESS_RD, GET_NAME(PRED_SUCCESS_RD));
    sc_trace(tf, next_state_pred_si, GET_NAME(next_state_pred_si));
    sc_trace(tf, PRED_FAILED_RD, GET_NAME(PRED_FAILED_RD));

    for (int i = 0; i < predictor_register_size; i++) {
        std::string regname = "REG_ADR_";
        regname += std::to_string(i);
        sc_trace(tf, BRANCH_ADR_REG[i], signal_get_name(BRANCH_ADR_REG[i].name(), regname.c_str()));

        regname = "REG_PRED_";
        regname += std::to_string(i);
        sc_trace(tf, PREDICTED_ADR_REG[i], signal_get_name(PREDICTED_ADR_REG[i].name(), regname.c_str()));

        regname = "REG_STATE_";
        regname += std::to_string(i);
        sc_trace(tf, PRED_STATE_REG[i], signal_get_name(PRED_STATE_REG[i].name(), regname.c_str()));
    }

    for (int i = 0; i < ret_stack_size; i++) {
        std::string regname = "STACK_ADR_";
        regname += std::to_string(i);
        sc_trace(tf, RET_STACK_RI[i], signal_get_name(RET_STACK_RI[i].name(), regname.c_str()));
    }

    for (int i = 0; i < ret_predictor_register_size; i++) {
        std::string regname = "RET_REG_ADR_";
        regname += std::to_string(i);
        sc_trace(tf, RET_ADR_RI[i], signal_get_name(RET_ADR_RI[i].name(), regname.c_str()));
    }

    sc_trace(tf, PUSH_ADR_RAS_RD, GET_NAME(PUSH_ADR_RAS_RD));
    sc_trace(tf, POP_ADR_RAS_RD, GET_NAME(POP_ADR_RAS_RD));
    sc_trace(tf, RETURN_ADR_RD, GET_NAME(RETURN_ADR_RD));
    sc_trace(tf, RET_INST_RD, GET_NAME(RET_INST_RD));
    sc_trace(tf, ret_stack_pointer_si, GET_NAME(ret_stack_pointer_si));
    sc_trace(tf, pred_branch_taken_si, GET_NAME(pred_branch_taken_si));
    sc_trace(tf, pred_ret_taken_si, GET_NAME(pred_ret_taken_si));
    sc_trace(tf, pred_ret_next_adr_si, GET_NAME(pred_ret_next_adr_si));
    sc_trace(tf, pred_branch_next_adr_si, GET_NAME(pred_branch_next_adr_si));

    fifo_if2dec.trace(tf);
}