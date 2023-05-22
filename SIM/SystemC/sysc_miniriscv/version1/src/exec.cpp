#include "exec.h"
#include "alu.h"
#include "shifter.h"

void exec::preprocess_op() {
    sc_uint<32> op1 = op1_se.read();
    sc_uint<32> op2 = op2_se.read();
    if (NEG_OP2_RD.read()) {
        alu_in_op2_se.write(~op2);
    } else {
        alu_in_op2_se.write(op2);
    }
    shift_val_se.write(op2.range(4, 0));
}

void exec::select_exec_res() {
    sc_uint<32> alu_out     = alu_out_se.read();
    sc_uint<32> shifter_out = shifter_out_se.read();

    if (OPTYPE_RD.read() == 1) {
        exe_res_se.write(shifter_out_se);
    } else if (OPTYPE_RD.read() == 0) {
        exe_res_se.write(alu_out_se);
    } else if (OPTYPE_RD.read() == 2) {
        if (CMD_RD.read() == 0) {
            if (op1_se.read()[31] == 1 && op2_se.read()[31] == 0) {
                exe_res_se.write(1);
            } else if (op1_se.read()[31] == 0 && op2_se.read()[31] == 1) {
                exe_res_se.write(0);
            } else if (op1_se.read() == op2_se.read()) {
                exe_res_se.write(0);
            } else {
                exe_res_se.write((bool)alu_out_se.read()[31]);
            }
        } else if (CMD_RD.read() == 1) {
            if (op1_se.read()[31] == 1 && op2_se.read()[31] == 0) {
                exe_res_se.write(0);
            } else if (op1_se.read()[31] == 0 && op2_se.read()[31] == 1) {
                exe_res_se.write(1);
            } else if (op1_se.read() == op2_se.read()) {
                exe_res_se.write(0);
            } else {
                exe_res_se.write((bool)alu_out_se.read()[31]);
            }
        }
    }
}

void exec::fifo_concat() {
    sc_bv<EXE2MEM_SIZE> ff_din;
    ff_din.range(31, 0)  = exe_res_se.read();
    ff_din.range(63, 32) = mem_data_se.read();
    ff_din.range(69, 64) = DEST_RD.read();
    ff_din.range(71, 70) = MEM_SIZE_RD.read();
    ff_din[72]           = WB_RD.read();
    ff_din[73]           = MEM_LOAD_RD.read();
    ff_din[74]           = MEM_STORE_RD.read();
    ff_din[75]           = MEM_SIGN_EXTEND_RD.read();

    exe2mem_din_se.write(ff_din);
}
void exec::fifo_unconcat() {
    sc_bv<EXE2MEM_SIZE> ff_dout = exe2mem_dout_se.read();
    EXE_RES_RE.write((sc_bv_base)ff_dout.range(31, 0));
    MEM_DATA_RE.write((sc_bv_base)ff_dout.range(63, 32));
    DEST_RE.write((sc_bv_base)ff_dout.range(69, 64));
    MEM_SIZE_RE.write((sc_bv_base)ff_dout.range(71, 70));
    WB_RE.write((bool)ff_dout[72]);
    MEM_LOAD_RE.write((bool)ff_dout[73]);
    MEM_STORE_RE.write((bool)ff_dout[74]);
    MEM_SIGN_EXTEND_RE.write((bool)ff_dout[75]);
}

void exec::manage_fifo() {
    bool stall = exe2mem_full_se || DEC2EXE_EMPTY_SD || invalid_operand_se;
    if (stall) {
        exe2mem_push_se = false;
        DEC2EXE_POP_SE  = false;
    } else {
        exe2mem_push_se = true;
        DEC2EXE_POP_SE  = true;
    }
}

void exec::bypasses() {
    // **********Bypasses for source registers 1*********
    if (RADR1_RD.read() == 0 || BLOCK_BP_RD) {
        // We ignore the regiter r0, which us always 0
        invalid_operand_se = false;
        op1_se             = OP1_RD;
    } else if (RADR1_RD == DEST_RE && MEM_LOAD_RE && !EXE2MEM_EMPTY_SE) {
        /*
         If instruction is currently in MEM, the result is not ready, and we mark it as such.
         We need to check EXE2MEM_EMPTY, because if that fifo is empty, it means there is no
         instruction in MEM, and the values sent by the bypass are garbage.
         We also check that it is a memory instruction to ensure the result is not ready
        */
        invalid_operand_se = true;
    } else if (RADR1_RD == DEST_RE && !EXE2MEM_EMPTY_SE) {
        /*
         EXE -> EXE Bypass
         If the result of EXE will be written in the register we need, we can use the bypass
         to get the value directly, saving clock cycles.
         Once again, we need to check the FIFO is not empty to ensure the value is valid.
         */
        op1_se             = EXE_RES_RE;
        invalid_operand_se = false;
    } else if (RADR1_RD == DEST_RM && !MEM2WBK_EMPTY_SM) {
        /*
         MEM -> EXE Bypass
         Same bypass, but in MEM. Bypass in EXE has priority over bypass in MEM, because the
         value is more recent.
        */
        op1_se             = MEM_RES_RM;
        invalid_operand_se = false;
    } else {
        /*
         If none of these conditions is true, we just use the value from registers.
        */
        invalid_operand_se = false;
        op1_se             = OP1_RD;
    }

    // **********Bypasses for source registers 2 (it's almost the same thing)*********
    if (RADR2_RD.read() == 0 || BLOCK_BP_RD) {
        // We ignore the regiter r0, which us always 0
        invalid_operand_se = false;
        op2_se             = OP2_RD;
        mem_data_se        = MEM_DATA_RD;
    } else if (RADR2_RD == DEST_RE && MEM_LOAD_RE && !EXE2MEM_EMPTY_SE) {
        /*
         If instruction is currently in MEM, the result is not ready, and we mark it as such.
         We need to check EXE2MEM_EMPTY, because if that fifo is empty, it means there is no
         instruction in MEM, and the values sent by the bypass are garbage.
         We also check that it is a memory instruction to ensure the result is not ready
        */
        invalid_operand_se = true;
        mem_data_se        = MEM_DATA_RD;
    } else if (RADR2_RD == DEST_RE && !EXE2MEM_EMPTY_SE) {
        /*
         EXE -> EXE Bypass
         If the result of EXE will be written in the register we need, we can use the bypass
         to get the value directly, saving clock cycles.
         Once again, we need to check the FIFO is not empty to ensure the value is valid.

         For memory stores, we need to bypass rs2 to the mem_data signal instead, as it is where the
         value is used, and not op2.
         */
        if (MEM_STORE_RD) {
            mem_data_se = EXE_RES_RE;
            op2_se      = OP2_RD;
        } else {
            op2_se      = EXE_RES_RE;
            mem_data_se = MEM_DATA_RD;
        }
        invalid_operand_se = false;
    } else if (RADR2_RD == DEST_RM && !MEM2WBK_EMPTY_SM) {
        /*
         MEM -> EXE Bypass
         Same bypass, but in MEM. Bypass in EXE has priority over bypass in MEM, because the
         value is more recent.
        */
        if (MEM_STORE_RD) {
            mem_data_se = MEM_RES_RM;
            op2_se      = OP2_RD;
        } else {
            op2_se      = MEM_RES_RM;
            mem_data_se = MEM_DATA_RD;
        }
        invalid_operand_se = false;
    } else {
        /*
         If none of these conditions is true, we just use the value from registers.
        */
        invalid_operand_se = false;
        op2_se             = OP2_RD;
        mem_data_se        = MEM_DATA_RD;
    }
}

void exec::pred_branch_signals(){
    int cmd = 0b00;
    if (PRED_BRANCH_MISS_RD.read() && IS_BRANCH_RD.read()) {
        cmd = 0b01;
    } else if (PRED_BRANCH_MISS_RD.read() && !IS_BRANCH_RD.read()) {
        cmd = 0b00;
    } else if (!PRED_BRANCH_MISS_RD.read() && IS_BRANCH_RD.read()) {
        cmd = 0b10;
    }
    PRED_BRANCH_CMD_OUT_SE.write(cmd);

    if (BRANCH_TAKEN_RD.read() && cmd == 0b01) {
        PRED_BRANCH_CPT_OUT_SE.write(0b10);
        PRED_BRANCH_WRITE_ADR_OUT_SE.write(PRED_BRANCH_ADR_RD.read());
        PRED_BRANCH_TARGET_OUT_SE.write(PRED_BRANCH_TARGET_ADR_RD.read());
        PRED_BRANCH_LRU_OUT_SE.write(PRED_BRANCH_LRU_RD.read());
        PRED_BRANCH_PNT_OUT_SE.write(PRED_BRANCH_PNT_RD.read());
    } else if (!BRANCH_TAKEN_RD.read() && cmd == 0b01) {
        PRED_BRANCH_CPT_OUT_SE.write(0b01);
        PRED_BRANCH_WRITE_ADR_OUT_SE.write(PRED_BRANCH_ADR_RD.read());
        PRED_BRANCH_TARGET_OUT_SE.write(PRED_BRANCH_TARGET_ADR_RD.read());
        PRED_BRANCH_LRU_OUT_SE.write(PRED_BRANCH_LRU_RD.read());
        PRED_BRANCH_PNT_OUT_SE.write(PRED_BRANCH_PNT_RD.read());
    } else if (cmd == 0b10) {
        int counter = PRED_BRANCH_CPT_RD.read();
        if (counter != 0b00 && !BRANCH_TAKEN_RD.read())
        {
            counter -= 1;
        }
        else if(counter != 0b11 && BRANCH_TAKEN_RD.read())
        {
            counter += 1;
        }
        PRED_BRANCH_CPT_OUT_SE.write(counter);
        PRED_BRANCH_WRITE_ADR_OUT_SE.write(0x00000000);
        PRED_BRANCH_TARGET_OUT_SE.write(0x00000000);
        PRED_BRANCH_LRU_OUT_SE.write(PRED_BRANCH_LRU_RD.read());
        PRED_BRANCH_PNT_OUT_SE.write(PRED_BRANCH_PNT_RD.read());
    }


}

void exec::trace(sc_trace_file* tf) {
    sc_trace(tf, OP1_RD, GET_NAME(OP1_RD));  // can contains CSR if CSR_type_operation_RD == 1
    sc_trace(tf, OP2_RD, GET_NAME(OP2_RD));
    sc_trace(tf, RADR1_RD, GET_NAME(RADR1_RD));
    sc_trace(tf, RADR2_RD, GET_NAME(RADR2_RD));

    sc_trace(tf, MEM_DATA_RD, GET_NAME(MEM_DATA_RD));
    sc_trace(tf, DEST_RD, GET_NAME(DEST_RD));
    sc_trace(tf, CMD_RD, GET_NAME(CMD_RD));
    sc_trace(tf, MEM_SIZE_RD, GET_NAME(MEM_SIZE_RD));
    sc_trace(tf, NEG_OP2_RD, GET_NAME(NEG_OP2_RD));
    sc_trace(tf, WB_RD, GET_NAME(WB_RD));
    sc_trace(tf, MEM_SIGN_EXTEND_RD, GET_NAME(MEM_SIGN_EXTEND_RD));
    sc_trace(tf, OPTYPE_RD, GET_NAME(OPTYPE_RD));
    sc_trace(tf, MEM_LOAD_RD, GET_NAME(MEM_LOAD_RD));
    sc_trace(tf, MEM_STORE_RD, GET_NAME(MEM_STORE_RD));
    sc_trace(tf, EXE2MEM_POP_SM, GET_NAME(EXE2MEM_POP_SM));
    sc_trace(tf, DEC2EXE_EMPTY_SD, GET_NAME(DEC2EXE_EMPTY_SD));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET, GET_NAME(RESET));
    sc_trace(tf, EXE_RES_RE, GET_NAME(EXE_RES_RE));
    sc_trace(tf, MEM_DATA_RE, GET_NAME(MEM_DATA_RE));
    sc_trace(tf, DEST_RE, GET_NAME(DEST_RE));
    sc_trace(tf, MEM_SIZE_RE, GET_NAME(MEM_SIZE_RE));
    sc_trace(tf, WB_RE, GET_NAME(WB_RE));
    sc_trace(tf, MEM_SIGN_EXTEND_RE, GET_NAME(MEM_SIGN_EXTEND_RE));
    sc_trace(tf, MEM_LOAD_RE, GET_NAME(MEM_LOAD_RE));
    sc_trace(tf, MEM_STORE_RE, GET_NAME(MEM_STORE_RE));
    sc_trace(tf, EXE2MEM_EMPTY_SE, GET_NAME(EXE2MEM_EMPTY_SE));
    sc_trace(tf, DEC2EXE_POP_SE, GET_NAME(DEC2EXE_POP_SE));
    sc_trace(tf, exe_res_se, GET_NAME(exe_res_se));
    sc_trace(tf, exe2mem_din_se, GET_NAME(exe2mem_din_se));
    sc_trace(tf, exe2mem_dout_se, GET_NAME(exe2mem_dout_se));
    sc_trace(tf, op1_se, GET_NAME(op1_se));
    sc_trace(tf, op2_se, GET_NAME(op2_se));
    sc_trace(tf, alu_in_op2_se, GET_NAME(alu_in_op2_se));
    sc_trace(tf, alu_out_se, GET_NAME(alu_out_se));
    sc_trace(tf, shifter_out_se, GET_NAME(shifter_out_se));
    sc_trace(tf, mem_data_se, GET_NAME(mem_data_se));
    sc_trace(tf, shift_val_se, GET_NAME(shift_val_se));
    sc_trace(tf, exe2mem_push_se, GET_NAME(exe2mem_push_se));
    sc_trace(tf, exe2mem_full_se, GET_NAME(exe2mem_full_se));
    sc_trace(tf, wb_re, GET_NAME(wb_re));
    sc_trace(tf, mem_load_re, GET_NAME(mem_load_re));
    sc_trace(tf, mem_store_re, GET_NAME(mem_store_re));
    sc_trace(tf, BLOCK_BP_RD, GET_NAME(BLOCK_BP_RD));
    sc_trace(tf, invalid_operand_se, GET_NAME(invalid_operand_se));

    sc_trace(tf, PRED_BRANCH_ADR_RD, GET_NAME(PRED_BRANCH_ADR_RD));
    sc_trace(tf, PRED_BRANCH_MISS_RD, GET_NAME(PRED_BRANCH_MISS_RD));
    sc_trace(tf, PRED_BRANCH_TARGET_ADR_RD, GET_NAME(PRED_BRANCH_TARGET_ADR_RD));
    sc_trace(tf, PRED_BRANCH_CPT_RD, GET_NAME(PRED_BRANCH_CPT_RD));
    sc_trace(tf, PRED_BRANCH_LRU_RD, GET_NAME(PRED_BRANCH_LRU_RD));
    sc_trace(tf, PRED_BRANCH_PNT_RD, GET_NAME(PRED_BRANCH_PNT_RD));
    sc_trace(tf, IS_BRANCH_RD, GET_NAME(IS_BRANCH_RD));
    sc_trace(tf, BRANCH_TAKEN_RD, GET_NAME(BRANCH_TAKEN_RD));

    sc_trace(tf, PRED_BRANCH_CMD_OUT_SE, GET_NAME(PRED_BRANCH_CMD_OUT_SE));
    sc_trace(tf, PRED_BRANCH_WRITE_ADR_OUT_SE, GET_NAME(PRED_BRANCH_WRITE_ADR_OUT_SE));
    sc_trace(tf, PRED_BRANCH_TARGET_OUT_SE, GET_NAME(PRED_BRANCH_TARGET_OUT_SE));
    sc_trace(tf, PRED_BRANCH_CPT_OUT_SE, GET_NAME(PRED_BRANCH_CPT_OUT_SE));
    sc_trace(tf, PRED_BRANCH_LRU_OUT_SE, GET_NAME(PRED_BRANCH_LRU_OUT_SE));
    sc_trace(tf, PRED_BRANCH_PNT_OUT_SE, GET_NAME(PRED_BRANCH_PNT_OUT_SE));


    alu_inst.trace(tf);
    shifter_inst.trace(tf);
    fifo_inst.trace(tf);
}