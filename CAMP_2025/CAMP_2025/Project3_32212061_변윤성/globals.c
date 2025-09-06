#include "set.h" 

int Reg[32]; 
unsigned int inst;
int pc; 
int memory[0x400000]; // 메모리


int BTB[0x400000];      
int B_Taken[0x400000];  
int GHR = 0;            
int BPHT[0x400000];     
int becond = 0;

int clock_cycle = 0; 
int sp; // $sp
int predict_correction = 0; 
int mis_predict = 0;        

// 명령어 종류별 카운터
int memoryops = 0;    
int regops = 0;       
int jumpinsts = 0;    
int branchinsts = 0;  
int numOfInsts = 0;   

// 파이프라인 래치
struct IFID_ g_if_id_latch[2];  // IF/ID
struct IDEX_ g_id_ex_latch[2];  // ID/EX
struct EXMEM_ g_ex_mem_latch[2]; // EX/MEM
struct MEMWB_ g_mem_wb_latch[2]; // MEM/WB