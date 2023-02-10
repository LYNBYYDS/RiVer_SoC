#pragma once

// Header files
#include <systemc.h>
#include "../../UTIL/debug_util.h"

SC_MODULE(reg) {

    // I/O Ports    
        // Global Interface :

            sc_in_clk          CLK;
            sc_in<bool>        RESET_N;
            sc_in<sc_uint<32>> PC_INIT;
        
        // Reading Port :

            sc_in<sc_uint<6>> RADR1_SD;                 // the number of the first register to be read
            sc_out<sc_uint<32>> RDATA1_SR;              // the data stocked in the first register

            sc_in<sc_uint<6>> RADR2_SD;                 // the number of the second register to be read
            sc_out<sc_uint<32>> RDATA2_SR;              // the data stocked in the second register

        // Writing Port :

            sc_in<sc_uint<6>>  WADR_SW;                 // the number of the register to write in
            sc_in<bool>        WENABLE_SW;              // the valide bit for the register if = 1 then is allowed to write on, otherwise is not allowed
            sc_in<sc_uint<32>> WDATA_SW;                // the value which going to be write in the register

        // PC Gestion :
            
            sc_in<sc_uint<32>> WRITE_PC_SD;             // the value which goint to be write in the register PC
            sc_in<bool>        WRITE_PC_ENABLE_SD;      // the valide bit for the PC register if = 1 then is allowed to write on, otherwise is not allowed
            sc_out<sc_uint<32>> READ_PC_SR;             // the data stocked in the PC register

    

    // vector stocked the data of the 33 registers
        sc_signal<sc_uint<32>> REG_RR[33];              
    
    // methods
        void reading_adresses();
        void writing_adresse();
        void trace(sc_trace_file * tf);

    // 
        SC_CTOR(reg) {
        
        // register reading function
        SC_METHOD(reading_adresses);
        
        // sensitive list :
        // when the reset signal changes
        sensitive(RESET_N);
        // when the number of the register to be read changes
        sensitive << RADR1_SD << RADR2_SD;
        // when the data in the registers has changed, reread the data in the registers
        for (int i = 0; i < 33; i++)
            sensitive << REG_RR[i];

        // Register write function, sensitive list : positive edge of clock(CLK)
        //                          reset when signal reset_n is false in mode synchrone
        SC_CTHREAD(writing_adresse, reg::CLK.pos());
        reset_signal_is(RESET_N, false);
    }
};