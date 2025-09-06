#ifndef SET_H
#define SET_H

#include <stdio.h>
#include <string.h>
#include "cache.h" 

struct M_ctrl {
    int memoryRead, memoryWrite, PCSrc1;
};

struct WB_ctrl {
    int RegWrite, memtoReg, j_and_l_toReg;
};


// 파이프라인 레지스터 구조체 정의
struct IFID_ {
    unsigned int inst;
    int pc_4;
    int valid;
};

struct IDEX_ {
    int Rdata1, Rdata2;
    int simm;
    unsigned int imm; 
    int rs, rt, rd, shamt;
    unsigned int funct;
    unsigned int original_opcode;
    unsigned int jump_target_absolute;
    int pc_4;

    struct {
        int RegDst, ALUSrc;
        int ALUOp;
        int branch, jr, jal, zero;
    } ex;

    struct M_ctrl m;
    struct WB_ctrl wb;

    int valid;
};

struct EXMEM_ {
    int aluResult;
    int writeData;
    int regDst;
    int pc_4;

    struct M_ctrl m;
    struct WB_ctrl wb;

    int valid;
};

struct MEMWB_ {
    int mReadData;
    int aluResult;
    int regDst;
    int pc_4;

    struct WB_ctrl wb;

    int valid;
};


// 전역 변수 extern 선언
extern int Reg[32];
extern unsigned int inst;
extern int pc;
extern int memory[0x400000];

extern int BTB[0x400000];
extern int B_Taken[0x400000];
extern int GHR;
extern int BPHT[0x400000];
extern int becond;

extern int clock_cycle;
extern int sp;
extern int predict_correction;
extern int mis_predict;

extern int memoryops;
extern int regops;
extern int jumpinsts;
extern int branchinsts;
extern int numOfInsts;

extern struct IFID_ g_if_id_latch[2];
extern struct IDEX_ g_id_ex_latch[2];
extern struct EXMEM_ g_ex_mem_latch[2];
extern struct MEMWB_ g_mem_wb_latch[2];

// 캐시 관련 전역 변수 선언
extern CacheLine i_cache[I_CACHE_SETS][I_CACHE_WAYS];
extern CacheLine d_cache[D_CACHE_SETS][D_CACHE_WAYS];
extern int pipeline_stall;
extern int i_cache_hits, i_cache_misses;
extern int d_cache_hits, d_cache_misses;

extern int d_cache_fifo_ptr[D_CACHE_SETS];
extern int i_cache_fifo_ptr[I_CACHE_SETS];

extern int d_cache_cold_misses, d_cache_conflict_misses;
extern int i_cache_cold_misses, i_cache_conflict_misses;


// 함수 프로토타입 선언
void control(int, int);
int reg(int, int);
int ALUcontrol(int);
int ALU(int, int);
void fetch();
void decode();
void execute();
void memaccess();
void wb();
void Latch_Update();
void Flush_All(int);
void Flush_IFID();
void Flush_IDEX();

void AlwaysNotTakenPredictCheck(int, int, int);
void AlwaysTakenPredictCheck(int, int, int);
void BTFNPredictCheck(int, int, int);
void OneBitBranchPredictCheck(int, int, int);
void TwoBitBranchPredictCheck(int, int, int);
void TwoLevelPredictCheck(int, int, int);
void LocalTwoLevelPredictCheck(int, int, int);

// 분기 예측 관련 함수 
int AlwaysTakenPredict(int pc);
int BTFN(int pc);
int OneBitBranchPredict(int pc);
int TwoBitBranchPredict(int pc);
int TwoLevelPredict(int GHR, int pc);
int LocalTwoLevelPredict(int pc);


#endif // SET_H