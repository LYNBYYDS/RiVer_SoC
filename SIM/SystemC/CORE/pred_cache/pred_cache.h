#pragma once
#include <systemc.h>
#include "constants.h"
#include "debug_util.h"
SC_MODULE(pred_cache) {
    // INPUT Port :

    sc_in<sc_uint<32>> PRED_ADR_KEY_IN_SD;
    sc_in<sc_uint<32>> PRED_ADR_CIBLE_IN_SD;
    sc_in<sc_uint<4>> PRED_STATE_IN_SD;
    sc_in<bool> PRED_LRU_IN_SD;
    sc_in<bool> PRED_READ_SD;
    sc_in<bool> PRED_WRITE_SD;

    // OUTPUT Port :

    sc_out<sc_uint<32>> PRED_ADR_CIBLE_OUT_SR;
    sc_out<sc_uint<4>> PRED_STATE_OUT_SR;
    sc_out<bool> PRED_LRU_OUT_SR;


    // Global Interface :

    sc_in_clk   CLK;
    sc_in<bool> RESET;

    // Registres :
    // sc_signal<sc_uint<32>> PRED_ADR_KEY[4];
    // sc_signal<sc_uint<32>> PRED_ADR_CIBLE[4];
    // sc_signal<sc_uint<4>> PRED_STATE[4];
    // sc_signal<bool> PRED_LRU[4];
    sc_signal<sc_uint<69>> PRED_VAL[PRED_CACHE_SIZE];

    void pred_check();
    void pred_write();
    void trace(sc_trace_file * tf);

    SC_CTOR(pred_cache) {
        SC_METHOD(pred_check);
        sensitive << PRED_ADR_KEY_IN_SD << PRED_READ_SD << RESET;
        for (int i = 0; i < PRED_CACHE_SIZE; i++)
            sensitive << PRED_VAL[i];
        SC_CTHREAD(pred_write, reg::CLK.pos());
        reset_signal_is(RESET, false);
    }
};