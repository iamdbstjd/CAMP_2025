#ifndef SET_H
#define SET_H
#include <stdio.h> 
// ���������� ��ġ ����ü

// Execute �ܰ� ���� ��ȣ
struct EX {
    int RegDst;
    int ALUSrc;
    int ALUOp;
    int jr;
    int jal;
    int branch;
    int zero;
};

// Memory Access �ܰ� ���� ��ȣ
struct M {
    int memoryWrite;
    int memoryRead;
    int PCSrc1;
};

// Write Back �ܰ� ���� ��ȣ
struct WB {
    int memtoReg;
    int PCSrc2; 
    int RegWrite;
    int j_and_l_toReg;
};

struct IFID_ {
    int valid;
    int pc_4;
    int inst;
};

struct IDEX_ {
    struct EX ex;
    struct M m;
    struct WB wb;
    int valid;
    int pc_4;
    int Rdata1;
    int Rdata2;
    int simm;
    int rs;
    int rd;
    int rt;
    int shamt;
    int imm;
    unsigned int jump_target_absolute;
    int original_opcode;
    unsigned int funct;
};


struct EXMEM_ {
    struct M m;
    struct WB wb;
    int valid;
    int pc_4;
    int aluResult;
    int writeData; // sw ��ɾ ���� MEM �ܰ�� ���޵� rt �������� ��
    int regDst;    // ������ �������� ��ȣ
};

struct MEMWB_ {
    struct WB wb;
    int valid;
    int mReadData; 
    int aluResult; 
    int regDst;    // ������ �������� ��ȣ
    int pc_4;      // JAL ��ɾ� �� $ra�� ����� PC+4 ��
};


extern int Reg[32];
extern unsigned int inst;
extern int pc;
extern int memory[0x400000];
extern int BTB[0x400000];
extern int B_Taken[0x400000]; // 1-bit, 2-bit ���� ���� �� PHT ����
extern int GHR;               // ���� �б� �����丮 ��������
extern int BPHT[0x400000];    // ���� �б� �����丮 ���� ���̺�

extern int becond;            // �б� ���� ���
extern int clock_cycle;
extern int sp;                // ���� ������
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

// functions.c
void fetch(void);
void decode(void);
void execute(void);
void memaccess(void);
void wb(void);
void control(int opcode, int funct);
int reg(int reg1_idx, int reg2_idx);
int ALU(int data1, int data2);
int ALUcontrol(int main_ALUOp_from_control_unit); 
int mem(int address, int writeData);
void Latch_Update(void);

// hazard_handling.c 
int FWD_path_rs(int rs, int regDst_ex_mem, int regDst_mem_wb);
int FWD_path_rt(int rt, int regDst_ex_mem, int regDst_mem_wb);
void Flush_All(int target_pc); // JR �÷���
void Flush_IFID(void); // IF/ID ��ġ �÷���
void Flush_IDEX(void); // ID/EX ��ġ �÷���

// prediction.c 
int AlwaysTakenPredict(int current_pc);
int BTFN(int current_pc);
int OneBitBranchPredict(int current_pc);
int TwoBitBranchPredict(int current_pc);
int TwoLevelPredict(int current_ghr, int current_pc); // Gshare ���
int LocalTwoLevelPredict(int current_pc);

// branch_control.c 
void AlwaysNotTakenPredictCheck(int b_address, int b_taken, int bpc);
void AlwaysTakenPredictCheck(int b_address, int b_taken, int bpc);
void BTFNPredictCheck(int b_address, int b_taken, int bpc);
void OneBitBranchPredictCheck(int b_address, int b_taken, int bpc);
void TwoBitBranchPredictCheck(int b_address, int b_taken, int bpc);
void TwoLevelPredictCheck(int b_address, int b_taken, int bpc);    
void LocalTwoLevelPredictCheck(int b_address, int b_taken, int bpc);

#endif 