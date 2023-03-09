#pragma once
#include <systemc.h>
#include "../config.h"
#include "../../UTIL/debug_util.h"

SC_MODULE(pred_branch_cache) {
    
    // Input Port :
        sc_in<sc_uint<32>> PRED_ADR_BRANCH_CHECK_IN_SD;     // Branch instruction address need to be check in the table if it exist or not

        
        sc_in<bool> CMD;                                    // CMD for the write/update it's depends on the signal MISS
        sc_in<sc_uint<4>> PRED_COUNTER_OUT_SD;              // Branch taken times
        sc_in<sc_uint<32>> PRED_ADR_TARGET_IN_SD;           // Branch taken target address
        sc_in<sc_uint<32>> PRED_ADR_BRANCH_WRITE_IN_SD;     // Branch instruction address need to be write into the table 
        sc_in<sc_uint<2>> PRED_WRITE_INDEX_IN_SD;           // The index where to put the data

    // Output Port :
        sc_out<bool> PRED_MISS_SP;                          // MISS/HIT for the cache
        sc_out<sc_uint<4>> PRED_COUNTER_OUT_SP;             // Branch taken times
        sc_out<sc_uint<4>> PRED_LRU_OUT_SP;                 // Less recent use
        sc_out<sc_uint<32>> PRED_ADR_TARGET_OUT_SP;         // Branch taken target address

    // Global Interface :
        sc_in_clk   CLK;
        sc_in<bool> RESET;

    // Cache Content: 
        sc_signal<bool> present[PRED_BRANCH_CACHE_SIZE];                            // Bit of present
        sc_signal<sc_uint<32>> branch_inst_adr[PRED_BRANCH_CACHE_SIZE];             // Branch instruction address
        sc_signal<sc_uint<32>> branch_success_target_adr[PRED_BRANCH_CACHE_SIZE];   // Branch success target address
        sc_signal<sc_uint<2>> branch_success_time[PRED_BRANCH_CACHE_SIZE];          // Branch success times
        sc_signal<sc_uint<2>> lru[PRED_BRANCH_CACHE_SIZE];                          // Less recent use


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