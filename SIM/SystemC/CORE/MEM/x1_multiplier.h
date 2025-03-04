#pragma once
#include <systemc.h>
#include "../../UTIL/debug_util.h"
#include "../../UTIL/fifo.h"

#define x12x2_size          130

SC_MODULE(x1_multiplier)
{
    // input :
    sc_in<sc_bv<320>> IN_RX0;
    sc_in<bool>       SELECT_HIGHER_BITS_RX0;
    sc_in<bool>       SIGNED_RES_RX0;
    sc_in<bool>       X02X1_EMPTY_SX0;
    sc_in<bool>       X12X2_POP_SX2;

    // output :
    sc_out<sc_bv<128>>        RES_RX1;
    sc_out<bool>              SELECT_HIGHER_BITS_RX1;
    sc_out<bool>              SIGNED_RES_RX1;
    sc_out<bool>              X12X2_EMPTY_SX1;
    sc_out<bool>              X02X1_POP_SX1;

    // General interace : 
    sc_in_clk   CLK;
    sc_in<bool> RESET;

    //data from input
    sc_signal<sc_bv<64>> M[5];

    //Res from stage 6
    sc_signal<sc_bv<64>> product_s6[4];

    //Res from stage 7
    sc_signal<sc_bv<64>> product_s7[2];

    //Res from stage 8
    sc_signal<sc_bv<64>> product_s8[2];

    // fifo x12x2
    sc_signal<sc_bv<x12x2_size>> x12x2_din_sx1;
    sc_signal<sc_bv<x12x2_size>> x12x2_dout_sx1;
    sc_signal<bool> x12x2_full_sx1;
    sc_signal<bool> x12x2_push_sx1;

    fifo<x12x2_size> fifo_if2dec;

    void parse_data();
    //stage 6 (2 CSA)
    void CSA1();
    void CSA2();

    //stage 7 (1 CSA remind product_s6 M3)
    void CSA3();

    //stage 8 (1 CSA)
    void CSA4();

    void manage_fifo();
    void fifo_concat();
    void fifo_unconcat();

    void trace(sc_trace_file* tf);

    
    SC_CTOR(x1_multiplier) :
    fifo_if2dec("x12x2")
    {
        fifo_if2dec.DIN_S(x12x2_din_sx1);
        fifo_if2dec.DOUT_R(x12x2_dout_sx1);
        fifo_if2dec.EMPTY_S(X12X2_EMPTY_SX1);
        fifo_if2dec.FULL_S(x12x2_full_sx1);
        fifo_if2dec.PUSH_S(x12x2_push_sx1);
        fifo_if2dec.POP_S(X12X2_POP_SX2);
        fifo_if2dec.CLK(CLK);
        fifo_if2dec.RESET_N(RESET);

        SC_METHOD(parse_data);
        sensitive << IN_RX0;
        SC_METHOD(CSA1);
        sensitive << M[0] << M[1] << M[2];

        SC_METHOD(CSA3);
        sensitive << product_s6[0] << product_s6[1] << M[3];

        SC_METHOD(CSA4);
        sensitive << product_s7[0] << product_s7[1] << M[4];

        SC_METHOD(manage_fifo);
        sensitive << x12x2_full_sx1 << X02X1_EMPTY_SX0;

        SC_METHOD(fifo_concat);
        sensitive << SELECT_HIGHER_BITS_RX0 << product_s8[0] << product_s8[1] << SIGNED_RES_RX0;

        SC_METHOD(fifo_unconcat);
        sensitive << x12x2_dout_sx1;
    }
};