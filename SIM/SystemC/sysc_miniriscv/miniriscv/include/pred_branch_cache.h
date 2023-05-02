#pragma once
#include <systemc.h>
#include <iostream>
#include "constants.h"
#include "debug_util.h"

SC_MODULE(pred_branch_cache) {
    
    // Input Port :
        sc_in<sc_uint<32>> PRED_BRANCH_CHECK_ADR_IN_SI;                                 // Branch instruction address need to be check in the table if it exist or not

        sc_in<sc_uint<2>> PRED_BRANCH_CMD_IN_SE;                                        // CMD for the skip(not branch instruction)/write/update it's depends on the signal PRED_BRANCH_MISS_OUT_SI skip = 0 write = 1 update = 2
        sc_in<sc_uint<32>> PRED_BRANCH_WRITE_ADR_IN_SI;                                 // Branch instruction address need to be write in the table
        sc_in<sc_uint<32>> PRED_BRANCH_TARGET_ADR_IN_SE;                                // Branch target address
        sc_in<sc_uint<2>> PRED_BRANCH_PNT_IN_SE;                                        // The index where to put the data
        sc_in<bool> PRED_BRANCH_LRU_IN_SE;                                              // Less Recent Use
        sc_in<sc_uint<PRED_BRANCH_CACHE_CPT_SIZE>> PRED_BRANCH_CPT_IN_SE;               // Branch taken counter

    // Output Port :
        sc_out<bool> PRED_BRANCH_MISS_OUT_SP;                                           // MISS/HIT for the cache MISS = 1 HIT = 0
        sc_out<sc_uint<32>>PRED_BRANCH_TARGET_ADR_OUT_SP;                               // Branch target address
        sc_out<sc_uint<2>> PRED_BRANCH_PNT_OUT_SP;                                      // Branch taken target address
        sc_out<bool> PRED_BRANCH_LRU_OUT_SP;                                            // Less Recent Use
        sc_out<sc_uint<2>> PRED_BRANCH_CPT_OUT_SP;                                      // Branch taken times

    // Global Interface :
        sc_in_clk   CLK;
        sc_in<bool> RESET_N;

    // Cache Content: 
        sc_signal<bool> present[PRED_BRANCH_CACHE_SIZE];                                // Bit of present
        sc_signal<sc_uint<32>> branch_inst_adr[PRED_BRANCH_CACHE_SIZE];                 // Branch instruction address
        sc_signal<sc_uint<32>> branch_target_adr[PRED_BRANCH_CACHE_SIZE];               // Branch success target address
        sc_signal<sc_uint<2>> branch_counter[PRED_BRANCH_CACHE_SIZE];                   // Branch success times
        sc_signal<bool> lru[PRED_BRANCH_CACHE_SIZE];                                    // Less Recent Use
        sc_signal<bool> inverse_lru;                                                    // LRU mechanism tell 0/1 have the less priority, when = 0 then the one have LRU = 0 is less priority otherwise is opposite 
        sc_signal<sc_uint<2>> p_nb;                                                     // Number of cache case used/(with high priority) if inverse_lru = 0 then is use directly otherwise is 4-p_nb


    void pred_check();
    void pred_write();
    void trace(sc_trace_file * tf);

    SC_CTOR(pred_branch_cache) {
        
        SC_METHOD(pred_check);
        sensitive << PRED_BRANCH_CHECK_ADR_IN_SI << RESET_N;
        sensitive << inverse_lru << p_nb;
        for (int i = 0; i < PRED_BRANCH_CACHE_SIZE; i++)
            sensitive << present[i] << branch_inst_adr[i] << branch_target_adr[i] << branch_counter[i] << lru[i] ;

        
        SC_THREAD(pred_write);
        sensitive << PRED_BRANCH_CMD_IN_SE << PRED_BRANCH_WRITE_ADR_IN_SI << PRED_BRANCH_TARGET_ADR_IN_SE << PRED_BRANCH_CPT_IN_SE << PRED_BRANCH_LRU_IN_SE << PRED_BRANCH_PNT_IN_SE << RESET_N;
        reset_signal_is(RESET_N, false);
    }
};
