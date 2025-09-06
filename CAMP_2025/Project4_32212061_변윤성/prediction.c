#include "set.h" 

// Always Taken 예측
int AlwaysTakenPredict(int current_pc) { // 예측: Always T
    if (BTB[current_pc % (sizeof(BTB) / sizeof(BTB[0]))] != 0) { 
        return 1; // Taken
    }
    else {
        return 0; // Not Taken 
    }
}

// BTFN (Backward Taken, Forward Not Taken) 예측
int BTFN(int current_pc) { 
    int btb_idx = current_pc % (sizeof(BTB) / sizeof(BTB[0]));
    if (BTB[btb_idx] != 0) { 
        if (BTB[btb_idx] < current_pc) { 
            return 1; // Taken
        }
        return 0; // Forward -> Not Taken
    }
    else {
        return 0; // BTB 미스 -> Not Taken
    }
}

// 1-bit 분기 예측
int OneBitBranchPredict(int current_pc) { // 예측: 1-bit
    int hit = 0;
    int taken = 0;
    int btb_idx = current_pc % (sizeof(BTB) / sizeof(BTB[0]));
    int b_taken_idx = current_pc % (sizeof(B_Taken) / sizeof(B_Taken[0]));

    if (BTB[btb_idx] != 0) { 
        hit = 1;
        if (B_Taken[b_taken_idx] == 1) {
            taken = 1;
        }
        else {
            taken = 0;
        }
    }
    return hit && taken; // BTB 히트 & Taken 예측 시 1
}

// 2-bit 분기 예측
int TwoBitBranchPredict(int current_pc) { // 예측: 2-bit
    int hit = 0;
    int taken = 0;
    int btb_idx = current_pc % (sizeof(BTB) / sizeof(BTB[0]));
    int b_taken_idx = current_pc % (sizeof(B_Taken) / sizeof(B_Taken[0]));

    if (BTB[btb_idx] != 0) { 
        hit = 1;
        switch (B_Taken[b_taken_idx]) {
        case 0: case 1: taken = 0; break; 
        case 2: case 3: taken = 1; break; 
        }
    }
    return hit && taken; // BTB 히트 & Taken 예측 시 1
}

// Gshare (전역 2-level) 분기 예측
int TwoLevelPredict(int current_ghr, int current_pc) { // 예측: Gshare
    int hit = 0;
    int taken = 0;
    int btb_idx = current_pc % (sizeof(BTB) / sizeof(BTB[0]));

    if (BTB[btb_idx] != 0) { 
        hit = 1;
        int pht_index = (current_pc ^ current_ghr) % (sizeof(B_Taken) / sizeof(B_Taken[0])); 

        switch (B_Taken[pht_index]) { // PHT 상태
        case 0: case 1: taken = 0; break; 
        case 2: case 3: taken = 1; break; 
        }
    }
    return hit && taken;
}

// Local 2-level 분기 예측
int LocalTwoLevelPredict(int current_pc) { // 예측: Local 2L
    int hit = 0;
    int taken = 0;
    int btb_idx = current_pc % (sizeof(BTB) / sizeof(BTB[0]));

    if (BTB[btb_idx] != 0) { 
        hit = 1;
        int local_history_pattern = BPHT[current_pc % (sizeof(BPHT) / sizeof(BPHT[0]))]; 
        int pht_index = local_history_pattern % (sizeof(B_Taken) / sizeof(B_Taken[0]));  

        switch (B_Taken[pht_index]) { 
        case 0: case 1: taken = 0; break; 
        case 2: case 3: taken = 1; break; 
        }
    }
    return hit && taken;
}