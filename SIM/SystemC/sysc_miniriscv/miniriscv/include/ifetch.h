#pragma once
#include <systemc.h>
#include <iostream>
#include "constants.h"
#include "debug_util.h"
#include "fifo.h"

SC_MODULE(ifetch) {

    /*****************************************************
            Interface with Prediction Branch Cache
    ******************************************************/

    sc_out<sc_uint<32>> PRED_BRANCH_CHECK_ADR_IN_SI;    // Branch instruction address need to be check in the table if it exist or not

    sc_in<bool> PRED_BRANCH_MISS_OUT_SP;                // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_in<sc_uint<32>>PRED_BRANCH_TARGET_ADR_OUT_SP;    // Branch target address
    sc_in<sc_uint<2>> PRED_BRANCH_PNT_OUT_SP;           // Branch taken target address
    sc_in<bool> PRED_BRANCH_LRU_OUT_SP;                 // Less Recent Use
    sc_in<sc_uint<2>> PRED_BRANCH_CPT_OUT_SP;           // Branch taken times

    /*****************************************************
        Interface with Instruction Cache, or RAM
    ******************************************************/

    // the instruction read from memory
    sc_in<sc_bv<32>> IC_INST_SI;
    // a stall signal, in case of a cache miss, or memory delay
    sc_in<bool> IC_STALL_SI;
    // The adress of the instruction to fetch
    sc_out<sc_uint<32>> ADR_SI;
    // Whether the adress is valid (no need to do a memory access if it is not)
    sc_out<bool> ADR_VALID_SI;

    /*****************************************************
                    Interface with Dec2if
    ******************************************************/

    sc_in<bool>  DEC2IF_EMPTY_SD;
    sc_out<bool> DEC2IF_POP_SI;

    /*****************************************************
                    Interface with If2Dec
    ******************************************************/

    sc_in<bool>  IF2DEC_FLUSH_SD;  // allow to flush if2dec in case of a branch
    sc_in<bool>  IF2DEC_POP_SD;
    sc_out<bool> IF2DEC_EMPTY_SI;


    /*****************************************************
                    Interface with pb_if2dec
    ******************************************************/

    sc_in<bool>  PB_IF2DEC_FLUSH_SD;  // allow to flush pb_if2dec in case of a branch
    sc_in<bool>  PB_IF2DEC_POP_SD;
    sc_out<bool> PB_IF2DEC_EMPTY_SI;

    /*****************************************************
                    Interface with Decod
    ******************************************************/
    sc_out<sc_uint<32>> PC_RI;     // pc sent to if2dec
    sc_out<sc_bv<32>>   INSTR_RI;  // instruction sent to if2dec

    sc_in<sc_bv<32>>    PC_RD;     // PC coming to fetch an instruction

    sc_out<bool>            PRED_BRANCH_MISS_RI;            // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_out<sc_uint<32>>     PRED_BRANCH_ADR_RI;             // branch instruction address
    sc_out<sc_unint<32>>    PRED_BRANCH_TARGET_ADR_RI;      // branch target address
    sc_out<sc_uint<2>>      PRED_BRANCH_PNT_RI;             // branch taken target address
    sc_out<bool>            PRED_BRANCH_LRU_RI;             // Less Recent Use
    sc_out<sc_uint<2>>      PRED_BRANCH_CPT_RI;             // branch taken times
    
    // Global Interface :
        sc_in_clk   CLK;
        sc_in<bool> RESET;

    // Component instanciation
        fifo<IF2DEC_SIZE> if2dec;
        fifo<PC_IF2DEC_SIZE> pb_if2dec;
        fifo<TARGET_PC_SIZE> target_pc;

    // Internals signals :
        sc_signal<bool>               IF2DEC_PUSH_SI;
        sc_signal<bool>               IF2DEC_FULL_SI;
        sc_signal<sc_bv<IF2DEC_SIZE>> if2dec_in_si;
        sc_signal<sc_bv<IF2DEC_SIZE>> if2dec_out_si; 

        sc_signal<bool>               pb_if2dec_PUSH_SI;
        sc_signal<bool>               pb_if2dec_FULL_SI;
        sc_signal<sc_bv<IF2DEC_SIZE>> pb_if2dec_in_si;
        sc_signal<sc_bv<IF2DEC_SIZE>> pb_if2dec_out_si;  

        sc_signal<bool>               target_pc_out_si;
        sc_signal<bool>               target_pc_empty_si;
        sc_signal<bool>               target_pc_full_si;
        sc_signal<bool>               target_pc_push_si;
        sc_signal<bool>               target_pc_pop_si;
        sc_signal<sc_bv<IF2DEC_SIZE>> target_pc_in_si;
        sc_signal<sc_bv<IF2DEC_SIZE>> target_pc_out_si; 

        sc_signal<bool>               miss_ri;
        sc_signal<sc_uint<32>>        target_adr_ri;
        sc_signal<sc_uint<2>>         cpt_ri;

    void fetch_method();
    void trace(sc_trace_file * tf);

    SC_CTOR(ifetch) : if2dec("if2dec"), pb_if2dec("pb_if2dec"), target_pc("target_pc"){
        if2dec.DATAIN_S(if2dec_in_si);
        if2dec.DATAOUT_R(if2dec_out_si);
        if2dec.EMPTY_S(IF2DEC_EMPTY_SI);
        if2dec.FULL_S(IF2DEC_FULL_SI);
        if2dec.PUSH_S(IF2DEC_PUSH_SI);
        if2dec.POP_S(IF2DEC_POP_SD);
        if2dec.CLK(CLK);
        if2dec.RESET(RESET);

        pb_if2dec.DATAIN_S(pb_if2dec_in_si);
        pb_if2dec.DATAOUT_R(pb_if2dec_out_si);
        pb_if2dec.EMPTY_S(PB_IF2DEC_EMPTY_SI);
        pb_if2dec.FULL_S(PB_IF2DEC_FULL_SI);
        pb_if2dec.PUSH_S(PB_IF2DEC_PUSH_SI);
        pb_if2dec.POP_S(PB_IF2DEC_POP_SD);
        pb_if2dec.CLK(CLK);
        pb_if2dec.RESET(RESET);

        target_pc.DATAIN_S(target_pc_in_si);
        target_pc.DATAOUT_R(target_pc_out_si);
        target_pc.EMPTY_S(target_pc_empty_si);
        target_pc.FULL_S(target_pc_full_si);
        target_pc.PUSH_S(target_pc_push_si);
        target_pc.POP_S(target_pc_pop_si);
        target_pc.CLK(CLK);
        target_pc.RESET(RESET);


        SC_METHOD(fetch_method);
        sensitive << PC_RD 
                  << IC_INST_SI << DEC2IF_EMPTY_SD << IF2DEC_FULL_SI << IF2DEC_FLUSH_SD
                  << IC_STALL_SI << RESET;
    }
};
