#pragma once
#include <systemc.h>
#include <iostream>
#include "../../UTIL/debug_util.h"
#include "../../UTIL/fifo.h"
#include "../config.h"

enum  // PREDICTION STATE
{     // 1,2,4,8 -> one-hot
    strongly_taken     = 1,
    weakly_taken       = 2,
    weakly_not_taken   = 4,
    strongly_not_taken = 8
};

SC_MODULE(ifetch) {

    // ICACHE - IFETCH Interface :
        sc_in<sc_bv<32>>        INST_SIC;                       // instruction fetch from ICACHE
        sc_in<bool>             STALL_SIC;                      // 
        sc_out<sc_uint<32>>     PC_SI;                          // address of the instruction going to fetch
        sc_out<bool>            PC_VALID_SI;                    // 

    // DEC2IF Interface :
        sc_in<bool>             DEC2IF_EMPTY_SI;                // DEC2IF fifo empty
        sc_out<bool>            DEC2IF_POP_SI;                  // DEC2IF fifo pop
        sc_in<sc_uint<32>>      PC_RD;                          // PC_NEXT (always pc+4 ?to be confirmed)

    // IF2DEC Interface :
        sc_in<bool>             IF2DEC_FLUSH_SD;                // IF2DEC fifo flush
        sc_out<bool>            IF2DEC_EMPTY_SI;                // IF2DEC fifo empty
        sc_out<sc_bv<32>>       INSTR_RI;                       // instruction get from fifo IF2DEC
        sc_out<sc_uint<32>>     PC_IF2DEC_RI;                   // pc get from fifo IF2DEC
        sc_out<bool>            EXCEPTION_RI;                   // ?i dont understand  tells if an instruction have been made in IFETCH
        sc_out<sc_uint<32>>     PRED_BRANCH_ADR_RI;             // branch instruction address
        sc_out<sc_unint<32>>    PRED_BRANCH_TARGET_ADR_RI;      // branch target address
        sc_out<bool>            PRED_BRANCH_MISS_OUT_SI;        // MISS/HIT for the cache MISS = 1 HIT = 0
        sc_out<sc_uint<2>>      PRED_BRANCH_CPT_OUT_SI;         // branch taken times
        sc_out<bool>            PRED_BRANCH_LRU_OUT_SI;         // Less Recent Use
        sc_out<sc_uint<2>>      PRED_BRANCH_PNT_OUT_SI;         // branch taken target address

    // PRED_BRANCH_CACHE - IFETCH Interface :
        sc_out<sc_uint<32>>     PRED_BRANCH_CHECK_ADR_IN_SI;    // branch instruction address need to be check in the table if it exist or not
        sc_in<bool>             PRED_BRANCH_MISS_OUT_SP;        // MISS/HIT for the cache MISS = 1 HIT = 0
        sc_in<sc_uint<32>>      PRED_BRANCH_TARGET_ADR_OUT_SP;  // branch target address
        sc_in<bool>             PRED_BRANCH_LRU_OUT_SP;         // less recent use
        sc_in<sc_uint<2>>       PRED_BRANCH_CPT_OUT_SP;         // branch taken times
        sc_in<sc_uint<2>>       PRED_BRANCH_PNT_OUT_SP;         // branch taken target address

    // Interruption :
        sc_in<bool>             INTERRUPTION_SE;
        sc_in<sc_uint<2>>       CURRENT_MODE_SM;
        sc_in<bool>             MRET_SM;
        sc_in<sc_uint<32>>      RETURN_ADRESS_SM;

    // Global Interface :
        sc_in<bool>             EXCEPTION_SM;
        sc_in_clk               CLK;
        sc_in<bool>             RESET;

    // FIFO
        fifo<if2dec_size>       fifo_if2dec;

    // Internals signals :
        sc_signal<bool>               if2dec_push;              // IF2DEC fifo push
        sc_signal<bool>               if2dec_full;              // IF2DEC fifo full
        sc_signal<sc_bv<if2dec_size>> if2dec_in;                // input data of IF2DEC fifo
        sc_signal<sc_bv<if2dec_size>> if2dec_out;               // output data of IF2DEC fifo


    void fetch_method();
    void exception();




    void trace(sc_trace_file * tf);

    SC_CTOR(ifetch) : fifo_if2dec("if2dec") {
        fifo_if2dec.DIN_S(if2dec_in);
        fifo_if2dec.DOUT_R(if2dec_out);
        fifo_if2dec.EMPTY_S(IF2DEC_EMPTY_SI);
        fifo_if2dec.FULL_S(if2dec_full);
        fifo_if2dec.PUSH_S(if2dec_push);
        fifo_if2dec.POP_S(IF2DEC_POP_SD);
        fifo_if2dec.CLK(CLK);
        fifo_if2dec.RESET_N(RESET);

        SC_METHOD(fetch_method);
        sensitive << INST_SIC << DEC2IF_EMPTY_SI << if2dec_full << PC_RD << IF2DEC_FLUSH_SD << STALL_SIC << RESET
                  << EXCEPTION_SM << MRET_SM << RETURN_ADRESS_SM << PRED_BRANCH_ADR_RI << PRED_TAKEN_RI << PRED_SUCCESS_RD
                  << PRED_FAILED_RD << PRED_ADR_TAKEN_SI << PRED_NEXT_ADR_SI;
        SC_METHOD(exception);
        sensitive << RESET << EXCEPTION_SM;
    }
};