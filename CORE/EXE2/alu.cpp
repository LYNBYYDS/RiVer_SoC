#include "alu.h"

void alu::operation() {
    sc_uint<32> res_interne;
    if (CMD_SE.read() == 0) {
        res_interne = OP1_SE_S2.read() + OP2_SE_S2.read() + CIN_SE;
    } else if (CMD_SE.read() == 1) {
        res_interne = OP1_SE_S2.read() & OP2_SE_S2.read();
    } else if (CMD_SE.read() == 2) {
        res_interne = OP1_SE_S2.read() | OP2_SE_S2.read();
    } else if (CMD_SE.read() == 3) {
        res_interne = OP1_SE_S2.read() ^ OP2_SE_S2.read();
    }
    RES_SE.write(res_interne);
}

void alu::trace(sc_trace_file* tf) {
    sc_trace(tf, OP1_SE_S2, GET_NAME(OP1_SE_S2));
    sc_trace(tf, OP2_SE_S2, GET_NAME(OP2_SE_S2));
    sc_trace(tf, CIN_SE, GET_NAME(CIN_SE));
    sc_trace(tf, CMD_SE, GET_NAME(CMD_SE));
    sc_trace(tf, RES_SE, GET_NAME(RES_SE));
}