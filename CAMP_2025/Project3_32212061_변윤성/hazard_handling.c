#include "set.h" 

int FWD_path_rs(int rs, int regDst_ex_mem, int regDst_mem_wb) { 
    // EX/MEM ��ġ Ȯ��
    if (g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && regDst_ex_mem != 0 && (regDst_ex_mem == rs)) {
        return 1; // EX/MEM Fwd
    }
    // MEM/WB ��ġ Ȯ��
    if (g_mem_wb_latch[1].valid && g_mem_wb_latch[1].wb.RegWrite && regDst_mem_wb != 0 && (regDst_mem_wb == rs)) {
        return 2; // MEM/WB Fwd
    }
    return 0; 
}

int FWD_path_rt(int rt, int regDst_ex_mem, int regDst_mem_wb) { 
    // EX/MEM ��ġ Ȯ��
    if (g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && regDst_ex_mem != 0 && (regDst_ex_mem == rt)) {
        return 1; // EX/MEM Fwd
    }
    // MEM/WB ��ġ Ȯ��
    if (g_mem_wb_latch[1].valid && g_mem_wb_latch[1].wb.RegWrite && regDst_mem_wb != 0 && (regDst_mem_wb == rt)) {
        return 2; // MEM/WB Fwd
    }
    return 0; // Fwd ����
}

void Flush_All(int target_pc) { // ��ü �÷��� (IF, ID)
    pc = target_pc; 

    g_if_id_latch[0].valid = 0; 
    g_if_id_latch[1].valid = 0;
    g_id_ex_latch[0].valid = 0; 
}

void Flush_IFID() { 
    g_if_id_latch[0].valid = 0;
    g_if_id_latch[1].valid = 0;
}

void Flush_IDEX() { 
    g_id_ex_latch[0].valid = 0;
    g_id_ex_latch[1].valid = 0;
}