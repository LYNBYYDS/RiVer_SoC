#pragma once
#include <systemc.h>
#include "../config.h"
#include "../../UTIL/debug_util.h"

SC_MODULE(pred_branch_cache) {
    
    // Input Port :
        sc_in<sc_uint<32>> PRED_ADR_BRANCH_IN_SD;   // Branch instruction address
        sc_in<sc_uint<32>> PRED_ADR_AIM_IN_SD;    // Branch success aim address
        sc_in<sc_uint<1>> PRED_ISSUCESS_IN_SD;       // Branch success or not 1 = sucess
        sc_in<sc_uint<2>> PRED_LRU_IN_SD;           // Branch LRU  when the table is full less recently used choose which to be replace
                                                    // Branch counter = 0 will be remove first (has more proprity)
    //sc_in<bool> PRED_READ_SD;
    //sc_in<bool> PRED_WRITE_SD;

    // Output Port :
        sc_out<sc_uint<32>> PRED_ADR_AIM_OUT_SP;
        sc_out<sc_uint<4>> PRED_COUNTER_OUT_SP;
        sc_out<bool> PRED_LRU_OUT_SP;

    // Global Interface :
        sc_in_clk   CLK;
        sc_in<bool> RESET;

    // Cache Content: 
        sc_signal<sc_uint<2>> pred_branch_cache_used;
        sc_signal<sc_uint<69>> pred_branch_cache[PRED_BRANCH_CACHE_SIZE];
        /*  0       bit of present
            32-1    branch instruction address
            64-33   branch success aim address
            66-65   branch success times
            68-67   LRU
        */

    void pred_check();
    void pred_write();
    void trace(sc_trace_file * tf);

    SC_CTOR(pred_branch_cache) {
        
        SC_METHOD(pred_check);
        sensitive << PRED_ADR_BRANCH_IN_SD << RESET;

        for (int i = 0; i < PRED_CACHE_SIZE; i++)
            sensitive << PRED_VAL[i];
        SC_CTHREAD(pred_write, reg::CLK.pos());
        reset_signal_is(RESET, false);
    }
};