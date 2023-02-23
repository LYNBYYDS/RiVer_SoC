#pragma once

#include <systemc.h>
#include <iostream>
#include <string>
#include "../../UTIL/debug_util.h"

SC_MODULE(shifter) {

    sc_in<sc_uint<32>> DIN_SE;          // value to be shift
    sc_in<sc_uint<5>>  SHIFT_VAL_SE;    // shift amount
    sc_in<sc_uint<2>>  CMD_SE;          // kind of shift
                                        // cmd == 0 : sll
                                        // cmd == 1 : srl
                                        // cmd == 2/3 : sra

    sc_out<sc_uint<32>> DOUT_SE;        // result of the shift operation

    // Internal signals :

    sc_signal<bool> sll_se;             // = 1 when is doing sll shift operation
    sc_signal<bool> srl_se;             // = 1 when is doing srl shift operation
    sc_signal<bool> sra_se;             // = 1 when is doing sra shift operation
    sc_signal<sc_uint<32>> sll_dout_se; // temporary output stockthe sll operation result
    sc_signal<sc_uint<32>> sr_dout_se;  // temporary output stock the sr operation result

    void decode_cmd();                  // decode the cmd type 
    void shifter_sll();                 // component can do sll  operation
    void shifter_sr();                  // component can do shift right operation 
                                        // arithmetic or logic one
    void shifter_agregate();            // component to choose the result
    void trace(sc_trace_file * tf);

    SC_CTOR(shifter) {

        // decode the cmd type when the cmd input change
        SC_METHOD(decode_cmd);
        sensitive << CMD_SE;

        // recalculate the result of sll operation when shift operator change or shift amount change 
        SC_METHOD(shifter_sll);
        sensitive << DIN_SE << SHIFT_VAL_SE;

        // recalculate the result of sr operation when shift operator change or shift amount change or change type between srl/sra
        SC_METHOD(shifter_sr);
        sensitive << DIN_SE << SHIFT_VAL_SE << sra_se;

        // change the result of the shift when any of the result change or change shift operation type
        SC_METHOD(shifter_agregate);
        sensitive << sll_dout_se << sr_dout_se << sll_se << srl_se << sra_se;
    }
};
