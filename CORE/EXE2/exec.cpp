#include "exec.h"
#include "alu.h"
#include "shifter.h"

void exec_s2::preprocess_op() {
    sc_uint<32> op1 = op1_se_s2.read();
    sc_uint<32> op2 = op2_se.read();
    if (NEG_OP2_RD_S2.read()) {
        alu_in_op2_se.write(~op2);
    } else {
        alu_in_op2_se.write(op2);
    }
    shift_val_se.write(op2.range(4, 0));

    OP1_SE_S2.write(op1_se_s2);
    OP2_SE_S2.write(op2_se);
}

void exec_s2::select_exec_res() {
    sc_uint<32> alu_out     = alu_out_se.read();
    sc_uint<32> shifter_out = shifter_out_se.read();

    /*if (SELECT_TYPE_OPERATIONS_RD_S2.read() == 0b1000)
    {
        exe_res_se_s2.write(divider_out_se);
    }
    else */
    if (SELECT_TYPE_OPERATIONS_RD_S2.read() == 0b0010) {
        exe_res_se_s2.write(shifter_out_se);
    } else if (SELECT_TYPE_OPERATIONS_RD_S2.read() == 0b0001) {
        if (SLT_RD_S2.read()) {
            if (op1_se_s2.read()[31] == 1 && op2_se.read()[31] == 0) {
                exe_res_se_s2.write(1);
            } else if (op1_se_s2.read()[31] == 0 && op2_se.read()[31] == 1) {
                exe_res_se_s2.write(0);
            } else if (op1_se_s2.read() == op2_se.read()) {
                exe_res_se_s2.write(0);
            } else {
                exe_res_se_s2.write((bool)alu_out_se.read()[31]);
            }
        } else if (SLTU_RD_S2.read()) {
            if (op1_se_s2.read()[31] == 1 && op2_se.read()[31] == 0) {
                exe_res_se_s2.write(0);
            } else if (op1_se_s2.read()[31] == 0 && op2_se.read()[31] == 1) {
                exe_res_se_s2.write(1);
            } else if (op1_se_s2.read() == op2_se.read()) {
                exe_res_se_s2.write(0);
            } else {
                exe_res_se_s2.write((bool)alu_out_se.read()[31]);
            }
        } else {
            if (MEM_LOAD_RD_S2.read() || MEM_STORE_RD_S2.read()) {
                // MEM_SIZE = 0 -> Word
                // MEM_SIZE = 1 -> Half word
                // MEM_SIZE = 2 -> byte
                if (((alu_out_se.read() & 0b11) != 0 && MEM_SIZE_RD_S2.read() == 0) ||
                    ((alu_out_se.read() & 0b1) != 0 &&
                     MEM_SIZE_RD_S2.read() == 1)) {  // if adress isn't aligned it creates an exception
                                                     // loading bytes on byte-aligned adresses is legal
                    if (MEM_LOAD_RD_S2)
                        load_adress_missaligned_se = 1;
                    else
                        store_adress_missaligned_se = 1;
                } else {
                    load_adress_missaligned_se.write(0);
                    store_adress_missaligned_se.write(0);
                }
                if ((CURRENT_MODE_SM_S2.read() == 0))  // If in User Mode
                {
                    if (alu_out_se.read() > start_kernel_adress) {
                        if (MEM_LOAD_RD_S2)
                            load_access_fault_se = 1;
                        else
                            store_access_fault_se = 1;
                    } else {
                        load_access_fault_se  = 0;
                        store_access_fault_se = 0;
                    }
                } else {
                    load_access_fault_se  = 0;
                    store_access_fault_se = 0;
                }
            } else {
                load_adress_missaligned_se.write(0);
                load_access_fault_se.write(0);
                store_adress_missaligned_se = 0;
                store_access_fault_se       = 0;
            }
            exe_res_se_s2.write(alu_out_se);
        }
    }
}

void exec_s2::fifo_concat() {
    sc_bv<exe2mem_size> ff_din;
    if (EXCEPTION_SM.read() == 0) {
        ff_din.range(31, 0)    = exe_res_se_s2.read();
        ff_din.range(63, 32)   = bp_mem_data_sd.read();
        ff_din.range(69, 64)   = DEST_RD_S2.read();
        ff_din.range(71, 70)   = MEM_SIZE_RD_S2.read();
        ff_din[72]             = wb_re.read();
        ff_din[73]             = mem_load_re.read();
        ff_din[74]             = mem_store_re.read();
        ff_din[75]             = MEM_SIGN_EXTEND_RD_S2.read();
        ff_din.range(107, 76)  = PC_DEC2EXE_RD_S2.read();
        ff_din[108]            = CSR_WENABLE_RD_S2.read();
        ff_din.range(120, 109) = CSR_WADR_RD_S2.read();
        ff_din.range(152, 121) = CSR_RDATA_RD_S2.read();  // sending Csr read to wb to register
        ff_din[153]            = ENV_CALL_S_MODE_RD_S2.read();
        ff_din[154]            = ENV_CALL_WRONG_MODE_RD_S2.read();
        ff_din[155]            = ILLEGAL_INSTRUCTION_RD_S2.read();
        ff_din[156]            = ADRESS_MISSALIGNED_RD_S2.read();
        ff_din[157]            = ENV_CALL_U_MODE_RD_S2.read();
        ff_din[158]            = ENV_CALL_M_MODE_RD_S2.read();
        ff_din[159]            = exception_se.read();
        ff_din[160]            = load_adress_missaligned_se.read();
        ff_din[161]            = load_access_fault_se.read();
        ff_din[162]            = MRET_RD_S2.read();
        ff_din[163]            = store_adress_missaligned_se;
        ff_din[164]            = store_access_fault_se;
        ff_din[165]            = INSTRUCTION_ACCESS_FAULT_RD_S2;
        ff_din[166]            = EBREAK_RD_S2;
        ff_din[167]            = MULT_INST_RD_S2.read();
        ff_din.range(168, 199) = PC_BRANCH_VALUE_RD_S2.read();

    } else {
        ff_din.range(31, 0)    = 0;
        ff_din.range(63, 32)   = 0;
        ff_din.range(69, 64)   = 0;
        ff_din.range(71, 70)   = 0;
        ff_din[72]             = 1;
        ff_din[73]             = 0;
        ff_din[74]             = 0;
        ff_din[75]             = 0;
        ff_din.range(107, 76)  = PC_DEC2EXE_RD_S2.read();
        ff_din[108]            = 0;
        ff_din.range(120, 109) = 0;
        ff_din.range(152, 121) = 0;  // sending Csr read to wb to register
        ff_din[153]            = 0;
        ff_din[154]            = 0;
        ff_din[155]            = 0;
        ff_din[156]            = 0;
        ff_din[157]            = 0;
        ff_din[158]            = 0;
        ff_din[159]            = 0;
        ff_din[160]            = 0;
        ff_din[161]            = 0;
        ff_din[162]            = 0;
        ff_din[163]            = 0;
        ff_din[164]            = 0;
        ff_din[165]            = 0;
        ff_din[166]            = 0;
        ff_din[167]            = 0;
        ff_din.range(168, 199) = 0;
    }

    exe2mem_din_se_s2.write(ff_din);
}
void exec_s2::fifo_unconcat() {
    sc_bv<exe2mem_size> ff_dout = exe2mem_dout_se_s2.read();
    EXE_RES_RE_S2.write((sc_bv_base)ff_dout.range(31, 0));
    MEM_DATA_RE_S2.write((sc_bv_base)ff_dout.range(63, 32));
    DEST_RE_S2.write((sc_bv_base)ff_dout.range(69, 64));
    MEM_SIZE_RE_S2.write((sc_bv_base)ff_dout.range(71, 70));
    WB_RE_S2.write((bool)ff_dout[72]);
    MEM_LOAD_RE_S2.write((bool)ff_dout[73]);
    MEM_STORE_RE_S2.write((bool)ff_dout[74]);
    MEM_SIGN_EXTEND_RE_S2.write((bool)ff_dout[75]);
    PC_EXE2MEM_RE_S2.write((sc_bv_base)ff_dout.range(107, 76));
    CSR_WENABLE_RE_S2.write((bool)ff_dout[108]);
    CSR_WADR_RE.write((sc_bv_base)ff_dout.range(120, 109));
    CSR_RDATA_RE_S2.write((sc_bv_base)ff_dout.range(152, 121));
    ENV_CALL_S_MODE_RE_S2.write((bool)ff_dout[153]);
    ENV_CALL_WRONG_MODE_RE_S2.write((bool)ff_dout[154]);
    ILLEGAL_INSTRUCTION_RE_S2.write((bool)ff_dout[155]);
    INSTRUCTION_ADRESS_MISSALIGNED_RE_S2.write((bool)ff_dout[156]);
    ENV_CALL_U_MODE_RE_S2.write((bool)ff_dout[157]);
    ENV_CALL_M_MODE_RE_S2.write((bool)ff_dout[158]);
    EXCEPTION_RE_S2.write((bool)ff_dout[159]);
    LOAD_ADRESS_MISSALIGNED_RE_S2.write((bool)ff_dout[160]);
    LOAD_ACCESS_FAULT_RE_S2.write((bool)ff_dout[161]);
    MRET_RE_S2.write((bool)ff_dout[162]);
    STORE_ADRESS_MISSALIGNED_RE_S2.write((bool)ff_dout[163]);
    STORE_ACCESS_FAULT_RE_S2.write((bool)ff_dout[164]);
    INSTRUCTION_ACCESS_FAULT_RE_S2.write((bool)ff_dout[165]);
    EBREAK_RE_S2.write((bool)ff_dout[166]);
    MULT_INST_RE_S2.write((bool)ff_dout[167]);
    PC_BRANCH_VALUE_RE_S2.write((sc_bv_base)ff_dout.range(168, 199));
}

void exec_s2::manage_fifo() {
    bool stall = exe2mem_full_se.read() || DEC2EXE_EMPTY_SD_S2.read() || blocked.read() || !r1_valid_se || !r2_valid_se;
    if (stall) {
        exe2mem_push_se.write(false);
        DEC2EXE_POP_SE_S2.write(false);
    } else {
        exe2mem_push_se.write(true);
        DEC2EXE_POP_SE_S2.write(true);
    }
    stall_se = stall;
}

void exec_s2::bypasses() {
    bool        blocked_var     = false;
    sc_uint<32> bp_mem_data_var = MEM_DATA_RD_S2.read();

    if (RADR1_RD_S2.read() == 0 || BLOCK_BP_RD_S2.read()) {
        op1_se_s2.write(OP1_RD_S2.read());
        r1_valid_se = true;
    } else if (DEST_RE_S2.read() == RADR1_RD_S2.read() && CSR_WENABLE_RE_S2) {
        op1_se_s2.write(CSR_RDATA_RE_S2.read());
        r1_valid_se = true;
    } else if (DEST_RE_S2.read() == RADR1_RD_S2.read() && !MEM_LOAD_RE_S2) {
        op1_se_s2.write(EXE_RES_RE_S2.read());
        r1_valid_se = !MULT_INST_RE_S2 || EXE2MEM_EMPTY_SE_S2;
    } else if (MEM_DEST_RM_S2.read() == RADR1_RD_S2.read() && CSR_WENABLE_RM_S2) {
        op1_se_s2.write(CSR_RDATA_RM_S2.read());
        r1_valid_se = true;
    } else if (MEM_DEST_RM_S2.read() == RADR1_RD_S2.read()) {
        op1_se_s2.write(MEM_RES_RM_S2.read());
        r1_valid_se = !MULT_INST_RM_S2 || BP_MEM2WBK_EMPTY_SM_S2;
    } else if (DEST_RE_S2.read() == RADR1_RD_S2.read() && MEM_LOAD_RE_S2 && !EXE2MEM_EMPTY_SE_S2) {
        blocked_var = true;
        r1_valid_se = true;
    } else {
        op1_se_s2.write(OP1_RD_S2.read());
        r1_valid_se = true;
    }

    if (RADR2_RD_S2.read() == 0 || MEM_LOAD_RD_S2.read() || BLOCK_BP_RD_S2.read()) {
        op2_se.write(OP2_RD_S2.read());
        r2_valid_se = true;
    } else if (DEST_RE_S2.read() == RADR2_RD_S2.read() && !MEM_LOAD_RE_S2) {
        sc_uint<32> bp_value;
        if (CSR_WENABLE_RE_S2)
            bp_value = CSR_RDATA_RE_S2;
        else
            bp_value = EXE_RES_RE_S2;
        if (MEM_STORE_RD_S2.read()) {  // on stores we need to bypass to the data not adr
            bp_mem_data_var = bp_value;
            op2_se.write(OP2_RD_S2.read());
            r2_valid_se = true;
        } else {
            op2_se.write(bp_value);
            r2_valid_se = !MULT_INST_RE_S2 || EXE2MEM_EMPTY_SE_S2;
        }
    } else if (MEM_DEST_RM_S2.read() == RADR2_RD_S2.read()) {
        sc_uint<32> bp_value;
        if (CSR_WENABLE_RM_S2)
            bp_value = CSR_RDATA_RM_S2;
        else
            bp_value = MEM_RES_RM_S2;
        if (MEM_STORE_RD_S2.read()) {
            bp_mem_data_var = MEM_RES_RM_S2.read();
            op2_se.write(OP2_RD_S2.read());
            r2_valid_se = true;
        } else {
            op2_se.write(MEM_RES_RM_S2.read());
            r2_valid_se = !MULT_INST_RM_S2 || BP_MEM2WBK_EMPTY_SM_S2;
        }
    } else if (DEST_RE_S2.read() == RADR2_RD_S2.read() && MEM_LOAD_RE_S2 && !EXE2MEM_EMPTY_SE_S2) {
        blocked_var = true;
        r2_valid_se = true;
    } else {
        op2_se.write(OP2_RD_S2.read());
        r2_valid_se = true;
    }
    bp_mem_data_sd.write(bp_mem_data_var);
    blocked.write(blocked_var);
}

void exec_s2::exception() {
    exception_se = EXCEPTION_RD_S2 | load_adress_missaligned_se | load_access_fault_se | store_access_fault_se |
                   store_adress_missaligned_se;

    if (MACHINE_SOFTWARE_INTERRUPT_SX | MACHINE_TIMER_INTERRUPT_SX | MACHINE_EXTERNAL_INTERRUPT_SX)
    // in case of interrupt or exception have to inform other stage
    {
        INTERRUPTION_SE.write(1);
    } else {
        INTERRUPTION_SE.write(0);
    }

    if (exception_se.read()) {
        wb_re.write(0);
        mem_load_re.write(0);
        mem_store_re.write(0);
    } else {
        wb_re.write(WB_RD_S2.read());
        mem_load_re.write(MEM_LOAD_RD_S2.read());
        mem_store_re.write(MEM_STORE_RD_S2.read());
    }
}

void exec_s2::trace(sc_trace_file* tf) {
    sc_trace(tf, OP1_RD_S2, GET_NAME(OP1_RD_S2));  // can contains CSR if CSR_type_operation_RD == 1
    sc_trace(tf, OP2_RD_S2, GET_NAME(OP2_RD_S2));
    sc_trace(tf, OP1_VALID_RD_S2, GET_NAME(OP1_VALID_RD_S2));
    sc_trace(tf, OP2_VALID_RD_S2, GET_NAME(OP2_VALID_RD_S2));
    sc_trace(tf, RADR1_RD_S2, GET_NAME(RADR1_RD_S2));
    sc_trace(tf, RADR2_RD_S2, GET_NAME(RADR2_RD_S2));

    sc_trace(tf, PC_DEC2EXE_RD_S2, GET_NAME(PC_DEC2EXE_RD_S2));
    sc_trace(tf, MEM_DATA_RD_S2, GET_NAME(MEM_DATA_RD_S2));
    sc_trace(tf, DEST_RD_S2, GET_NAME(DEST_RD_S2));
    sc_trace(tf, CMD_RD_S2, GET_NAME(CMD_RD_S2));
    sc_trace(tf, MEM_SIZE_RD_S2, GET_NAME(MEM_SIZE_RD_S2));
    sc_trace(tf, NEG_OP2_RD_S2, GET_NAME(NEG_OP2_RD_S2));
    sc_trace(tf, WB_RD_S2, GET_NAME(WB_RD_S2));
    sc_trace(tf, MEM_SIGN_EXTEND_RD_S2, GET_NAME(MEM_SIGN_EXTEND_RD_S2));
    sc_trace(tf, SELECT_TYPE_OPERATIONS_RD_S2, GET_NAME(SELECT_TYPE_OPERATIONS_RD_S2));
    sc_trace(tf, MEM_LOAD_RD_S2, GET_NAME(MEM_LOAD_RD_S2));
    sc_trace(tf, MEM_STORE_RD_S2, GET_NAME(MEM_STORE_RD_S2));
    sc_trace(tf, EXE2MEM_POP_SM_S2, GET_NAME(EXE2MEM_POP_SM_S2));
    sc_trace(tf, DEC2EXE_EMPTY_SD_S2, GET_NAME(DEC2EXE_EMPTY_SD_S2));
    sc_trace(tf, SLT_RD_S2, GET_NAME(SLT_RD_S2));
    sc_trace(tf, SLTU_RD_S2, GET_NAME(SLTU_RD_S2));
    sc_trace(tf, CSR_WENABLE_RD_S2, GET_NAME(CSR_WENABLE_RD_S2));
    sc_trace(tf, CSR_WADR_RD_S2, GET_NAME(CSR_WADR_RD_S2));
    sc_trace(tf, CSR_RDATA_RD_S2, GET_NAME(CSR_RDATA_RD_S2));
    sc_trace(tf, EXCEPTION_RD_S2, GET_NAME(EXCEPTION_RD_S2));  // tells if an instruction have been made in DEC

    sc_trace(tf, ENV_CALL_S_MODE_RD_S2, GET_NAME(ENV_CALL_S_MODE_RD_S2));
    sc_trace(tf, ENV_CALL_WRONG_MODE_RD_S2, GET_NAME(ENV_CALL_WRONG_MODE_RD_S2));
    sc_trace(tf, ILLEGAL_INSTRUCTION_RD_S2, GET_NAME(ILLEGAL_INSTRUCTION_RD_S2));  // accessing stuff in wrong mode
    sc_trace(tf, ADRESS_MISSALIGNED_RD_S2, GET_NAME(ADRESS_MISSALIGNED_RD_S2));    // branch offset is misaligned
    sc_trace(tf, ENV_CALL_U_MODE_RD_S2, GET_NAME(ENV_CALL_U_MODE_RD_S2));
    sc_trace(tf, ENV_CALL_M_MODE_RD_S2, GET_NAME(ENV_CALL_M_MODE_RD_S2));

    sc_trace(tf, EXCEPTION_RE_S2, GET_NAME(EXCEPTION_RE_S2));
    sc_trace(
        tf, LOAD_ADRESS_MISSALIGNED_RE_S2, GET_NAME(LOAD_ADRESS_MISSALIGNED_RE_S2));  // adress from store/load isn't aligned
    sc_trace(tf, LOAD_ACCESS_FAULT_RE_S2,
             GET_NAME(LOAD_ACCESS_FAULT_RE_S2));  // trying to access memory in wrong mode
    sc_trace(tf, ENV_CALL_U_MODE_RE_S2, GET_NAME(ENV_CALL_U_MODE_RE_S2));
    sc_trace(tf, ENV_CALL_WRONG_MODE_RE_S2, GET_NAME(ENV_CALL_WRONG_MODE_RE_S2));
    sc_trace(tf, ILLEGAL_INSTRUCTION_RE_S2, GET_NAME(ILLEGAL_INSTRUCTION_RE_S2));  // accessing stuff in wrong mode
    sc_trace(tf,
             INSTRUCTION_ADRESS_MISSALIGNED_RE_S2,
             GET_NAME(INSTRUCTION_ADRESS_MISSALIGNED_RE_S2));  // branch offset is misaligned
    sc_trace(tf, ENV_CALL_S_MODE_RE_S2, GET_NAME(ENV_CALL_S_MODE_RE_S2));
    sc_trace(tf, ENV_CALL_M_MODE_RE_S2, GET_NAME(ENV_CALL_M_MODE_RE_S2));
    sc_trace(tf, PC_BRANCH_VALUE_RD_S2, GET_NAME(PC_BRANCH_VALUE_RD_S2));
    sc_trace(tf, PC_BRANCH_VALUE_RE_S2, GET_NAME(PC_BRANCH_VALUE_RE_S2));

    // Interruption :

    sc_trace(tf, INTERRUPTION_SE, GET_NAME(INTERRUPTION_SE));
    sc_trace(tf, MACHINE_SOFTWARE_INTERRUPT_SX, GET_NAME(MACHINE_SOFTWARE_INTERRUPT_SX));
    sc_trace(tf, MACHINE_TIMER_INTERRUPT_SX, GET_NAME(MACHINE_TIMER_INTERRUPT_SX));
    sc_trace(tf, MACHINE_EXTERNAL_INTERRUPT_SX, GET_NAME(MACHINE_EXTERNAL_INTERRUPT_SX));
    sc_trace(tf, MACHINE_SOFTWARE_INTERRUPT_SE_S2, GET_NAME(MACHINE_SOFTWARE_INTERRUPT_SE_S2));
    sc_trace(tf, MACHINE_TIMER_INTERRUPT_SE, GET_NAME(MACHINE_TIMER_INTERRUPT_SE));
    sc_trace(tf, MACHINE_EXTERNAL_INTERRUPT_SE, GET_NAME(MACHINE_EXTERNAL_INTERRUPT_SE));

    // bypasses

    sc_trace(tf, MEM_DEST_RM_S2, GET_NAME(MEM_DEST_RM_S2));
    sc_trace(tf, MEM_RES_RM_S2, GET_NAME(MEM_RES_RM_S2));
    sc_trace(tf, CSR_WENABLE_RM_S2, GET_NAME(CSR_WENABLE_RM_S2));
    sc_trace(tf, CSR_RDATA_RM_S2, GET_NAME(CSR_RDATA_RM_S2));

    // Genral Interface :

    sc_trace(tf, EXCEPTION_SM, GET_NAME(EXCEPTION_SM));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET, GET_NAME(RESET));

    // Fifo exe2mem interface :

    sc_trace(tf, EXE_RES_RE_S2, GET_NAME(EXE_RES_RE_S2));
    sc_trace(tf, MEM_DATA_RE_S2, GET_NAME(MEM_DATA_RE_S2));
    sc_trace(tf, DEST_RE_S2, GET_NAME(DEST_RE_S2));
    sc_trace(tf, MEM_SIZE_RE_S2, GET_NAME(MEM_SIZE_RE_S2));
    sc_trace(tf, PC_EXE2MEM_RE_S2, GET_NAME(PC_EXE2MEM_RE_S2));

    sc_trace(tf, WB_RE_S2, GET_NAME(WB_RE_S2));
    sc_trace(tf, MEM_SIGN_EXTEND_RE_S2, GET_NAME(MEM_SIGN_EXTEND_RE_S2));
    sc_trace(tf, MEM_LOAD_RE_S2, GET_NAME(MEM_LOAD_RE_S2));
    sc_trace(tf, MEM_STORE_RE_S2, GET_NAME(MEM_STORE_RE_S2));
    sc_trace(tf, EXE2MEM_EMPTY_SE_S2, GET_NAME(EXE2MEM_EMPTY_SE_S2));
    sc_trace(tf, DEC2EXE_POP_SE_S2, GET_NAME(DEC2EXE_POP_SE_S2));

    sc_trace(tf, CSR_WENABLE_RE_S2, GET_NAME(CSR_WENABLE_RE_S2));
    sc_trace(tf, CSR_WADR_RE, GET_NAME(CSR_WADR_RE));
    sc_trace(tf, CSR_RDATA_RE_S2, GET_NAME(CSR_RDATA_RE_S2));

    // Internals signals :

    sc_trace(tf, exe_res_se_s2, GET_NAME(exe_res_se_s2));

    sc_trace(tf, exe2mem_din_se_s2, GET_NAME(exe2mem_din_se_s2));  // concatenation of exe_res, mem_data...etc
    sc_trace(tf, exe2mem_dout_se_s2, GET_NAME(exe2mem_dout_se_s2));

    sc_trace(tf, op1_se_s2, GET_NAME(op1_se_s2));
    sc_trace(tf, op2_se, GET_NAME(op2_se));
    sc_trace(tf, alu_in_op2_se, GET_NAME(alu_in_op2_se));
    sc_trace(tf, alu_out_se, GET_NAME(alu_out_se));
    sc_trace(tf, shifter_out_se, GET_NAME(shifter_out_se));
    sc_trace(tf, bp_mem_data_sd, GET_NAME(bp_mem_data_sd));
    sc_trace(tf, shift_val_se, GET_NAME(shift_val_se));
    sc_trace(tf, exe2mem_push_se, GET_NAME(exe2mem_push_se));
    sc_trace(tf, exe2mem_full_se, GET_NAME(exe2mem_full_se));
    sc_trace(tf, blocked, GET_NAME(blocked));

    sc_trace(tf, wb_re, GET_NAME(wb_re));
    sc_trace(tf, mem_load_re, GET_NAME(mem_load_re));
    sc_trace(tf, mem_store_re, GET_NAME(mem_store_re));
    sc_trace(tf, MULT_INST_RE_S2, GET_NAME(MULT_INST_RE_S2));
    // Exception :

    sc_trace(tf, exception_se, GET_NAME(exception_se));
    sc_trace(tf, MRET_RD_S2, GET_NAME(MRET_RD_S2));
    sc_trace(tf, MRET_RE_S2, GET_NAME(MRET_RE_S2));
    sc_trace(tf, store_access_fault_se, GET_NAME(store_access_fault_se));
    sc_trace(tf, store_adress_missaligned_se, GET_NAME(store_adress_missaligned_se));
    sc_trace(tf, INSTRUCTION_ACCESS_FAULT_RE_S2, GET_NAME(INSTRUCTION_ACCESS_FAULT_RE_S2));
    sc_trace(
        tf, load_adress_missaligned_se, GET_NAME(load_adress_missaligned_se));  // adress from store/load isn't aligned
    sc_trace(tf, load_access_fault_se, GET_NAME(load_access_fault_se));

    sc_trace(tf, stall_se, GET_NAME(stall_se));
    sc_trace(tf, r1_valid_se, GET_NAME(r1_valid_se));
    sc_trace(tf, r2_valid_se, GET_NAME(r2_valid_se));
    sc_trace(tf, MULT_INST_RM_S2, GET_NAME(MULT_INST_RM_S2));

    sc_trace(tf, OP1_SE_S2, GET_NAME(OP1_SE_S2));
    sc_trace(tf, OP2_SE_S2, GET_NAME(OP2_SE_S2));

    sc_trace(tf, BLOCK_BP_RD_S2, GET_NAME(BLOCK_BP_RD_S2));

    alu_inst.trace(tf);
    shifter_inst.trace(tf);
    fifo_inst.trace(tf);
}

// 0000010000101100010001100011011
// 00111101000110110101100010111010