#include <stdio.h>
#include <string.h> 
#include "set.h"    

int main(int argc, char* argv[]) {
    char* input_file = "simple.bin"; 
    FILE* fp;
    int ret;
    int fin = 0; 

    // 명령어 메모리 로딩
    if (argc == 2) { 
        input_file = argv[1];
    }

    fp = fopen(input_file, "rb"); 
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    int index = 0;
    while (1) {
        unsigned char in[4];
        unsigned int instruction_word;

        for (int i = 0; i < 4; i++) { 
            ret = fread(&in[i], 1, 1, fp);
            if (ret != 1) {
                fin = 1; 
                break;
            }
        }
        if (fin == 1) break;

        // 바이트 순서 (엔디안) 맞게 조합 (MIPS: Big Endian)
        instruction_word = ((unsigned int)in[0] << 24) |
            ((unsigned int)in[1] << 16) |
            ((unsigned int)in[2] << 8) |
            ((unsigned int)in[3]);

        memory[index] = instruction_word; 
        index++;
        if (index >= (sizeof(memory) / sizeof(memory[0]))) { // 메모리 용량 초과
            printf("Warning: Instruction memory capacity exceeded.\n");
            break;
        }
    }
    fclose(fp);

    // 상태 초기화
    Reg[31] = -1;            
    Reg[29] = 0x1000000;     
    pc = 0;                  
    sp = Reg[29];            

    // 래치 초기화 (valid=0)
    memset(g_if_id_latch, 0, sizeof(g_if_id_latch));
    memset(g_id_ex_latch, 0, sizeof(g_id_ex_latch));
    memset(g_ex_mem_latch, 0, sizeof(g_ex_mem_latch));
    memset(g_mem_wb_latch, 0, sizeof(g_mem_wb_latch));

    // 분기 예측기 초기화
    GHR = 0;

    // 통계 변수 초기화
    clock_cycle = 0;
    predict_correction = 0;
    mis_predict = 0;
    memoryops = 0;
    regops = 0;
    jumpinsts = 0;
    branchinsts = 0;
    numOfInsts = 0;



    printf("Starting simulation...\n");
    while (1) {
        // 파이프라인 역순 실행 (WB -> MEM -> EX -> ID -> IF)
        wb();
        memaccess();

        if (pc == -1) { // 종료 조건
            printf("Simulation halted. PC reached -1.\n");
            break;
        }

        fetch();
        decode();
        execute();

        Latch_Update(); // 래치 업데이트

        clock_cycle++;

        if (clock_cycle > 200000000) { // 무한 루프 방지
            printf("Error: Exceeded maximum clock cycles. Halting.\n");
            break;
        }
    }

    printf("*********************************************\n");
    printf("Final PC: 0x%x\n", pc);
    printf("Total Clock Cycles: %d\n", clock_cycle);
    printf("Result -> R[2]: %d (0x%x)\n", Reg[2], Reg[2]);
    printf("Number of instructions executed (approx.): %d\n", numOfInsts - mis_predict); // 오예측 제외 추정치
    printf("Number of memory access operations: %d\n", memoryops);
    printf("Number of register read operations: %d\n", regops);
    printf("Number of branch instructions encountered: %d\n", branchinsts);
    printf("Number of jump instructions (j, jal, jr): %d\n", jumpinsts);
    printf("Branch predict corrections: %d\n", predict_correction);
    printf("Branch mispredictions: %d\n", mis_predict);
    if ((predict_correction + mis_predict) > 0) {
        printf("Branch prediction accuracy: %.2f%%\n", (double)predict_correction * 100.0 / (predict_correction + mis_predict));
    }
    else {
        printf("No branch predictions made.\n");
    }
    printf("*********************************************\n");

    return 0;
}