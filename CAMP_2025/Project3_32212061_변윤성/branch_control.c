#include "set.h" 
// Always Not Taken 예측 검사
void AlwaysNotTakenPredictCheck(int b_address, int b_taken, int bpc) { // ANT 검사
    if (!b_taken) { 
        predict_correction += 1;
    }
    else { 
        Flush_IFID();
        Flush_IDEX();
        pc = b_address;
        mis_predict += 1;
    }
}

// Always Taken 예측 검사
void AlwaysTakenPredictCheck(int b_address, int b_taken, int bpc) { // AT 검사
    if (BTB[bpc] != 0) { 
        if (b_taken) { 
            predict_correction += 1;
        }
        else { 
            Flush_IFID();
            Flush_IDEX();
            pc = bpc + 4;
            mis_predict += 1;
        }
    }
    else { 
        if (!b_taken) { 
            predict_correction += 1;
        }
        else { 
            Flush_IFID();
            Flush_IDEX();
            BTB[bpc] = b_address; 
            pc = b_address;
            mis_predict += 1;
        }
    }
}

// BTFN 예측 검사
void BTFNPredictCheck(int b_address, int b_taken, int bpc) { // BTFN 검사
    int predicted_taken = 0;
    if (BTB[bpc] != 0 && BTB[bpc] < bpc) { 
        predicted_taken = 1;
    }

    if (predicted_taken) { 
        if (b_taken) { 
            predict_correction += 1;
        }
        else { 
            Flush_IFID();
            Flush_IDEX();
            pc = bpc + 4;
            mis_predict += 1;
        }
    }
    else { 
        if (!b_taken) { 
            predict_correction += 1;
        }
        else { 
            Flush_IFID();
            Flush_IDEX();
            if (b_address < bpc) { // BTB 업데이트 (실제 뒤로 감)
                BTB[bpc] = b_address;
            }
            pc = b_address;
            mis_predict += 1;
        }
    }
}

// 1-bit 예측 검사 및 상태 업데이트
void OneBitBranchPredictCheck(int b_address, int b_taken, int bpc) { // 1-bit 검사
    int predicted_state = B_Taken[bpc]; 
    int predicted_taken_flag = (predicted_state == 1);

    if (BTB[bpc] != 0) { 
        if (predicted_taken_flag) { 
            if (b_taken) { 
                predict_correction += 1;
            }
            else { 
                Flush_IFID();
                Flush_IDEX();
                B_Taken[bpc] = 0; 
                pc = bpc + 4;
                mis_predict += 1;
            }
        }
        else { 
            if (!b_taken) { 
                predict_correction += 1;
            }
            else { 
                Flush_IFID();
                Flush_IDEX();
                B_Taken[bpc] = 1; 
                pc = b_address;
                mis_predict += 1;
            }
        }
    }
    else { 
        if (!b_taken) { 
            B_Taken[bpc] = 0; 
            predict_correction += 1;
        }
        else { 
            Flush_IFID();
            Flush_IDEX();
            B_Taken[bpc] = 1;   
            BTB[bpc] = b_address; 
            pc = b_address;
            mis_predict += 1;
        }
    }
}

// 2-bit 예측 검사 및 상태 업데이트
void TwoBitBranchPredictCheck(int b_address, int b_taken, int bpc) { // 2-bit 검사
    int current_state = B_Taken[bpc]; 
    int predicted_taken_flag = (current_state >= 2); 

    if (predicted_taken_flag == b_taken) { // 예측 성공
        predict_correction += 1;
        if (b_taken) { 
            if (B_Taken[bpc] < 3) B_Taken[bpc]++; 
        }
        else { 
            if (B_Taken[bpc] > 0) B_Taken[bpc]--; 
        }
    }
    else { // 예측 실패
        mis_predict += 1;
        Flush_IFID();
        Flush_IDEX();
        if (b_taken) { 
            B_Taken[bpc]++; 
            pc = b_address;
        }
        else { 
            B_Taken[bpc]--; 
            pc = bpc + 4;
        }
    }

    if (BTB[bpc] == 0 && b_taken) { 
        BTB[bpc] = b_address;
    }
    if (B_Taken[bpc] < 0) B_Taken[bpc] = 0;
    if (B_Taken[bpc] > 3) B_Taken[bpc] = 3;
}

// Gshare 예측 검사 및 상태 업데이트
void TwoLevelPredictCheck(int b_address, int b_taken, int bpc) { // Gshare 검사
    int pht_index = (bpc ^ GHR) % (sizeof(B_Taken) / sizeof(B_Taken[0])); 
    int current_state = B_Taken[pht_index];
    int predicted_taken_flag = (current_state >= 2);

    if (predicted_taken_flag == b_taken) { // 예측 성공
        predict_correction += 1;
        if (b_taken) {
            if (B_Taken[pht_index] < 3) B_Taken[pht_index]++;
        }
        else {
            if (B_Taken[pht_index] > 0) B_Taken[pht_index]--;
        }
    }
    else { // 예측 실패
        mis_predict += 1;
        Flush_IFID();
        Flush_IDEX();
        if (b_taken) {
            B_Taken[pht_index]++;
            pc = b_address;
        }
        else {
            B_Taken[pht_index]--;
            pc = bpc + 4;
        }
    }

    if (BTB[bpc] == 0 && b_taken) { // BTB 업데이트
        BTB[bpc] = b_address;
    }
    if (B_Taken[pht_index] < 0) B_Taken[pht_index] = 0;
    if (B_Taken[pht_index] > 3) B_Taken[pht_index] = 3;

    GHR = (GHR << 1) | b_taken; // GHR 업데이트
}

// Local 2-level 예측 검사 및 상태 업데이트
void LocalTwoLevelPredictCheck(int b_address, int b_taken, int bpc) { // Local 2L 검사
    int bpht_pc_idx = bpc % (sizeof(BPHT) / sizeof(BPHT[0])); 
    int local_history_pattern = BPHT[bpht_pc_idx];
    int pht_index = local_history_pattern % (sizeof(B_Taken) / sizeof(B_Taken[0])); 

    int current_state = B_Taken[pht_index];
    int predicted_taken_flag = (current_state >= 2);

    if (predicted_taken_flag == b_taken) { // 예측 성공
        predict_correction += 1;
        if (b_taken) {
            if (B_Taken[pht_index] < 3) B_Taken[pht_index]++;
        }
        else {
            if (B_Taken[pht_index] > 0) B_Taken[pht_index]--;
        }
    }
    else { // 예측 실패
        mis_predict += 1;
        Flush_IFID();
        Flush_IDEX();
        if (b_taken) {
            B_Taken[pht_index]++;
            pc = b_address;
        }
        else {
            B_Taken[pht_index]--;
            pc = bpc + 4;
        }
    }
    if (BTB[bpc] == 0 && b_taken) { // BTB 업데이트
        BTB[bpc] = b_address;
    }
    if (B_Taken[pht_index] < 0) B_Taken[pht_index] = 0;
    if (B_Taken[pht_index] > 3) B_Taken[pht_index] = 3;

    BPHT[bpht_pc_idx] = (BPHT[bpht_pc_idx] << 1) | b_taken; // 지역 히스토리 업데이트
}