#include "CSR/csr.h"
#include "DEC/dec.h"
#include "EXE/exec.h"
#include "IFETCH/ifetch.h"
#include "MEM/mem.h"
#include "REG/reg.h"
#include "PRED_BRANCH_CACHE/pred_branch_cache.h"
#include "TIMER/timer.h"
#include "WBK/wbk.h"
#include "systemc.h"

//Multipliers
#include "EXE/x0_multiplier.h"
#include "MEM/x1_multiplier.h"
#include "WBK/x2_multiplier.h"

//Divider
#include "EXE/Diviseur.h"


SC_MODULE(core) {
    // Global Interface :

    sc_in_clk   CLK;
    sc_in<bool> RESET;

    // IFETCH-DEC interface
    // DEC2IF :

    sc_signal<bool>        DEC2IF_EMPTY_SI;
    sc_signal<bool>        DEC2IF_POP_SI;
    sc_signal<sc_uint<32>> PC_RD;

    // IF2DEC :

    sc_signal<bool>             IF2DEC_FLUSH_SD;                // IF2DEC fifo flush
    sc_signal<bool>             IF2DEC_POP_SD;                  // IF2DEC fifo pop
    sc_signal<bool>             IF2DEC_EMPTY_SI;                // IF2DEC fifo empty
    sc_signal<sc_bv<32>>        INSTR_RI;                       // instruction get from fifo IF2DEC
    sc_signal<sc_uint<32>>      PC_IF2DEC_RI;                   // pc get from fifo IF2DEC
    sc_signal<bool>             EXCEPTION_RI;                   // ?i dont understand  tells if an instruction have been made in IFETCH
    sc_signal<sc_uint<32>>      PRED_BRANCH_ADR_RI;             // branch instruction address
    sc_signal<sc_unint<32>>     PRED_BRANCH_TARGET_ADR_RI;      // branch target address
    sc_signal<bool>             PRED_BRANCH_MISS_OUT_SI;        // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_signal<sc_uint<2>>       PRED_BRANCH_CPT_OUT_SI;         // branch taken times
    sc_signal<bool>             PRED_BRANCH_LRU_OUT_SI;         // Less Recent Use
    sc_signal<sc_uint<2>>       PRED_BRANCH_PNT_OUT_SI;         // branch taken target address

    // IF-PRED_BRANCH_CACHE interface

    sc_signal<sc_uint<32>>      PRED_BRANCH_CHECK_ADR_IN_SI;         // Branch instruction address need to be check in the table if it exist or not

    // PRED_BRANCH_CACHE-IF interface

    sc_signal<bool>             PRED_BRANCH_MISS_OUT_SP;              // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_signal<sc_uint<32>>      PRED_BRANCH_TARGET_ADR_OUT_SP;        // Branch target address
    sc_signal<bool>             PRED_BRANCH_LRU_OUT_SP;               // Less Recent Use
    sc_signal<sc_uint<2>>       PRED_BRANCH_CPT_OUT_SP;               // Branch taken times
    sc_signal<sc_uint<2>>       PRED_BRANCH_PNT_OUT_SP;               // Branch taken target address

    // EXE-PRED_BRANCH_CACHE interface

    sc_signal<sc_uint<2>>       PRED_BRANCH_CMD_IN_SE;            // CMD for the skip(not branch instruction)/write/update it's depends on the signal PRED_BRANCH_MISS_OUT_SI skip = 0 write = 1 update = 2
    sc_signal<sc_uint<32>>      PRED_BRANCH_WRITE_ADR_IN_SE;     // Branch instruction address need to be write in the table
    sc_signal<sc_uint<32>>      PRED_BRANCH_TARGET_ADR_IN_SE;    // Branch target address
    sc_signal<sc_uint<2>>       PRED_BRANCH_CPT_IN_SE;            // Branch taken counter
    sc_signal<bool>             PRED_BRANCH_LRU_IN_SE;                  // Less Recent Use
    sc_signal<sc_uint<2>>       PRED_BRANCH_PNT_IN_SE;            // The index where to put the data


    // DEC-EXE interface

    sc_signal<sc_uint<32>> PC_DEC2EXE_RD;
    sc_signal<sc_uint<32>> OP1_RD;
    sc_signal<sc_uint<32>> OP2_RD;
    sc_signal<sc_uint<2>>  EXE_CMD_RD;
    sc_signal<bool>        NEG_OP2_RD;
    sc_signal<bool>        WB_RD;
    sc_signal<sc_uint<4>>  SELECT_TYPE_OPERATIONS_RD;
    sc_signal<sc_uint<32>> PC_BRANCH_VALUE_RD;

    sc_signal<sc_uint<32>> MEM_DATA_RD;
    sc_signal<bool>        MEM_LOAD_RD;
    sc_signal<bool>        MEM_STORE_RD;
    sc_signal<bool>        MEM_SIGN_EXTEND_RD;
    sc_signal<sc_uint<2>>  MEM_SIZE_RD;
    sc_signal<bool>        SLT_RD;
    sc_signal<bool>        SLTU_RD;

    sc_signal<bool> DEC2EXE_POP_SE;
    sc_signal<bool> DEC2EXE_EMPTY_SD;

    sc_signal<bool>       BP_R1_VALID_RD;
    sc_signal<bool>       BP_R2_VALID_RD;
    sc_signal<sc_uint<6>> BP_RADR1_RD;
    sc_signal<sc_uint<6>> BP_RADR2_RD;
    sc_signal<bool>       BLOCK_BP_RD;

    sc_signal<bool>        CSR_WENABLE_RD;
    sc_signal<sc_uint<32>> CSR_RDATA_RD;
    sc_signal<sc_uint<12>> CSR_WADR_RD;
    sc_signal<bool>        INTERRUPTION_SE;
    sc_signal<bool>        EXCEPTION_RD;
    sc_signal<bool>        ECALL_I_RD;
    sc_signal<bool>        EBREAK_RD;
    sc_signal<bool>        EBREAK_RE;
    sc_signal<bool>        ILLEGAL_INSTRUCTION_RD;  // accessing stuff in wrong mode
    sc_signal<bool>        ADRESS_MISSALIGNED_RD;   // branch offset is misaligned
    sc_signal<bool>        ENV_CALL_U_MODE_RD;
    sc_signal<bool>        ENV_CALL_M_MODE_RD;
    sc_signal<sc_uint<32>> KERNEL_ADR_SC;

    // DEC2EXE PRED_BRANCH_CACHE interface
    sc_signal<sc_uint<32>> PRED_BRANCH_ADR_RD;        // branch instruction address
    sc_signal<bool>        PRED_BRANCH_MISS_RD;       // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_signal<sc_uint<2>>  PRED_BRANCH_CPT_RD;        // branch taken times
    sc_signal<bool>        PRED_BRANCH_LRU_RD;        // Less Recent Use
    sc_signal<sc_uint<2>>  PRED_BRANCH_PNT_RD;        // branch taken target address
    sc_signal<bool>        IS_BRANCH_RD;              // branch instruction or not
    sc_signal<bool>        BRANCH_TAKEN_RD;           // branch taken or not


    // DEC-CSR interface
    sc_signal<sc_uint<12>> CSR_RADR_SD;
    sc_signal<bool> CSRRC_I_RD;
    sc_signal<sc_uint<32>> CSR_RDATA_SC;
    sc_signal<sc_uint<32>> MCAUSE_SC;
    // DEC-REG interface
    sc_signal<sc_uint<32>> RDATA1_SR;
    sc_signal<sc_uint<32>> RDATA2_SR;

    sc_signal<sc_uint<6>> RADR1_SD;
    sc_signal<sc_uint<6>> RADR2_SD;

    sc_signal<sc_uint<6>> EXE_DEST_RD;

    sc_signal<sc_uint<32>> READ_PC_SR;
    sc_signal<sc_uint<32>> WRITE_PC_SD;
    sc_signal<bool>        WRITE_PC_ENABLE_SD;

    //EXE-X0 OR EXE-DIVISEUR
    sc_signal<sc_uint<32>> op1_se;
    sc_signal<sc_uint<32>> op2_se;

    //EXE DIVIDER
    sc_signal<bool> START_SE;
    sc_signal<sc_uint<32>> DIVIDER_RES_OUTPUT;
    sc_signal<bool> DIV_BUSY_SE;
    sc_signal<bool> DONE_SE;


    // X0-X1 interface
    sc_signal<sc_bv<320>> multiplier_out_sx0;
    sc_signal<bool>       select_higher_bits_rx0;
    sc_signal<bool>       x02x1_EMPTY_SX0, x02x1_POP_SX1;
    sc_signal<bool>       SIGNED_RES_RX0;
    sc_signal<bool>       SIGNED_RES_RX1;
    // EXE-MEM interface
    sc_signal<sc_uint<32>> PC_BRANCH_VALUE_RE;
    sc_signal<sc_uint<32>> EXE_RES_RE;
    sc_signal<sc_uint<32>> MEM_DATA_RE;
    sc_signal<sc_uint<6>>  DEST_RE;
    sc_signal<sc_uint<2>>  MEM_SIZE_RE;
    sc_signal<sc_uint<32>> PC_EXE2MEM_RE;
    sc_signal<bool>        MEM_WB, MEM_SIGN_EXTEND_RE;
    sc_signal<bool>        MEM_LOAD_RE, MEM_STORE_RE;
    sc_signal<bool>        MEM_MULT_RE, MULT_SEL_HIGH_RE;

    sc_signal<bool> EXE2MEM_EMPTY_SE, EXE2MEM_POP_SM;
    sc_signal<bool> CSR_WENABLE_RE;
    sc_signal<bool> MACHINE_SOFTWARE_INTERRUPT_SX;
    sc_signal<bool> MACHINE_TIMER_INTERRUPT_SX;
    sc_signal<bool> MACHINE_EXTERNAL_INTERRUPT_SX;

    sc_signal<bool>        MACHINE_SOFTWARE_INTERRUPT_SE;
    sc_signal<bool>        MACHINE_TIMER_INTERRUPT_SE;
    sc_signal<bool>        MACHINE_EXTERNAL_INTERRUPT_SE;
    sc_signal<sc_uint<12>> CSR_WADR_RE;
    sc_signal<sc_uint<32>> CSR_RDATA_RE;

    sc_signal<bool> EXCEPTION_RE;
    sc_signal<bool> LOAD_ADRESS_MISSALIGNED_RE;  // adress from store/load isn't aligned
    sc_signal<bool> LOAD_ACCESS_FAULT_RE;        // trying to access memory in wrong mode

    sc_signal<bool> STORE_ADRESS_MISSALIGNED_RE;
    sc_signal<bool> STORE_ACCESS_FAULT_RE;
    sc_signal<bool> ECALL_I_RE;
    sc_signal<bool> EBREAK_I_RE;
    sc_signal<bool> ILLEGAL_INSTRUCTION_RE;             // accessing stuff in wrong mode
    sc_signal<bool> INSTRUCTION_ADRESS_MISSALIGNED_RE;  // branch offset is misaligned
    sc_signal<bool> ENV_CALL_S_MODE_RE;
    sc_signal<bool> ENV_CALL_M_MODE_RE;
    sc_signal<bool> ENV_CALL_S_MODE_RD;
    sc_signal<bool> ENV_CALL_WRONG_MODE_RD;
    sc_signal<bool> MRET_RD;
    sc_signal<bool> ENV_CALL_WRONG_MODE_RE;
    sc_signal<bool> ENV_CALL_U_MODE_RE;
    sc_signal<bool> MRET_RE;
    sc_signal<bool> INSTRUCTION_ACCESS_FAULT_RD;
    sc_signal<bool> INSTRUCTION_ACCESS_FAULT_RE;
    sc_signal<bool> MULT_INST_RD;
    sc_signal<bool> MULT_INST_RE;
    sc_signal<bool> MULT_INST_RM;
    // X1-X2 interface
    sc_signal<sc_bv<128>> multiplier_out_sx1;
    sc_signal<sc_uint<32>>  multiplier_out_sx2;
    sc_signal<bool>       select_higher_bits_rx1;
    sc_signal<bool>       x12x2_EMPTY_SX1, x12x2_POP_SX2;

    // MEM-WBK interface
    sc_signal<sc_uint<32>> MEM_RES_RM;
    sc_signal<sc_uint<6>>  DEST_RM;
    sc_signal<bool>        WB_RM;
    sc_signal<bool>        WBK_MEM_SIGN_EXTEND;
    sc_signal<bool>        MEM2WBK_EMPTY_SM;
    sc_signal<bool>        MEM2WBK_POP_SW;
    sc_signal<bool>        WBK_MEM_LOAD;
    sc_signal<sc_uint<32>> PC_MEM2WBK_RM;
    sc_signal<bool>        CSR_WENABLE_RM;
    sc_signal<sc_uint<32>> CSR_RDATA_RM;

    // MEM-CSR interface

    sc_signal<sc_uint<12>> CSR_WADR_SM;
    sc_signal<sc_uint<32>> CSR_WDATA_SM;

    sc_signal<sc_uint<32>> MSTATUS_WDATA_RM;
    sc_signal<sc_uint<32>> MIP_WDATA_RM;
    sc_signal<sc_uint<32>> MEPC_WDATA_RM;
    sc_signal<sc_uint<32>> MCAUSE_WDATA_SM;
    sc_signal<sc_uint<32>> MEPC_SC;
    sc_signal<sc_uint<32>> MSTATUS_RC;
    sc_signal<sc_uint<32>> MTVEC_VALUE_RC;
    sc_signal<sc_uint<32>> MIP_VALUE_RC;
    sc_signal<sc_uint<32>> MIE_VALUE_RC;
    sc_signal<sc_uint<32>> MTVAL_WDATA_SM;
    sc_signal<bool>        CSR_ENABLE_SM;

    // MEM-IFETCH

    sc_signal<bool>        MRET_SM;
    sc_signal<sc_uint<32>> RETURN_ADRESS_SM;

    // MEM - Pipeline :

    sc_signal<bool> EXCEPTION_SM;
    sc_signal<bool> BUS_ERROR_SX;
    // WBK-REG interface

    sc_signal<sc_uint<6>>  WADR_SW;
    sc_signal<sc_uint<32>> WDATA_SW;
    sc_signal<bool>        WENABLE_SW;

    // Timer interface
    sc_signal<bool>        TIMER_CONFIG_WB_SC;
    sc_signal<bool>        TIMER_DIVIDER_WB_SC;
    sc_signal<sc_uint<32>> DATA_SC;
    sc_signal<sc_uint<64>> TIME_RT;
    sc_signal<bool>        TIMER_INT_ST;
    sc_signal<bool>        ACK_SP;

    // Dcache interface
    sc_out<sc_uint<2>>  MEM_SIZE_SM;
    sc_out<sc_uint<32>> MCACHE_ADR_SM;

    sc_out<sc_uint<32>> MCACHE_DATA_SM;
    sc_out<bool>        MCACHE_ADR_VALID_SM, MCACHE_STORE_SM, MCACHE_LOAD_SM;

    sc_in<sc_uint<32>> MCACHE_RESULT_SM;
    sc_in<bool>        MCACHE_STALL_SM;

    // Icache interface
    sc_out<sc_uint<32>> PC_SI;
    sc_out<bool>        PC_VALID_SI;

    sc_in<sc_bv<32>> INST_SIC;
    sc_in<bool>      STALL_SIC;

    // Debug
    sc_in<sc_uint<32>>  PC_INIT;
    sc_out<sc_uint<32>> DEBUG_PC_READ;
    sc_in<sc_uint<32>>  PROC_ID;

    // Pipeline Mode

    sc_signal<sc_uint<2>> CURRENT_MODE_SM;

    // Stage instanciation
    decod  dec_inst;
    exec   exec_inst;
    ifetch ifetch_inst;
    mem    mem_inst;
    reg    reg_inst;
    pred_branch_cache pred_branch_cache_inst;
    wbk    wbk_inst;
    csr    csr_inst;
    timer  timer_inst;

    Diviseur divider_inst;

    x0_multiplier      x0_multiplier_inst;
    x1_multiplier      x1_multiplier_inst;
    x2_multiplier      x2_multiplier_inst;

    void core_method();

    void trace(sc_trace_file * tf);
    SC_CTOR(core)
        : ifetch_inst("ifetch"),
          dec_inst("decod"),
          exec_inst("exec"),
          divider_inst("Diviseur"),
          x0_multiplier_inst("x0_multiplier"),
          mem_inst("mem"),
          x1_multiplier_inst("x1_multiplier"),
          wbk_inst("wbk"),
          x2_multiplier_inst("x2_multiplier"),
          reg_inst("reg"),
          csr_inst("csr"),
          timer_inst("timer") {
        SC_METHOD(core_method);
        sensitive << READ_PC_SR;


        // ICACHE-IFETCH interface
        ifetch_inst.INST_SIC(INST_SIC);
        ifetch_inst.STALL_SIC(STALL_SIC);
        ifetch_inst.PC_SI(PC_SI);
        ifetch_inst.PC_VALID_SI(PC_VALID_SI);

        // DECOD-IFETCH interface
        ifetch_inst.DEC2IF_EMPTY_SI(DEC2IF_EMPTY_SI);
        ifetch_inst.DEC2IF_POP_SI(DEC2IF_POP_SI);
        ifetch_inst.PC_RD(PC_RD);

        // IFETCH-DECOD interface
        ifetch_inst.IF2DEC_FLUSH_SD(IF2DEC_FLUSH_SD);
        ifetch_inst.IF2DEC_POP_SD(IF2DEC_POP_SD);
        ifetch_inst.IF2DEC_EMPTY_SI(IF2DEC_EMPTY_SI);
        ifetch_inst.INSTR_RI(INSTR_RI);
        ifetch_inst.PC_IF2DEC_RI(PC_IF2DEC_RI);
        ifetch_inst.EXCEPTION_RI(EXCEPTION_RI);
        ifetch_inst.PRED_BRANCH_ADR_RI(PRED_BRANCH_ADR_RI);
        ifetch_inst.PRED_BRANCH_TARGET_ADR_RI(PRED_BRANCH_TARGET_ADR_RI);
        ifetch_inst.PRED_BRANCH_MISS_OUT_SI(PRED_BRANCH_MISS_OUT_SI);
        ifetch_inst.PRED_BRANCH_CPT_OUT_SI(PRED_BRANCH_CPT_OUT_SI);
        ifetch_inst.PRED_BRANCH_LRU_OUT_SI(PRED_BRANCH_LRU_OUT_SI);
        ifetch_inst.PRED_BRANCH_PNT_OUT_SI(PRED_BRANCH_PNT_OUT_SI);

        // IFETCH -> PRED_BRANCH_CACHE
        ifetch_inst.PRED_BRANCH_CHECK_ADR_IN_SI(PRED_BRANCH_CHECK_ADR_IN_SI);

        // PRED_BRANCH_CACHE -> IFETCH
        ifetch_inst.PRED_BRANCH_MISS_OUT_SP(PRED_BRANCH_MISS_OUT_SP);                     // MISS/HIT for the cache MISS = 1 HIT = 0
        ifetch_inst.PRED_BRANCH_TARGET_ADR_OUT_SP(PRED_BRANCH_TARGET_ADR_OUT_SP);         // Branch target address
        ifetch_inst.PRED_BRANCH_LRU_OUT_SP(PRED_BRANCH_LRU_OUT_SP);                       // Less Recent Use
        ifetch_inst.PRED_BRANCH_CPT_OUT_SP(PRED_BRANCH_CPT_OUT_SP);                       // Branch taken times
        ifetch_inst.PRED_BRANCH_PNT_OUT_SP(PRED_BRANCH_PNT_OUT_SP);                       // Branch taken target address

        // INTERRUPTION
        ifetch_inst.INTERRUPTION_SE(INTERRUPTION_SE);
        ifetch_inst.CURRENT_MODE_SM(CURRENT_MODE_SM);
        ifetch_inst.MRET_SM(MRET_SM);
        ifetch_inst.RETURN_ADRESS_SM(RETURN_ADRESS_SM);

        // GLOBAL INTERFACE
        ifetch_inst.EXCEPTION_SM(EXCEPTION_SM);
        ifetch_inst.CLK(CLK);
        ifetch_inst.RESET(RESET);

        // IFETCH -> PRED_BRANCH_CACHE
        pred_branch_cache_inst.PRED_BRANCH_CHECK_ADR_IN_SI(PRED_BRANCH_CHECK_ADR_IN_SI);

        // EXEC -> PRED_BRANCH_CACHE
        pred_branch_cache_inst.PRED_BRANCH_CMD_IN_SE(PRED_BRANCH_CMD_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_WRITE_ADR_IN_SE(PRED_BRANCH_WRITE_ADR_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_TARGET_ADR_IN_SE(PRED_BRANCH_TARGET_ADR_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_CPT_IN_SE(PRED_BRANCH_CPT_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_LRU_IN_SE(PRED_BRANCH_LRU_IN_SE);


        // PRED_BRANCH_CACHE -> IFETCH
        pred_branch_cache_inst.PRED_BRANCH_MISS_OUT_SP(PRED_BRANCH_MISS_OUT_SP);                     // MISS/HIT for the cache MISS = 1 HIT = 0
        pred_branch_cache_inst.PRED_BRANCH_TARGET_ADR_OUT_SP(PRED_BRANCH_TARGET_ADR_OUT_SP);         // Branch target address
        pred_branch_cache_inst.PRED_BRANCH_LRU_OUT_SP(PRED_BRANCH_LRU_OUT_SP);                       // Less Recent Use
        pred_branch_cache_inst.PRED_BRANCH_CPT_OUT_SP(PRED_BRANCH_CPT_OUT_SP);                       // Branch taken times
        pred_branch_cache_inst.PRED_BRANCH_PNT_OUT_SP(PRED_BRANCH_PNT_OUT_SP);                       // Branch taken target address

        // GLOBAL INTERFACE
        pred_branch_cache_inst.CLK(CLK);
        pred_branch_cache_inst.RESET(RESET);






        dec_inst.DEC2IF_POP_SI(DEC2IF_POP_SI);
        dec_inst.DEC2IF_EMPTY_SD(DEC2IF_EMPTY_SI);
        dec_inst.PC_RD(PC_RD);
        dec_inst.PRED_FAILED_RD(PRED_FAILED_RD);
        dec_inst.PRED_SUCCESS_RD(PRED_SUCCESS_RD);
        dec_inst.INSTR_IS_BRANCH_RD(INSTR_IS_BRANCH_RD);
        dec_inst.BRANCH_INST_ADR_RD(BRANCH_INST_ADR_RD);
        dec_inst.BRANCH_TARGET_ADR_RD(BRANCH_TARGET_ADR_RD);

        dec_inst.PC_DEC2EXE_RD(PC_DEC2EXE_RD);
        dec_inst.PC_IF2DEC_RI(PC_IF2DEC_RI);
        dec_inst.INSTR_RI(INSTR_RI);
        dec_inst.IF2DEC_EMPTY_SI(IF2DEC_EMPTY_SI);

        dec_inst.PRED_BRANCH_ADR_RI(PRED_BRANCH_ADR_RI);
        dec_inst.PRED_TAKEN_RI(PRED_TAKEN_RI);

        dec_inst.IF2DEC_POP_SD(IF2DEC_POP_SD);
        dec_inst.IF2DEC_FLUSH_SD(IF2DEC_FLUSH_SD);

        dec_inst.OP1_RD(OP1_RD);
        dec_inst.OP2_RD(OP2_RD);
        dec_inst.EXE_CMD_RD(EXE_CMD_RD);
        dec_inst.NEG_OP2_RD(NEG_OP2_RD);
        dec_inst.WB_RD(WB_RD);
        dec_inst.SELECT_TYPE_OPERATIONS_RD(SELECT_TYPE_OPERATIONS_RD);
        dec_inst.SLT_RD(SLT_RD);
        dec_inst.SLTU_RD(SLTU_RD);

        dec_inst.MEM_DATA_RD(MEM_DATA_RD);
        dec_inst.MEM_LOAD_RD(MEM_LOAD_RD);
        dec_inst.MEM_STORE_RD(MEM_STORE_RD);
        dec_inst.MEM_SIGN_EXTEND_RD(MEM_SIGN_EXTEND_RD);
        dec_inst.MEM_SIZE_RD(MEM_SIZE_RD);

        dec_inst.DEC2EXE_POP_SE(DEC2EXE_POP_SE);
        dec_inst.DEC2EXE_EMPTY_SD(DEC2EXE_EMPTY_SD);

        dec_inst.RDATA1_SR(RDATA1_SR);
        dec_inst.RDATA2_SR(RDATA2_SR);

        dec_inst.RADR1_SD(RADR1_SD);
        dec_inst.RADR2_SD(RADR2_SD);

        dec_inst.EXE_DEST_RD(EXE_DEST_RD);
        dec_inst.KERNEL_ADR_SC(KERNEL_ADR_SC);

        dec_inst.READ_PC_SR(READ_PC_SR);
        dec_inst.WRITE_PC_SD(WRITE_PC_SD);
        dec_inst.WRITE_PC_ENABLE_SD(WRITE_PC_ENABLE_SD);

        dec_inst.DEST_RE(DEST_RE);
        dec_inst.EXE_RES_RE(EXE_RES_RE);
        dec_inst.DEST_RM(DEST_RM);
        dec_inst.MEM_RES_RM(MEM_RES_RM);
        dec_inst.EXE2MEM_EMPTY_SE(EXE2MEM_EMPTY_SE);
        dec_inst.MEM2WBK_EMPTY_SM(MEM2WBK_EMPTY_SM);

        dec_inst.PRED_ADR_SD(PRED_ADR_SD);
        dec_inst.PRED_TAKEN_SD(PRED_TAKEN_SD);

        dec_inst.PUSH_ADR_RAS_RD(PUSH_ADR_RAS_RD);
        dec_inst.POP_ADR_RAS_RD(POP_ADR_RAS_RD); 
        dec_inst.RETURN_ADR_RD(RETURN_ADR_RD);

        dec_inst.RET_INST_RD(RET_INST_RD);

        dec_inst.BP_R1_VALID_RD(BP_R1_VALID_RD);
        dec_inst.BP_R2_VALID_RD(BP_R2_VALID_RD);
        dec_inst.BP_RADR1_RD(BP_RADR1_RD);
        dec_inst.BP_RADR2_RD(BP_RADR2_RD);
        dec_inst.MEM_LOAD_RE(MEM_LOAD_RE);

        dec_inst.CSR_WENABLE_RE(CSR_WENABLE_RE);
        dec_inst.CSR_RDATA_RE(CSR_RDATA_RE);
        dec_inst.CSRRC_I_RD(CSRRC_I_RD);
        dec_inst.CSR_WENABLE_RM(CSR_WENABLE_RM);
        dec_inst.CSR_RDATA_RM(CSR_RDATA_RM);
        dec_inst.EBREAK_RD(EBREAK_RD);

        dec_inst.CSR_WENABLE_RD(CSR_WENABLE_RD);
        dec_inst.CSR_WADR_RD(CSR_WADR_RD);
        dec_inst.CSR_RADR_SD(CSR_RADR_SD);
        dec_inst.CSR_RDATA_SC(CSR_RDATA_SC);
        dec_inst.CSR_RDATA_RD(CSR_RDATA_RD);
        dec_inst.INTERRUPTION_SE(INTERRUPTION_SE);
        dec_inst.EXCEPTION_RI(EXCEPTION_RI);
        dec_inst.EXCEPTION_RD(EXCEPTION_RD);
        dec_inst.ENV_CALL_S_MODE_RD(ENV_CALL_S_MODE_RD);
        dec_inst.ENV_CALL_WRONG_MODE_RD(ENV_CALL_WRONG_MODE_RD);
        dec_inst.ILLEGAL_INSTRUCTION_RD(ILLEGAL_INSTRUCTION_RD);  // accessing stuff in wrong mode
        dec_inst.ADRESS_MISSALIGNED_RD(ADRESS_MISSALIGNED_RD);    // branch offset is misaligned
        dec_inst.ENV_CALL_U_MODE_RD(ENV_CALL_U_MODE_RD);
        dec_inst.ENV_CALL_M_MODE_RD(ENV_CALL_M_MODE_RD);
        dec_inst.EXCEPTION_SM(EXCEPTION_SM);
        dec_inst.MTVEC_VALUE_RC(MTVEC_VALUE_RC);
        dec_inst.MRET_SM(MRET_SM);
        dec_inst.BLOCK_BP_RD(BLOCK_BP_RD);
        dec_inst.CURRENT_MODE_SM(CURRENT_MODE_SM);
        dec_inst.MRET_RD(MRET_RD);
        dec_inst.RETURN_ADRESS_SM(RETURN_ADRESS_SM);
        dec_inst.INSTRUCTION_ACCESS_FAULT_RD(INSTRUCTION_ACCESS_FAULT_RD);
        dec_inst.MCAUSE_WDATA_SM(MCAUSE_WDATA_SM);
        dec_inst.MULT_INST_RD(MULT_INST_RD);
        dec_inst.MULT_INST_RE(MULT_INST_RE);
        dec_inst.MULT_INST_RM(MULT_INST_RM);
        dec_inst.PC_BRANCH_VALUE_RD(PC_BRANCH_VALUE_RD);

        dec_inst.CLK(CLK);
        dec_inst.RESET_N(RESET);


        // DEC2EXE PRED_BRANCH_CACHE
        dec_inst.PRED_BRANCH_ADR_RD(PRED_BRANCH_ADR_RD);        // address of the branch instruction
        dec_inst.PRED_BRANCH_MISS_RD(PRED_BRANCH_MISS_RD);      // branch miss
        dec_inst.PRED_BRANCH_TARGET_RD(PRED_BRANCH_TARGET_RD);  // target of the branch
        dec_inst.PRED_BRANCH_CPT_RD(PRED_BRANCH_CPT_RD);        // branch counter
        dec_inst.PRED_BRANCH_LRU_RD(PRED_BRANCH_LR_RD);         // branch last result
        dec_inst.PRED_BRANCH_PNT_RD(PRED_BRANCH_PNT_RD);        // branch prediction index pointer
        dec_inst.IS_BRANCH_RD(IS_BRANCH_RD);                    // is branch
        dec_inst.PRED_BRANCH_TAKEN_RD(PRED_BRANCH_TAKEN_RD);    // branch taken

        exec_inst.PRED_BRANCH_ADR_RD(PRED_BRANCH_ADR_RD);        // address of the branch instruction
        exec_inst.PRED_BRANCH_MISS_RD(PRED_BRANCH_MISS_RD);      // branch miss
        exec_inst.PRED_BRANCH_TARGET_RD(PRED_BRANCH_TARGET_RD);  // target of the branch
        exec_inst.PRED_BRANCH_CPT_RD(PRED_BRANCH_CPT_RD);        // branch counter
        exec_inst.PRED_BRANCH_LRU_RD(PRED_BRANCH_LR_RD);         // branch last result
        exec_inst.PRED_BRANCH_PNT_RD(PRED_BRANCH_PNT_RD);        // branch prediction index pointer
        exec_inst.IS_BRANCH_RD(IS_BRANCH_RD);                    // is branch
        exec_inst.PRED_BRANCH_TAKEN_RD(PRED_BRANCH_TAKEN_RD);    // branch taken

        //EXE2PRED_BRANCH_CACHE
        exec_inst.PRED_BRANCH_CMD_IN_SE(PRED_BRANCH_CMD_IN_SE);
        exec_inst.PRED_BRANCH_WRITE_ADR_IN_SE(PRED_BRANCH_WRITE_ADR_IN_SE);
        exec_inst.PRED_BRANCH_TARGET_ADR_IN_SE(PRED_BRANCH_TARGET_ADR_IN_SE);
        exec_inst.PRED_BRANCH_CPT_IN_SE(PRED_BRANCH_CPT_IN_SE);
        exec_inst.PRED_BRANCH_LRU_IN_SE(PRED_BRANCH_LRU_IN_SE);
        exec_inst.PRED_BRANCH_PNT_IN_SE(PRED_BRANCH_PNT_IN_SE);
        

        pred_branch_cache_inst.PRED_BRANCH_CMD_IN_SE(PRED_BRANCH_CMD_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_WRITE_ADR_IN_SE(PRED_BRANCH_WRITE_ADR_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_TARGET_ADR_IN_SE(PRED_BRANCH_TARGET_ADR_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_CPT_IN_SE(PRED_BRANCH_CPT_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_LRU_IN_SE(PRED_BRANCH_LRU_IN_SE);
        pred_branch_cache_inst.PRED_BRANCH_PNT_IN_SE(PRED_BRANCH_PNT_IN_SE);






        exec_inst.RADR1_RD(BP_RADR1_RD);
        exec_inst.RADR2_RD(BP_RADR2_RD);
        exec_inst.OP1_VALID_RD(BP_R1_VALID_RD);
        exec_inst.OP2_VALID_RD(BP_R2_VALID_RD);
        exec_inst.MEM_DEST_RM(DEST_RM);
        exec_inst.MEM_RES_RM(MEM_RES_RM);
        exec_inst.OP1_RD(OP1_RD);
        exec_inst.OP2_RD(OP2_RD);
        exec_inst.CMD_RD(EXE_CMD_RD);
        exec_inst.DEST_RD(EXE_DEST_RD);
        exec_inst.NEG_OP2_RD(NEG_OP2_RD);
        exec_inst.WB_RD(WB_RD);
        exec_inst.SELECT_TYPE_OPERATIONS_RD(SELECT_TYPE_OPERATIONS_RD);
        exec_inst.EBREAK_RD(EBREAK_RD);
        exec_inst.EBREAK_RE(EBREAK_RE);

        exec_inst.MULT_INST_RD(MULT_INST_RD);
        exec_inst.PC_DEC2EXE_RD(PC_DEC2EXE_RD);
        exec_inst.PC_EXE2MEM_RE(PC_EXE2MEM_RE);

        exec_inst.MEM_DATA_RD(MEM_DATA_RD);
        exec_inst.MEM_LOAD_RD(MEM_LOAD_RD);
        exec_inst.MEM_STORE_RD(MEM_STORE_RD);
        exec_inst.MEM_SIGN_EXTEND_RD(MEM_SIGN_EXTEND_RD);
        exec_inst.MEM_SIZE_RD(MEM_SIZE_RD);
        exec_inst.SLT_RD(SLT_RD);
        exec_inst.SLTU_RD(SLTU_RD);

        exec_inst.DEC2EXE_POP_SE(DEC2EXE_POP_SE);
        exec_inst.DEC2EXE_EMPTY_SD(DEC2EXE_EMPTY_SD);

        exec_inst.EXE_RES_RE(EXE_RES_RE);
        exec_inst.MEM_DATA_RE(MEM_DATA_RE);
        exec_inst.DEST_RE(DEST_RE);
        exec_inst.CSRRC_I_RD(CSRRC_I_RD);
        exec_inst.MEM_SIZE_RE(MEM_SIZE_RE);

        exec_inst.WB_RE(MEM_WB);
        exec_inst.MEM_SIGN_EXTEND_RE(MEM_SIGN_EXTEND_RE);
        exec_inst.MEM_LOAD_RE(MEM_LOAD_RE);
        exec_inst.MEM_STORE_RE(MEM_STORE_RE);
        exec_inst.MULT_INST_RE(MULT_INST_RE);
        exec_inst.MULT_SEL_HIGH_RE(MULT_SEL_HIGH_RE);
        exec_inst.EXE2MEM_EMPTY_SE(EXE2MEM_EMPTY_SE);
        exec_inst.EXE2MEM_POP_SM(EXE2MEM_POP_SM);

        exec_inst.CSR_WENABLE_RM(CSR_WENABLE_RM);
        exec_inst.CSR_RDATA_RM(CSR_RDATA_RM);
        exec_inst.KERNEL_ADR_SC(KERNEL_ADR_SC);

        exec_inst.INTERRUPTION_SE(INTERRUPTION_SE);
        exec_inst.MACHINE_SOFTWARE_INTERRUPT_SX(MACHINE_SOFTWARE_INTERRUPT_SX);
        exec_inst.MACHINE_TIMER_INTERRUPT_SX(MACHINE_TIMER_INTERRUPT_SX);
        exec_inst.MACHINE_EXTERNAL_INTERRUPT_SX(MACHINE_EXTERNAL_INTERRUPT_SX);
        exec_inst.MACHINE_SOFTWARE_INTERRUPT_SE(MACHINE_SOFTWARE_INTERRUPT_SE);
        exec_inst.MACHINE_TIMER_INTERRUPT_SE(MACHINE_TIMER_INTERRUPT_SE);
        exec_inst.MACHINE_EXTERNAL_INTERRUPT_SE(MACHINE_EXTERNAL_INTERRUPT_SE);
        exec_inst.EXCEPTION_RD(EXCEPTION_RD);
        exec_inst.CSR_WENABLE_RD(CSR_WENABLE_RD);
        exec_inst.CSR_WENABLE_RE(CSR_WENABLE_RE);
        exec_inst.CSR_WADR_RD(CSR_WADR_RD);
        exec_inst.CSR_WADR_RE(CSR_WADR_RE);
        exec_inst.CSR_RDATA_RE(CSR_RDATA_RE);
        exec_inst.CSR_RDATA_RD(CSR_RDATA_RD);
        exec_inst.ENV_CALL_S_MODE_RD(ENV_CALL_S_MODE_RD);
        exec_inst.ENV_CALL_WRONG_MODE_RD(ENV_CALL_WRONG_MODE_RD);
        exec_inst.ILLEGAL_INSTRUCTION_RD(ILLEGAL_INSTRUCTION_RD);  // accessing stuff in wrong mode
        exec_inst.ADRESS_MISSALIGNED_RD(ADRESS_MISSALIGNED_RD);    // branch offset is misaligned
        exec_inst.ENV_CALL_U_MODE_RD(ENV_CALL_U_MODE_RD);
        exec_inst.ENV_CALL_M_MODE_RD(ENV_CALL_M_MODE_RD);
        exec_inst.MRET_RD(MRET_RD);

        exec_inst.EXCEPTION_RE(EXCEPTION_RE);
        exec_inst.LOAD_ADRESS_MISSALIGNED_RE(LOAD_ADRESS_MISSALIGNED_RE);
        exec_inst.LOAD_ACCESS_FAULT_RE(LOAD_ACCESS_FAULT_RE);
        exec_inst.STORE_ADRESS_MISSALIGNED_RE(STORE_ADRESS_MISSALIGNED_RE);
        exec_inst.STORE_ACCESS_FAULT_RE(STORE_ACCESS_FAULT_RE);
        exec_inst.ENV_CALL_WRONG_MODE_RE(ENV_CALL_WRONG_MODE_RE);
        exec_inst.ENV_CALL_U_MODE_RE(ENV_CALL_U_MODE_RE);
        exec_inst.ILLEGAL_INSTRUCTION_RE(ILLEGAL_INSTRUCTION_RE);
        exec_inst.INSTRUCTION_ADRESS_MISSALIGNED_RE(INSTRUCTION_ADRESS_MISSALIGNED_RE);
        exec_inst.ENV_CALL_S_MODE_RE(ENV_CALL_S_MODE_RE);
        exec_inst.ENV_CALL_M_MODE_RE(ENV_CALL_M_MODE_RE);
        exec_inst.EXCEPTION_SM(EXCEPTION_SM);
        exec_inst.BLOCK_BP_RD(BLOCK_BP_RD);
        exec_inst.CURRENT_MODE_SM(CURRENT_MODE_SM);
        exec_inst.MRET_RE(MRET_RE);
        exec_inst.INSTRUCTION_ACCESS_FAULT_RD(INSTRUCTION_ACCESS_FAULT_RD);
        exec_inst.INSTRUCTION_ACCESS_FAULT_RE(INSTRUCTION_ACCESS_FAULT_RE);
        exec_inst.PC_BRANCH_VALUE_RD(PC_BRANCH_VALUE_RD);
        exec_inst.PC_BRANCH_VALUE_RE(PC_BRANCH_VALUE_RE);

        exec_inst.OP1_SE(op1_se);
        exec_inst.OP2_SE(op2_se);

        exec_inst.START_SE(START_SE);
        exec_inst.DIVIDER_RES_OUTPUT(DIVIDER_RES_OUTPUT);
        exec_inst.DIV_BUSY_SE(DIV_BUSY_SE);
        exec_inst.DONE_SE(DONE_SE);

        exec_inst.MULT_INST_RM(MULT_INST_RM);
        exec_inst.MEM2WBK_EMPTY_SM(MEM2WBK_EMPTY_SM);

        exec_inst.CLK(CLK);
        exec_inst.RESET(RESET);

        //Divider
        divider_inst.OP1_SE(op1_se);
        divider_inst.OP2_SE(op2_se);
        divider_inst.START_SE(START_SE);
        divider_inst.CMD_RD(EXE_CMD_RD);
        divider_inst.DIVIDER_RES_OUTPUT(DIVIDER_RES_OUTPUT);
        divider_inst.BUSY_SE(DIV_BUSY_SE);
        divider_inst.DONE_SE(DONE_SE);
        divider_inst.CLK(CLK);

        //X0 - MULTIPLIER port map :

        x0_multiplier_inst.OP1_SE(op1_se);
        x0_multiplier_inst.OP2_SE(op2_se);
        x0_multiplier_inst.EXE_CMD_RD(EXE_CMD_RD);
        x0_multiplier_inst.X02X1_POP_SX1(x02x1_POP_SX1);
        x0_multiplier_inst.DEC2X0_EMPTY_SD(DEC2EXE_EMPTY_SD);

        x0_multiplier_inst.RES_RX0(multiplier_out_sx0);
        x0_multiplier_inst.SELECT_HIGHER_BITS_RX0(select_higher_bits_rx0);
        x0_multiplier_inst.SIGNED_RES_RX0(SIGNED_RES_RX0);
        x0_multiplier_inst.X02X1_EMPTY_SX0(x02x1_EMPTY_SX0);

        x0_multiplier_inst.CLK(CLK);
        x0_multiplier_inst.RESET(RESET);

        // MEM port map :

        mem_inst.EXE_RES_RE(EXE_RES_RE);  // 0
        mem_inst.MEM_DATA_RE(MEM_DATA_RE);
        mem_inst.DEST_RE(DEST_RE);
        mem_inst.MEM_SIZE_RE(MEM_SIZE_RE);

        mem_inst.WB_RE(MEM_WB);
        mem_inst.SIGN_EXTEND_RE(MEM_SIGN_EXTEND_RE);
        mem_inst.LOAD_RE(MEM_LOAD_RE);
        mem_inst.STORE_RE(MEM_STORE_RE);
        mem_inst.MULT_INST_RE(MULT_INST_RE);
        mem_inst.MULT_INST_RM(MULT_INST_RM);
        mem_inst.EXE2MEM_EMPTY_SE(EXE2MEM_EMPTY_SE);
        mem_inst.EXE2MEM_POP_SM(EXE2MEM_POP_SM);

        mem_inst.MEM_RES_RM(MEM_RES_RM);
        mem_inst.DEST_RM(DEST_RM);
        mem_inst.WB_RM(WB_RM);
        mem_inst.EBREAK_RE(EBREAK_RE);

        mem_inst.MEM2WBK_EMPTY_SM(MEM2WBK_EMPTY_SM);
        mem_inst.MEM2WBK_POP_SW(MEM2WBK_POP_SW);

        mem_inst.MCACHE_ADR_SM(MCACHE_ADR_SM);
        mem_inst.MCACHE_DATA_SM(MCACHE_DATA_SM);
        mem_inst.MCACHE_ADR_VALID_SM(MCACHE_ADR_VALID_SM);
        mem_inst.MCACHE_STORE_SM(MCACHE_STORE_SM);
        mem_inst.MCACHE_LOAD_SM(MCACHE_LOAD_SM);  // 19
        mem_inst.MEM_SIZE_SM(MEM_SIZE_SM);

        mem_inst.MCACHE_RESULT_SM(MCACHE_RESULT_SM);
        mem_inst.MCACHE_STALL_SM(MCACHE_STALL_SM);

        mem_inst.PC_EXE2MEM_RE(PC_EXE2MEM_RE);
        mem_inst.PC_MEM2WBK_RM(PC_MEM2WBK_RM);

        mem_inst.INTERRUPTION_SE(INTERRUPTION_SE);
        mem_inst.MACHINE_SOFTWARE_INTERRUPT_SE(MACHINE_SOFTWARE_INTERRUPT_SE);
        mem_inst.MACHINE_TIMER_INTERRUPT_SE(MACHINE_TIMER_INTERRUPT_SE);
        mem_inst.MACHINE_EXTERNAL_INTERRUPT_SE(MACHINE_EXTERNAL_INTERRUPT_SE);
        mem_inst.CSR_WADR_SE(CSR_WADR_RE);
        mem_inst.CSR_WADR_SM(CSR_WADR_SM);
        mem_inst.CSR_WENABLE_RE(CSR_WENABLE_RE);
        mem_inst.CSR_WENABLE_RM(CSR_WENABLE_RM);
        mem_inst.CSR_WDATA_SM(CSR_WDATA_SM);
        mem_inst.CSR_RDATA_RM(CSR_RDATA_RM);
        mem_inst.CSR_RDATA_RE(CSR_RDATA_RE);

        mem_inst.EXCEPTION_RE(EXCEPTION_RE);
        mem_inst.LOAD_ADRESS_MISSALIGNED_RE(LOAD_ADRESS_MISSALIGNED_RE);
        mem_inst.LOAD_ACCESS_FAULT_RE(LOAD_ACCESS_FAULT_RE);
        mem_inst.STORE_ADRESS_MISSALIGNED_RE(STORE_ADRESS_MISSALIGNED_RE);
        mem_inst.STORE_ACCESS_FAULT_RE(STORE_ACCESS_FAULT_RE);
        mem_inst.ENV_CALL_U_MODE_RE(ENV_CALL_U_MODE_RE);
        mem_inst.ENV_CALL_WRONG_MODE_RE(ENV_CALL_WRONG_MODE_RE);
        mem_inst.ILLEGAL_INSTRUCTION_RE(ILLEGAL_INSTRUCTION_RE);
        mem_inst.INSTRUCTION_ADRESS_MISSALIGNED_RE(INSTRUCTION_ADRESS_MISSALIGNED_RE);
        mem_inst.ENV_CALL_S_MODE_RE(ENV_CALL_S_MODE_RE);  // 39
        mem_inst.ENV_CALL_M_MODE_RE(ENV_CALL_M_MODE_RE);
        mem_inst.MRET_RE(MRET_RE);
        mem_inst.INSTRUCTION_ACCESS_FAULT_RE(INSTRUCTION_ACCESS_FAULT_RE);

        mem_inst.BUS_ERROR_SX(BUS_ERROR_SX);

        mem_inst.EXCEPTION_SM(EXCEPTION_SM);
        mem_inst.CURRENT_MODE_SM(CURRENT_MODE_SM);
        mem_inst.RETURN_ADRESS_SM(RETURN_ADRESS_SM);
        mem_inst.MRET_SM(MRET_SM);

        mem_inst.MSTATUS_WDATA_RM(MSTATUS_WDATA_RM);
        mem_inst.MIP_WDATA_RM(MIP_WDATA_RM);
        mem_inst.MEPC_WDATA_RM(MEPC_WDATA_RM);
        mem_inst.MCAUSE_WDATA_SM(MCAUSE_WDATA_SM);

        mem_inst.MEPC_SC(MEPC_SC);
        mem_inst.MSTATUS_RC(MSTATUS_RC);
        mem_inst.MTVEC_VALUE_RC(MTVEC_VALUE_RC);
        mem_inst.MIP_VALUE_RC(MIP_VALUE_RC);
        mem_inst.MIE_VALUE_RC(MIE_VALUE_RC);
        mem_inst.MTVAL_WDATA_SM(MTVAL_WDATA_SM);

        mem_inst.CSR_ENABLE_SM(CSR_ENABLE_SM);
        mem_inst.PC_BRANCH_VALUE_RE(PC_BRANCH_VALUE_RE);

        mem_inst.CLK(CLK);
        mem_inst.RESET(RESET);

        // X1 - MULTIPLIER port map :

        x1_multiplier_inst.IN_RX0(multiplier_out_sx0);
        x1_multiplier_inst.SELECT_HIGHER_BITS_RX0(select_higher_bits_rx0);
        x1_multiplier_inst.SIGNED_RES_RX0(SIGNED_RES_RX0);
        x1_multiplier_inst.X12X2_POP_SX2(x12x2_POP_SX2);
        x1_multiplier_inst.RES_RX1(multiplier_out_sx1);
        x1_multiplier_inst.SELECT_HIGHER_BITS_RX1(select_higher_bits_rx1);
        x1_multiplier_inst.SIGNED_RES_RX1(SIGNED_RES_RX1);
        x1_multiplier_inst.X12X2_EMPTY_SX1(x12x2_EMPTY_SX1);
        x1_multiplier_inst.X02X1_EMPTY_SX0(x02x1_EMPTY_SX0);
        x1_multiplier_inst.X02X1_POP_SX1(x02x1_POP_SX1);
        x1_multiplier_inst.CLK(CLK);
        x1_multiplier_inst.RESET(RESET);
        // REG port map :

        reg_inst.RADR1_SD(RADR1_SD);
        reg_inst.RADR2_SD(RADR2_SD);
        reg_inst.RDATA1_SR(RDATA1_SR);
        reg_inst.RDATA2_SR(RDATA2_SR);

        reg_inst.WADR_SW(WADR_SW);
        reg_inst.WENABLE_SW(WENABLE_SW);
        reg_inst.WDATA_SW(WDATA_SW);

        reg_inst.READ_PC_SR(READ_PC_SR);
        reg_inst.WRITE_PC_SD(WRITE_PC_SD);
        reg_inst.WRITE_PC_ENABLE_SD(WRITE_PC_ENABLE_SD);
        reg_inst.PC_INIT(PC_INIT);

        reg_inst.CLK(CLK);
        reg_inst.RESET_N(RESET);

        wbk_inst.MEM_RES_RM(MEM_RES_RM);
        wbk_inst.DEST_RM(DEST_RM);
        wbk_inst.WB_RM(WB_RM);
        wbk_inst.MEM2WBK_EMPTY_SM(MEM2WBK_EMPTY_SM);
        wbk_inst.MEM2WBK_POP_SW(MEM2WBK_POP_SW);

        wbk_inst.WADR_SW(WADR_SW);
        wbk_inst.WDATA_SW(WDATA_SW);
        wbk_inst.WENABLE_SW(WENABLE_SW);
        wbk_inst.CSR_RDATA_RM(CSR_RDATA_RM);
        wbk_inst.CSR_WENABLE_RM(CSR_WENABLE_RM);

        wbk_inst.PC_MEM2WBK_RM(PC_MEM2WBK_RM);

        wbk_inst.INTERRUPTION_SE(INTERRUPTION_SE);
        wbk_inst.CURRENT_MODE_SM(CURRENT_MODE_SM);

        wbk_inst.MULT_INST_RM(MULT_INST_RM);
        wbk_inst.X2_RES_RX2(multiplier_out_sx2);

        wbk_inst.CLK(CLK);
        wbk_inst.RESET(RESET);

        // X1 - MULTIPLIER port map :

        x2_multiplier_inst.IN_RX1(multiplier_out_sx1);
        x2_multiplier_inst.SELECT_HIGHER_BITS_RX1(select_higher_bits_rx1);
        x2_multiplier_inst.SIGNED_RES_RX1(SIGNED_RES_RX1);
        x2_multiplier_inst.X12X2_POP_SX2(x12x2_POP_SX2);
        x2_multiplier_inst.RES_RX2(multiplier_out_sx2);
        x2_multiplier_inst.X12X2_EMPTY_SX1(x12x2_EMPTY_SX1);
        x2_multiplier_inst.CLK(CLK);
        x2_multiplier_inst.RESET(RESET);

        csr_inst.CSR_WADR_SM(CSR_WADR_SM);
        csr_inst.CSR_WDATA_SM(CSR_WDATA_SM);
        csr_inst.CSR_ENABLE_SM(CSR_ENABLE_SM);

        csr_inst.CSR_RADR_SD(CSR_RADR_SD);
        csr_inst.CSR_RDATA_SC(CSR_RDATA_SC);

        csr_inst.EXCEPTION_SM(EXCEPTION_SM);
        csr_inst.MSTATUS_WDATA_RM(MSTATUS_WDATA_RM);
        csr_inst.MIP_WDATA_RM(MIP_WDATA_RM);
        csr_inst.MEPC_WDATA_RM(MEPC_WDATA_RM);
        csr_inst.MCAUSE_WDATA_SM(MCAUSE_WDATA_SM);

        csr_inst.MEPC_SC(MEPC_SC);
        csr_inst.MSTATUS_RC(MSTATUS_RC);
        csr_inst.MTVEC_VALUE_RC(MTVEC_VALUE_RC);
        csr_inst.MIP_VALUE_RC(MIP_VALUE_RC);
        csr_inst.MIE_VALUE_RC(MIE_VALUE_RC);
        csr_inst.MCAUSE_SC(MCAUSE_SC);
        csr_inst.MTVAL_WDATA_SM(MTVAL_WDATA_SM);

        csr_inst.TIMER_CONFIG_WB_SC(TIMER_CONFIG_WB_SC);
        csr_inst.TIMER_DIVIDER_WB_SC(TIMER_DIVIDER_WB_SC);
        csr_inst.TIME_RT(TIME_RT);
        csr_inst.KERNEL_ADR_SC(KERNEL_ADR_SC);
        csr_inst.TIMER_INT_ST(TIMER_INT_ST);
        csr_inst.ACK_SP(ACK_SP);

        csr_inst.CLK(CLK);
        csr_inst.RESET_N(RESET);
        csr_inst.PROC_ID(PROC_ID);
        
        timer_inst.TIMER_CONFIG_WB_SC(TIMER_CONFIG_WB_SC);
        timer_inst.TIMER_DIVIDER_WB_SC(TIMER_DIVIDER_WB_SC);
        timer_inst.DATA_SC(CSR_WDATA_SM);
        timer_inst.TIME_RT(TIME_RT);
        timer_inst.TIMER_INT_ST(TIMER_INT_ST);
        timer_inst.ACK_SP(ACK_SP);

        timer_inst.CLK(CLK);
        timer_inst.RESET(RESET);
    }
};