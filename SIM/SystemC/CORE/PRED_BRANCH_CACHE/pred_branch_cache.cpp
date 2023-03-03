#include "pred_cache.h"

void pred_branch_cache::pred_check() {
    // To pass all the line of cache
    bool hit = false;
    for(int i = 0 ; i < PRED_BRANCH_CACHE_SIZE ; i++)
    {
        if (pred_branch_cache[i][0])   // if the cache bit of present is valide 
        {
            if (PRED_ADR_BRANCH_IN_SD.read() == pred_branch_cache[i].range(32, 1))  // if the branch instruction is the same as the stored branch instruction
            {// if is the same one 
                PRED_ADR_AIM_OUT_SP.write(pred_branch_cache[i].range(64, 33));// change output port of the aim address
                PRED_COUNTER_OUT_SP.write(pred_branch_cache[i].range(66, 65));// change output port of the aim address
                hit = true; // hit not miss
            }
        }
    }
}

void pred_branch_cache::pred_write() {

    // Starting RESET :
    for (int i = 0; i < PRED_BRANCH_CACHE_SIZE; i++) 
    {
        pred_branch_cache[i] = 0;        // all line of the cache set to unpresent
        pred_branch_cache_used = 0;         // 0 line of cache used
    }
    bool found_empty_line = false;
    wait(1);




    while (1) 
    {
        bool found = false;
        int index = 0;
        
        // pass all the line used try to find the branch instruction
        for (int i = 0; i < pred_branch_cache_used; i++) 
        {
            if (PRED_ADR_BRANCH_IN_SD.read() == pred_branch_cache[i].range(32, 1))  // if the branch instruction is found
            {
                found = true; // the branch instruction exist in the table 
                index = i; // stock the index 

            }

        }

        if(found) // if the branch instruction info exist in the table 
        {
            if (PRED_ISSUCESS_IN_SD.read() == 1)// if the branch success
            {
                // add 1 to the counter of this branch instruction but if is 11 we dont change 
                pred_branch_cache[index].range(66, 65) = pred_branch_cache[index].range(66, 65) == 0b11? 0b11 : red_branch_cache[index].range(66, 65)+1;
                
            }
            else // the branch instruction not success
            {
                // minus 1 to the counter of this branch instruction  
                pred_branch_cache[index].range(66, 65) = pred_branch_cache[index].range(66, 65) == 0b00? 0b00 : red_branch_cache[index].range(66, 65)-1;
                // but if is 00 we delete this line : bit the present set to 0
                red_branch_cache[index][0] = 0;
            }
            // minus 1 to the LRU for all the LRU bigger than this branch instruction's LRU
            for (int j = 0; j < pred_branch_cache_used.read(); j++) 
            {
                if (j != index && pred_branch_cache[j].range(68, 67) > pred_branch_cache[index].range(68, 67))
                {
                    pred_branch_cache[j].range(68, 67) = pred_branch_cache[j].range(68, 67) -1;
                }
            }
            // change the LRU of this branch instruction to pred_branch_cache_used -1
            pred_branch_cache[i].range(68, 67) = pred_branch_cache_used.read() -1;
        }
        else // if the branch instruction is not found
        {
            // if the branch insrucition is success
            if (PRED_ISSUCESS_IN_SD.read() == 1)
            {
                if (pred_branch_cache_used.read() < PRED_BRANCH_CACHE_SIZE) // if the table is not full
                {
                    // pass the table find the first empty line
                    for (int i = 0; i < pred_branch_cache_used.read(); i++) 
                    {
                        if (pred_branch_cache[i][0] == 0)           // found the first empty line with index i
                        {
                            pred_branch_cache[i][0] = 1;            // set present bit to 1
                            pred_branch_cache[i].range(32, 1) = PRED_ADR_BRANCH_IN_SD; // wrte branch instruction address
                            pred_branch_cache[i].range(64, 33) = PRED_ADR_AIM_IN_SD;    // write branch instruction aim address
                            pred_branch_cache[i].range(66, 65) = 1; // set the set counter of success to 1
                            pred_branch_cache[i].range(68, 67) = pred_branch_cache_used.read();
                            break;
                        }

                    }
                    // add 1 to the used line number
                    pred_branch_cache_used = pred_branch_cache_used.read()+1;
                }
                else // the table is full
                {
                    int found_counter_0 = 0;
                    // pass the table find if there is a branch instruction with a counter == 0
                    for (int i = 0; i < pred_branch_cache_used.read(); i++) 
                    {
                        if (pred_branch_cache[i].range(66, 65) == 0)// if found with index i
                        {
                            pred_branch_cache[i][0] = 1;            // set present bit to 1
                            pred_branch_cache[i].range(32, 1) = PRED_ADR_BRANCH_IN_SD; // wrte branch instruction address
                            pred_branch_cache[i].range(64, 33) = PRED_ADR_AIM_IN_SD;    // write branch instruction aim address
                            pred_branch_cache[i].range(66, 65) = 1; // set the set counter of success to 1
                            
                            // minus 1 to the LRU for all the LRU bigger than this branch instruction's LRU
                            for (int j = 0; j < pred_branch_cache_used.read(); j++) 
                            {
                                if (j != index && pred_branch_cache[j].range(68, 67) > pred_branch_cache[index].range(68, 67))
                                {
                                    pred_branch_cache[j].range(68, 67) = pred_branch_cache[j].range(68, 67) -1;
                                }
                            }
                            // change the LRU of this branch instruction to pred_branch_cache_used -1
                            pred_branch_cache[i].range(68, 67) = pred_branch_cache_used.read() -1;
                            found_counter_0 = 1;
                            break;
                        }
                    }
                    if(!found_counter_0) // if no one with counter == 0
                    {
                        // find the smallest LRU which is 0
                        for (int i = 0; i < pred_branch_cache_used.read(); i++) 
                        {
                            if (pred_branch_cache[i].range(68, 67) == 0)
                            {
                                pred_branch_cache[i][0] = 1;            // set present bit to 1
                                pred_branch_cache[i].range(32, 1) = PRED_ADR_BRANCH_IN_SD; // wrte branch instruction address
                                pred_branch_cache[i].range(64, 33) = PRED_ADR_AIM_IN_SD;    // write branch instruction aim address
                                pred_branch_cache[i].range(66, 65) = 1; // set the set counter of success to 1
                                
                                // minus 1 to the LRU for all the LRU bigger than this branch instruction's LRU
                                for (int j = 0; j < pred_branch_cache_used.read(); j++) 
                                {
                                    if (j != index && pred_branch_cache[j].range(68, 67) > pred_branch_cache[index].range(68, 67))
                                    {
                                        pred_branch_cache[j].range(68, 67) = pred_branch_cache[j].range(68, 67) -1;
                                    }
                                }
                                // change the LRU of this branch instruction to pred_branch_cache_used -1
                                pred_branch_cache[i].range(68, 67) = pred_branch_cache_used.read() -1;
                                break;
                            }
                        }
                    }
                }

                
            }
        }


        wait(1);
    }
}

void pred_branch_cache::trace(sc_trace_file* tf) {
    sc_trace(tf, PRED_ADR_BRANCH_IN_SD, GET_NAME(PRED_ADR_BRANCH_IN_SD));
    sc_trace(tf, PRED_ADR_AIM_IN_SD, GET_NAME(PRED_ADR_AIM_IN_SD));
    sc_trace(tf, PRED_ISSUCESS_IN_SD, GET_NAME(PRED_ISSUCESS_IN_SD));
    sc_trace(tf, PRED_LRU_IN_SD, GET_NAME(PRED_LRU_IN_SD));
    sc_trace(tf, PRED_ADR_AIM_OUT_SP, GET_NAME(PRED_ADR_AIM_OUT_SP));
    sc_trace(tf, PRED_COUNTER_OUT_SP, GET_NAME(PRED_COUNTER_OUT_SP));
    sc_trace(tf, PRED_LRU_OUT_SP, GET_NAME(PRED_LRU_OUT_SP));
    sc_trace(tf, CLK, GET_NAME(CLK));
    sc_trace(tf, RESET_N, GET_NAME(RESET_N));
    sc_trace(tf, pred_branch_cache_used, GET_NAME(pred_branch_cache_used));


    for (int i = 0; i < PRED_BRANCH_CACHE_SIZE; i++) {
        std::string cachename = "PRED_BRANCH_CACHE_";
        cachename += std::to_string(i);
        sc_trace(tf, pred_branch_cache[i], signal_get_name(pred_branch_cache[i].name(), cachename.c_str()));
    }


}