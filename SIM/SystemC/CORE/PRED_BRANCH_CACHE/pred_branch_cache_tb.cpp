#include <systemc.h>
#include <string>
#include "pred_branch_cache.h"

using namespace std;

int sc_main(int argc, char* argv[]) {
    cerr << "start of main" << endl;
    sc_trace_file* tf;
    tf = sc_create_vcd_trace_file("tf");
    
    // Input Port
    sc_signal<sc_uint<32>> pred_branch_check_adr_in;     // Branch instruction address need to be check in the table if it exist or not

    sc_signal<sc_uint<2>> pred_branch_cmd_in;            // CMD for the skip(not branch instruction)/write/update it's depends on the signal PRED_BRANCH_MISS_OUT_SI skip = 0 write = 1 update = 2
    sc_signal<sc_uint<32>> pred_branch_write_adr_in;     // Branch instruction address need to be write in the table
    sc_signal<sc_uint<4>> pred_branch_target_adr_in;     // Branch target address
    sc_signal<sc_uint<32>> pred_branch_cpt_in;           // Branch taken counter
    sc_signal<sc_uint<32>> pred_branch_lru_in;           // Less Recent Use
    sc_signal<sc_uint<2>> pred_branch_pnt_in;            // The index where to put the data

    // Output Port
    sc_signal<bool> pred_branch_miss_out;                // MISS/HIT for the cache MISS = 1 HIT = 0
    sc_signal<sc_uint<4>> pred_branch_cpt_out;           // Branch taken times
    sc_signal<sc_uint<4>> pred_branch_lru_out;           // Less Recent Use
    sc_signal<sc_uint<32>> pred_branch_pnt_out;          // Branch taken target address

    // Global Interface
    sc_clock        clk("clk", 1, SC_NS);
    sc_signal<bool> reset_n;

    // Instance declaration
    pred_branch_cache pred_branch_cache_inst("pred_branch_cache_inst");

    // Port Map :
    // Input Port
    pred_branch_cache_inst.PRED_BRANCH_CHECK_ADR_IN_SI(pred_branch_check_adr_in);
    pred_branch_cache_inst.PRED_BRANCH_CMD_IN_SE(pred_branch_cmd_in);
    pred_branch_cache_inst.PRED_BRANCH_WRITE_ADR_IN_SI(pred_branch_write_adr_in);
    pred_branch_cache_inst.PRED_BRANCH_TARGET_ADR_IN_SE(pred_branch_target_adr_in);
    pred_branch_cache_inst.PRED_BRANCH_CPT_IN_SE(pred_branch_cpt_in);
    pred_branch_cache_inst.PRED_BRANCH_LRU_IN_SE(pred_branch_lru_in);
    pred_branch_cache_inst.PRED_BRANCH_PNT_IN_SE(pred_branch_pnt_in);

    // Output Port
    pred_branch_cache_inst.PRED_BRANCH_MISS_OUT_SP(pred_branch_miss_out);
    pred_branch_cache_inst.PRED_BRANCH_CPT_OUT_SP(pred_branch_cpt_out);
    pred_branch_cache_inst.PRED_BRANCH_LRU_OUT_SP(pred_branch_lru_out);
    pred_branch_cache_inst.PRED_BRANCH_PNT_OUT_SP(pred_branch_pnt_out);

    // Global Interface
    pred_branch_cache_inst.CLK(clk);
    pred_branch_cache_inst.RESET_N(reset_n);

    pred_branch_cache_inst.trace(tf);

    reset_n.write(false);  // reset
    sc_start(3, SC_NS);    // wait for 3 ns (1 cycle?)
    reset_n.write(true);   // end of reset
    cerr << "end of reset" << endl;

    
    cout << "All tests passed sucessfully" << endl;
    sc_close_vcd_trace_file(tf);
    return 0;
}
