#include "x0_multiplier.h"

void x0_multiplier::operation()
{
    // +-------------------------------------------------+-------------------------------------------+
    // |               WALLACE MULTIPLIER                |              SIGNED EXTENSION             |
    // +-------------------------------------------------+-------------------------------------------+
    // |                        a3    a2   a1   a0       |                                           |
    // |                      x b3    b2   b1   b0       |  (-A)*(-B)                                |
    // |                       ------------------        |      =  !A * !B + !A + !B + 1             |
    // | 0    0     0     0    b0a3  b0a2 b0a1 b0a0 = M0 |         \     /    |    |   |             |
    // | 0    0     0    b1a3  b1a2  b1a1 b1a0  0   = M1 |   <=====_____/     |    |  c[0] of CLA    |
    // | 0    0    b2a3  b2a2  b2a1  b2a0  0    0   = M2 |                    |  33° prod            |
    // | 0   b3a3  b3a2  b3a1  b3a0   0    0    0   = M3 |                32° prod                   |
    // |    ----------------------------------------     |   (-A)*B = !A * B + B                                        |
    // |                                                 |                                           |
    // +-------------------------------------------------+-------------------------------------------+

    sc_uint<32> op1 = OP1_SE;
    sc_uint<32> op2 = OP2_SE;

    // signed extension
    
    bool signed_type = EXE_CMD_RD.read() != 3;

    select_higher_bits_sx0 = (EXE_CMD_RD.read() != 1);

    if(EXE_CMD_RD.read() == 2 || EXE_CMD_RD.read() == 1)
    {
        if(op1[31] == 1 && op2[31] == 1)
        {
            op1 = (~op1) + 1;
            op2 = (~op2) + 1;
            signed_type = false;
            signed_res_sx0 = false;
        }
        else if(op1[31] == 0 && op2[31] == 1)
        {
            sc_uint<32> tmp = op1;
            op1 = op2;
            op2 = tmp;
        }
    }
        
    // generating partial product
    for(int i = 0; i < 32; i++)
    {
        sc_bv<64> prod = 0;
        if(op2[i] != 0)
        {
            for(int j = i; j < i+32; j++)
            {
                prod[j] = (bool)(op2[i] & op1[j-i]);
            }
            if(signed_type && op1[31] == 1)
                for(int j = i+32; j < 64; j++)
                    prod[j] = 1;
        }
        product[i] = prod;
    }
    
}
//stage 1 - M33 CSA_32_30 CSA_29_27 CSA_26_24 CSA_23_21 CSA_20_18 CSA_17_15 
//        -     CSA_14_12 CSA_11_9  CSA_8_6   CSA_5_3   CSA_2_0
void x0_multiplier::CSA_1()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[0] ^ (sc_bv<64>)product[1]; 
    product_s1[0] =  xor_ ^ (sc_bv<64>)product[2];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[0] & (sc_bv<64>)product[1]) | (xor_ & (sc_bv<64>)product[2]);
    product_s1[1] = shf; 
}
void x0_multiplier::CSA_2()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[3] ^ (sc_bv<64>)product[4]; 
    product_s1[2] =  xor_ ^ (sc_bv<64>)product[5];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[3] & (sc_bv<64>)product[4]) | (xor_ & (sc_bv<64>)product[5]);  
    product_s1[3] = shf;
}
void x0_multiplier::CSA_3()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[6] ^ (sc_bv<64>)product[7]; 
    product_s1[4] =  xor_ ^ (sc_bv<64>)product[8];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[6] & (sc_bv<64>)product[7]) | (xor_ & (sc_bv<64>)product[8]);  
    product_s1[5] = shf;
}
void x0_multiplier::CSA_4()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[9] ^ (sc_bv<64>)product[10]; 
    product_s1[6] =  xor_ ^ (sc_bv<64>)product[11];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[9] & (sc_bv<64>)product[10]) | (xor_ & (sc_bv<64>)product[11]);  
    product_s1[7] = shf;
}
void x0_multiplier::CSA_5()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[12] ^ (sc_bv<64>)product[13]; 
    product_s1[8] =  xor_ ^ (sc_bv<64>)product[14];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[12] & (sc_bv<64>)product[13]) | (xor_ & (sc_bv<64>)product[14]);  
    product_s1[9] = shf;
}
void x0_multiplier::CSA_6()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[15] ^ (sc_bv<64>)product[16]; 
    product_s1[10] =  xor_ ^ (sc_bv<64>)product[17];

    sc_bv<64> shf = 0;
    shf.range(63, 1)  = ((sc_bv<64>)product[15] & (sc_bv<64>)product[16]) | (xor_ & (sc_bv<64>)product[17]);  
    product_s1[11] = shf;
}
void x0_multiplier::CSA_7()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[18] ^ (sc_bv<64>)product[19]; 
    product_s1[12] =  xor_ ^ (sc_bv<64>)product[20];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[18] & (sc_bv<64>)product[19]) | (xor_ & (sc_bv<64>)product[20]);  
    product_s1[13] = shf;
}
void x0_multiplier::CSA_8()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[21] ^ (sc_bv<64>)product[22]; 
    product_s1[14] =  xor_ ^ (sc_bv<64>)product[23];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[21] & (sc_bv<64>)product[22]) | (xor_ & (sc_bv<64>)product[23]);  
    product_s1[15] = shf;
}
void x0_multiplier::CSA_9()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[24] ^ (sc_bv<64>)product[25]; 
    product_s1[16] =  xor_ ^ (sc_bv<64>)product[26];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[24] & (sc_bv<64>)product[25]) | (xor_ & (sc_bv<64>)product[26]);  
    product_s1[17] = shf;
}
void x0_multiplier::CSA_10()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[27] ^ (sc_bv<64>)product[28]; 
    product_s1[18] =  xor_ ^ (sc_bv<64>)product[29];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[27] & (sc_bv<64>)product[28]) | (xor_ & (sc_bv<64>)product[29]);  
    product_s1[19] = shf;
}
/*void x0_multiplier::CSA_11()
{
    sc_bv<64> xor_ = (sc_bv<64>)product[30] ^ (sc_bv<64>)product[31]; 
    product_s1[20] =  xor_ ^ (sc_bv<64>)product[32];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product[30] & (sc_bv<64>)product[31]) | (xor_ & (sc_bv<64>)product[32]);  
    product_s1[21] = shf;
}*/

//stage 2 - M33 product_s1(21)  CSA_20_18 CSA_17_15 
//        - CSA_14_12 CSA_11_9  CSA_8_6   CSA_5_3   CSA_2_0

void x0_multiplier::CSA_11()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[0] ^ (sc_bv<64>)product_s1[1]; 
    product_s2[0] =  xor_ ^ (sc_bv<64>)product_s1[2];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[0] & (sc_bv<64>)product_s1[1]) | (xor_ & (sc_bv<64>)product_s1[2]);  
    product_s2[1] = shf;
}
void x0_multiplier::CSA_12()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[3] ^ (sc_bv<64>)product_s1[4]; 
    product_s2[2] =  xor_ ^ (sc_bv<64>)product_s1[5];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[3] & (sc_bv<64>)product_s1[4]) | (xor_ & (sc_bv<64>)product_s1[5]);  
    product_s2[3] = shf;
}
void x0_multiplier::CSA_13()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[6] ^ (sc_bv<64>)product_s1[7]; 
    product_s2[4] =  xor_ ^ (sc_bv<64>)product_s1[8];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[6] & (sc_bv<64>)product_s1[7]) | (xor_ & (sc_bv<64>)product_s1[8]);  
    product_s2[5] = shf;
}
void x0_multiplier::CSA_14()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[9] ^ (sc_bv<64>)product_s1[10]; 
    product_s2[6] =  xor_ ^ (sc_bv<64>)product_s1[11];
    
    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[9] & (sc_bv<64>)product_s1[10]) | (xor_ & (sc_bv<64>)product_s1[11]);  
    product_s2[7] = shf;
}
void x0_multiplier::CSA_15()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[12] ^ (sc_bv<64>)product_s1[13]; 
    product_s2[8] =  xor_ ^ (sc_bv<64>)product_s1[14];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[12] & (sc_bv<64>)product_s1[13]) | (xor_ & (sc_bv<64>)product_s1[14]);  
    product_s2[9] = shf;
}
void x0_multiplier::CSA_16()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[15] ^ (sc_bv<64>)product_s1[16]; 
    product_s2[10] =  xor_ ^ (sc_bv<64>)product_s1[17];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[15] & (sc_bv<64>)product_s1[16]) | (xor_ & (sc_bv<64>)product_s1[17]);  
    product_s2[11] = shf;
}
void x0_multiplier::CSA_17()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s1[18] ^ (sc_bv<64>)product_s1[19]; 
    product_s2[12] =  xor_ ^ (sc_bv<64>)product[30];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s1[18] & (sc_bv<64>)product_s1[19]) | (xor_ & (sc_bv<64>)product[30]);  
    product_s2[13] = shf;
}
//stage 3 - M33 CSA_14_12 CSA_11_9  CSA_8_6   CSA_5_3   CSA_2_0

void x0_multiplier::CSA_18()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s2[0] ^ (sc_bv<64>)product_s2[1]; 
    product_s3[0] =  xor_ ^ (sc_bv<64>)product_s2[2];
    
    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s2[0] & (sc_bv<64>)product_s2[1]) | (xor_ & (sc_bv<64>)product_s2[2]);  
    product_s3[1] = shf;
}
void x0_multiplier::CSA_19()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s2[3] ^ (sc_bv<64>)product_s2[4]; 
    product_s3[2] =  xor_ ^ (sc_bv<64>)product_s2[5];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s2[3] & (sc_bv<64>)product_s2[4]) | (xor_ & (sc_bv<64>)product_s2[5]);  
    product_s3[3] = shf;
}
void x0_multiplier::CSA_20()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s2[6] ^ (sc_bv<64>)product_s2[7]; 
    product_s3[4] =  xor_ ^ (sc_bv<64>)product_s2[8];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s2[6] & (sc_bv<64>)product_s2[7]) | (xor_ & (sc_bv<64>)product_s2[8]);  
    product_s3[5] = shf;
}
void x0_multiplier::CSA_21()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s2[9] ^ (sc_bv<64>)product_s2[10]; 
    product_s3[6] =  xor_ ^ (sc_bv<64>)product_s2[11];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s2[9] & (sc_bv<64>)product_s2[10]) | (xor_ & (sc_bv<64>)product_s2[11]);  
    product_s3[7] = shf;
}
void x0_multiplier::CSA_22()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s2[12] ^ (sc_bv<64>)product_s2[13]; 
    product_s3[8] =  xor_ ^ (sc_bv<64>)product[31];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s2[12] & (sc_bv<64>)product_s2[13]) | (xor_ & (sc_bv<64>)product[31]);  
    product_s3[9] = shf;
}

//stage 4 - M33 product_s3(9) CSA_8_6   CSA_5_3   CSA_2_0
void x0_multiplier::CSA_23()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s3[0] ^ (sc_bv<64>)product_s3[1]; 
    product_s4[0] =  xor_ ^ (sc_bv<64>)product_s3[2];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s3[0] & (sc_bv<64>)product_s3[1]) | (xor_ & (sc_bv<64>)product_s3[2]);  
    product_s4[1] = shf;
}
void x0_multiplier::CSA_24()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s3[3] ^ (sc_bv<64>)product_s3[4]; 
    product_s4[2] =  xor_ ^ (sc_bv<64>)product_s3[5];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s3[3] & (sc_bv<64>)product_s3[4]) | (xor_ & (sc_bv<64>)product_s3[5]);  
    product_s4[3] = shf;
}
void x0_multiplier::CSA_25()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s3[6] ^ (sc_bv<64>)product_s3[7]; 
    product_s4[4] =  xor_ ^ (sc_bv<64>)product_s3[8];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s3[6] & (sc_bv<64>)product_s3[7]) | (xor_ & (sc_bv<64>)product_s3[8]);  
    product_s4[5] = shf;
}
//stage 5 - M33 product_s3(9) CSA_5_3   CSA_2_0
void x0_multiplier::CSA_26()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s4[0] ^ (sc_bv<64>)product_s4[1]; 
    product_s5[0] =  xor_ ^ (sc_bv<64>)product_s4[2];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s4[0] & (sc_bv<64>)product_s4[1]) | (xor_ & (sc_bv<64>)product_s4[2]);  
    product_s5[1] = shf;
}
void x0_multiplier::CSA_27()
{
    sc_bv<64> xor_ = (sc_bv<64>)product_s4[3] ^ (sc_bv<64>)product_s4[4]; 
    product_s5[2] =  xor_ ^ (sc_bv<64>)product_s4[5];

    sc_bv<64> shf = 0;
    shf.range(63, 1) = ((sc_bv<64>)product_s4[3] & (sc_bv<64>)product_s4[4]) | (xor_ & (sc_bv<64>)product_s4[5]);  
    product_s5[3] = shf;
}
void x0_multiplier::fifo_concat() {
    sc_bv<x02x1_size> ff_din;
    ff_din[321]            = select_higher_bits_sx0;
    ff_din[320]            = signed_res_sx0;   
    ff_din.range(319, 256) = (sc_bv_base)product_s3[9];
    ff_din.range(255, 192) = (sc_bv_base)product_s5[3];
    ff_din.range(191, 128) = (sc_bv_base)product_s5[2];
    ff_din.range(127, 64)  = (sc_bv_base)product_s5[1];
    ff_din.range(63, 0)    = (sc_bv_base)product_s5[0];

    x02x1_din_sx0.write(ff_din);
}
void x0_multiplier::fifo_unconcat()
{
    sc_bv<x02x1_size> ff_dout = x02x1_dout_sx0.read();
    SELECT_HIGHER_BITS_RX0.write((bool)ff_dout[321]);
    SIGNED_RES_RX0.write((bool)ff_dout[320]);
    RES_RX0.write(ff_dout.range(319, 0));
}
void x0_multiplier::manage_fifo() 
{
    bool stall = x02x1_full_sx0.read() || DEC2X0_EMPTY_SD.read();
    x02x1_push_sx0.write(!stall);
}
void x0_multiplier::trace(sc_trace_file* tf)
{
    sc_trace(tf, x02x1_full_sx0, GET_NAME(x02x1_full_sx0));
    sc_trace(tf, DEC2X0_EMPTY_SD, GET_NAME(DEC2X0_EMPTY_SD));
    sc_trace(tf, OP1_SE, GET_NAME(OP1_SE));
    sc_trace(tf, OP2_SE, GET_NAME(OP2_SE));
    sc_trace(tf, RES_RX0, GET_NAME(RES_RX0));
    sc_trace(tf, x02x1_din_sx0, GET_NAME(x02x1_din_sx0));
    sc_trace(tf, x02x1_dout_sx0, GET_NAME(x02x1_dout_sx0));

    sc_trace(tf, signed_res_sx0, GET_NAME(signed_res_sx0));
    sc_trace(tf, SIGNED_RES_RX0, GET_NAME(SIGNED_RES_RX0));
    sc_trace(tf, SELECT_HIGHER_BITS_RX0, GET_NAME(SELECT_HIGHER_BITS_RX0));
    sc_trace(tf, EXE_CMD_RD, GET_NAME(EXE_CMD_RD));
    sc_trace(tf, select_higher_bits_sx0, GET_NAME(select_higher_bits_sx0));

    for(int i = 0; i < 32; i++)
    {
        std::string icname = "prod_"+std::to_string(i);
        sc_trace(tf, product[i], signal_get_name(product[i].name(), icname.c_str()));
    }
    for(int i = 0; i < 20; i++)
    {
        std::string icname = "prod_s1_"+std::to_string(i);
        sc_trace(tf, product_s1[i], signal_get_name(product_s1[i].name(), icname.c_str()));
    }
    for(int i = 0; i < 14; i++)
    {
        std::string icname = "prod_s2_"+std::to_string(i);
        sc_trace(tf, product_s2[i], signal_get_name(product_s2[i].name(), icname.c_str()));
    }
    for(int i = 0; i < 10; i++)
    {
        std::string icname = "prod_s3_"+std::to_string(i);
        sc_trace(tf, product_s3[i], signal_get_name(product_s3[i].name(), icname.c_str()));
    }
    for(int i = 0; i < 6; i++)
    {
        std::string icname = "prod_s4_"+std::to_string(i);
        sc_trace(tf, product_s4[i], signal_get_name(product_s4[i].name(), icname.c_str()));
    }
    for(int i = 0; i < 4; i++)
    {
        std::string icname = "prod_s5_"+std::to_string(i);
        sc_trace(tf, product_s5[i], signal_get_name(product_s5[i].name(), icname.c_str()));
    }
    fifo_if2dec.trace(tf);
}