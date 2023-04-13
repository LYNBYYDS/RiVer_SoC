#include "pred_branch_cache.h"

void pred_branch_cache::pred_check() {
    // To pass all the line of cache
    bool miss = true;                                                               // local value MISS/HIT
    sc_uint<2> replace_index;                                                       // local value last index with low priority
    for(int i = 0 ; i < PRED_BRANCH_CACHE_SIZE ; i++)
    {
        if (present[i].read() == 1)                                                             // if the cache bit of present is valide 
        {
            if (PRED_BRANCH_CHECK_ADR_IN_SI.read() == branch_inst_adr[i])           // if the branch instruction is the same as the stored branch instruction
            {// if is the same one 
                PRED_BRANCH_TARGET_ADR_OUT_SP.write(branch_target_adr[i]);      // target address
                PRED_BRANCH_LRU_OUT_SP.write(lru[i]);                               // LRU
                PRED_BRANCH_CPT_OUT_SP.write(branch_counter[i]);               // counter
                PRED_BRANCH_PNT_OUT_SP.write(i);                                    // write the pointer(index) to tell which case will be change in the exe stage
                miss = false;
            }
            if (lru[i].read() == inverse_lru)                                              // this case is with low priority
            {
                replace_index = i;                                                  // store the index for future use
            }
        }
    }
    if (miss)                                                                       // instruction not stocked in the cache
    {
        // for the CPT and LRU we won't use the output PRED_BRANCH_CPT_OUT_SP and PRED_BRANCH_LRU_OUT_SP but need give a pointer to write
        PRED_BRANCH_PNT_OUT_SP.write(replace_index);                                // pointer to the last cache case with low priority
    }
    // after go over all the case of the cache 
    PRED_BRANCH_MISS_OUT_SP.write(miss);
}

void pred_branch_cache::pred_write() {

    // Starting RESET :
    for (int i = 0; i < PRED_BRANCH_CACHE_SIZE; i++) 
    {
        present[i] = 0;                                                             // reset present bit
        lru[i] = 0;                                                                 // reset LRU bit 
    }

    p_nb = 0;                                                                       // reset number of case used 
    inverse_lru = 0;                                                                // reset LRU LRU = 0 means all low priority

    wait(1);
 
    while (1) 
    {
        if ((PRED_BRANCH_CMD_IN_SE.read()) != 0)                                                                      // is a branch instruction write or update
        {
            present[PRED_BRANCH_PNT_IN_SE.read()] = 1;                                                          // no matter write or update rewrite the present bit to 1

            if (PRED_BRANCH_CMD_IN_SE.read() == 1)                                                              // write situation the instruction is not in the cache
            {
                // update all the local signal with the input
                branch_inst_adr[PRED_BRANCH_PNT_IN_SE.read()].write(PRED_BRANCH_WRITE_ADR_IN_SI.read());
                branch_target_adr[PRED_BRANCH_PNT_IN_SE.read()].write(PRED_BRANCH_TARGET_ADR_IN_SE.read());
                branch_counter[PRED_BRANCH_PNT_IN_SE.read()].write(PRED_BRANCH_CPT_IN_SE.read());          // PRED_BRANCH_CPT_IN_SE is calculer outside the module depends on the brach success or not
                // lru, inverse_lru and p_nb need to test if the cache is full or not
                
                if ((p_nb.read() == 3 && inverse_lru == 0) || (p_nb.read() == 1 && inverse_lru == 1))                         // this is the last case that with low priority and for replace it should choose a case with low priority
                {
                    inverse_lru = 1 - inverse_lru;                                                              // change the inverse_lru but dont change the p_nb
                }
                else                                                                                            // if is not the last low priority case need to change p_nb and lru
                {
                    p_nb = p_nb.read() + 1;
                    lru[PRED_BRANCH_PNT_IN_SE.read()] = 1 - inverse_lru;
                }
            }
            else if (PRED_BRANCH_CMD_IN_SE.read() == 2)                                                         // update situation the instruction is in the cache
            {
                // update all the local signal with the input
                // branch instruction adresse and branch target adresse will not change 
                branch_counter[PRED_BRANCH_PNT_IN_SE.read()].write(PRED_BRANCH_CPT_IN_SE.read());          // PRED_BRANCH_CPT_IN_SE is calculer outside the module depends on the brach success or not
                if (lru[PRED_BRANCH_PNT_IN_SE.read()] == inverse_lru)                                           // case low priority so need to change lru, p_nb and inverse_lru on condition of number of case in high priority
                {
                    if ((p_nb.read() == 3 && inverse_lru == 0) || (p_nb.read() == 1 && inverse_lru == 1))                     // this is the last case that with low priority need to change inverse_lru
                    {
                        inverse_lru = 1 - inverse_lru;
                    }
                    else                                                                                        // this is not the last case that with low priority
                    {
                        p_nb = p_nb.read() + 1;
                        lru[PRED_BRANCH_PNT_IN_SE.read()] = 1 - inverse_lru;
                    }
                }
                // if the case is in high priority we dont change lru, p_nb or inverse_lru

                // inverse_lru and p_nb don't need to be change
            }
        }

        wait(1);
    }
}

void pred_branch_cache::trace(sc_trace_file* tf) {
    sc_trace(tf, PRED_BRANCH_CHECK_ADR_IN_SI, GET_NAME(PRED_BRANCH_CHECK_ADR_IN_SI));
    sc_trace(tf, PRED_BRANCH_CMD_IN_SE, GET_NAME(PRED_BRANCH_CMD_IN_SE));
    sc_trace(tf, PRED_BRANCH_WRITE_ADR_IN_SI, GET_NAME(PRED_BRANCH_WRITE_ADR_IN_SI));
    sc_trace(tf, PRED_BRANCH_TARGET_ADR_IN_SE, GET_NAME(PRED_BRANCH_TARGET_ADR_IN_SE));
    sc_trace(tf, PRED_BRANCH_CPT_IN_SE, GET_NAME(PRED_BRANCH_CPT_IN_SE));
    sc_trace(tf, PRED_BRANCH_LRU_IN_SE, GET_NAME(PRED_BRANCH_LRU_IN_SE));
    sc_trace(tf, PRED_BRANCH_PNT_IN_SE, GET_NAME(PRED_BRANCH_PNT_IN_SE));
    sc_trace(tf, PRED_BRANCH_MISS_OUT_SP, GET_NAME(PRED_BRANCH_MISS_OUT_SP));
    sc_trace(tf, PRED_BRANCH_CPT_OUT_SP, GET_NAME(PRED_BRANCH_CPT_OUT_SP));
    sc_trace(tf, PRED_BRANCH_LRU_OUT_SP, GET_NAME(PRED_BRANCH_LRU_OUT_SP));
    sc_trace(tf, PRED_BRANCH_PNT_OUT_SP, GET_NAME(PRED_BRANCH_PNT_OUT_SP));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET_N, GET_NAME(RESET_N));
    sc_trace(tf, inverse_lru, GET_NAME(inverse_lru));
    sc_trace(tf, p_nb, GET_NAME(p_nb));


    for (int x = 0; x < PRED_BRANCH_CACHE_SIZE; x++) {
        //std::string cachename = "PRED_BRANCH_CACHE_";
        //cachename += std::to_string(i);
        sc_trace(tf, present[x], signal_get_name(present[x].name(), "present"));
        sc_trace(tf, branch_inst_adr[x], signal_get_name(branch_inst_adr[x].name(), "branch_inst_adr"));
        sc_trace(tf, branch_target_adr[x], signal_get_name(branch_target_adr[x].name(), "branch_target_adr"));
        sc_trace(tf, branch_counter[x], signal_get_name(branch_counter[x].name(), "branch_counter"));
        sc_trace(tf, lru[x], signal_get_name(lru[x].name(), "lru"));
    }


}