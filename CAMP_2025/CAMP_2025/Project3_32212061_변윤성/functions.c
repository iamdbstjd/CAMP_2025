#include "set.h"
#include <string.h>

// Control Unit 
void control(int opcode, int funct) { // 제어 신호 생성
	memset(&g_id_ex_latch[0].ex, 0, sizeof(g_id_ex_latch[0].ex));
	memset(&g_id_ex_latch[0].m, 0, sizeof(g_id_ex_latch[0].m));
	memset(&g_id_ex_latch[0].wb, 0, sizeof(g_id_ex_latch[0].wb));
	g_id_ex_latch[0].ex.ALUOp = opcode;

	if (opcode == 0) { // R-type
		g_id_ex_latch[0].ex.RegDst = 1;
		g_id_ex_latch[0].wb.RegWrite = 1;
		g_id_ex_latch[0].ex.ALUOp = 0;
		if (funct == 0x08) { // jr
			g_id_ex_latch[0].ex.jr = 1;
			g_id_ex_latch[0].wb.RegWrite = 0;
		}
	}
	else { // I-type or J-type
		if (opcode != 0x4 && opcode != 0x5 && opcode != 0x2 && opcode != 0x3) { // !Branch, !J, !JAL
			g_id_ex_latch[0].ex.ALUSrc = 1;
		}

		if (opcode == 0x4 || opcode == 0x5) { // beq, bne
			g_id_ex_latch[0].ex.branch = 1;
			g_id_ex_latch[0].ex.ALUSrc = 0;
		}
		else if (opcode == 0x23) { // lw
			g_id_ex_latch[0].m.memoryRead = 1;
			g_id_ex_latch[0].wb.memtoReg = 1;
			g_id_ex_latch[0].wb.RegWrite = 1;
		}
		else if (opcode == 0x2b) { // sw
			g_id_ex_latch[0].m.memoryWrite = 1;
		}
		else if (opcode == 0x2 || opcode == 0x3) { // j, jal
			g_id_ex_latch[0].m.PCSrc1 = 1;
			g_id_ex_latch[0].ex.ALUSrc = 0;
			if (opcode == 0x3) { // jal
				g_id_ex_latch[0].ex.jal = 1;
				g_id_ex_latch[0].ex.RegDst = 2; // $ra Dest
				g_id_ex_latch[0].wb.RegWrite = 1;
				g_id_ex_latch[0].wb.j_and_l_toReg = 1; // PC+4 to Reg
			}
		}
		else if (opcode == 0x8 || opcode == 0x9 || opcode == 0xa || // I-type 산술/논리
			opcode == 0xc || opcode == 0xd || opcode == 0xe || opcode == 0xf) {
			g_id_ex_latch[0].wb.RegWrite = 1;
		}

		if (opcode == 0xc || opcode == 0xd || opcode == 0xe || opcode == 0xf) { // Zero-extend 대상
			g_id_ex_latch[0].ex.zero = 1;
		}
	}
}


// Register File Read 
int reg(int reg1_idx, int reg2_idx) {
	regops++;
	g_id_ex_latch[0].Rdata1 = (reg1_idx == 0) ? 0 : Reg[reg1_idx];
	g_id_ex_latch[0].Rdata2 = (reg2_idx == 0) ? 0 : Reg[reg2_idx];
	return 0;
}

// ALU Control Unit 
int ALUcontrol(int main_ALUOp_from_control_unit) {
	if (main_ALUOp_from_control_unit == 0x0) { // R-type
		return g_id_ex_latch[1].funct;
	}
	else { // I-type 등
		return main_ALUOp_from_control_unit; // Opcode로 연산 결정
	}
}

// Arithmetic Logic Unit (ALU) 
int ALU(int data1, int data2) { // ALU 연산
	unsigned int result = 0;
	int operation_to_perform = ALUcontrol(g_id_ex_latch[1].ex.ALUOp);

	if (g_id_ex_latch[1].ex.ALUOp == 0x0) { // R-type
		switch (operation_to_perform) {
		case 0x20: result = data1 + data2; break; // add
		case 0x21: result = data1 + data2; break; // addu
		case 0x22: result = data1 - data2; break; // sub
		case 0x23: result = data1 - data2; break; // subu
		case 0x24: result = data1 & data2; break; // and
		case 0x25: result = data1 | data2; break; // or
		case 0x26: result = data1 ^ data2; break; // xor
		case 0x27: result = ~(data1 | data2); break; // nor
		case 0x2a: result = (data1 < data2) ? 1 : 0; break; // slt
		case 0x00: result = data2 << g_id_ex_latch[1].shamt; break; // sll
		case 0x02: result = (unsigned int)data2 >> g_id_ex_latch[1].shamt; break; // srl
		case 0x03: result = data2 >> g_id_ex_latch[1].shamt; break; // sra
		case 0x08: break; // jr
		default: result = 0; break;
		}
	}
	else { // I-type 등
		switch (g_id_ex_latch[1].ex.ALUOp) {
		case 0x8: result = data1 + data2; break; // addi
		case 0x9: result = data1 + data2; break; // addiu
		case 0xc: result = data1 & data2; break; // andi
		case 0xd: result = data1 | data2; break; // ori
		case 0xe: result = data1 ^ data2; break; // xori
		case 0xa: result = (data1 < data2) ? 1 : 0; break; // slti
		case 0xf: result = data2; break; // lui
		case 0x23: result = data1 + data2; break; // lw (주소)
		case 0x2b: result = data1 + data2; break; // sw (주소)
		case 0x4: if (data1 == data2) becond = 1; else becond = 0; branchinsts++; break; // beq
		case 0x5: if (data1 != data2) becond = 1; else becond = 0; branchinsts++; break; // bne
		case 0x2: case 0x3: break; // j, jal
		default: result = 0; break;
		}
	}
	g_ex_mem_latch[0].aluResult = result; // EX/MEM 저장
	return 0;
}

// Instruction Fetch (IF) 단계
void fetch() { // IF
	if (pc == -1) {
		return;
	}

	if ((pc / 4) < 0 || (pc / 4) >= (sizeof(memory) / sizeof(memory[0]))) {
		// printf("[FETCH] Cycle: %d, Error: PC 0x%x out of instruction memory bounds.\n", clock_cycle, pc);
		pc = -1;
		g_if_id_latch[0].valid = 0;
		return;
	}

	inst = memory[pc / 4]; // 명령어 읽기

	g_if_id_latch[0].inst = inst; // IF/ID 저장
	numOfInsts++;
	g_if_id_latch[0].pc_4 = pc + 4; // PC+4 저장

	// 분기 예측 (컴파일 시 매크로 정의에 따라 선택)
	int predicted_taken;

#if defined(PREDICTOR_ALWAYS_NOT_TAKEN)
	predicted_taken = 0; // Always Not Taken은 항상 0 (Not Taken)
#elif defined(PREDICTOR_ALWAYS_TAKEN)
	predicted_taken = AlwaysTakenPredict(pc);
#elif defined(PREDICTOR_BTFN)
	predicted_taken = BTFN(pc);
#elif defined(PREDICTOR_ONE_BIT)
	predicted_taken = OneBitBranchPredict(pc);
#elif defined(PREDICTOR_TWO_BIT)
	predicted_taken = TwoBitBranchPredict(pc);
#elif defined(PREDICTOR_GLOBAL_TWO_LEVEL) // Gshare
	predicted_taken = TwoLevelPredict(GHR, pc); // Gshare는 GHR 인자 필요
#elif defined(PREDICTOR_LOCAL_TWO_LEVEL)
	predicted_taken = LocalTwoLevelPredict(pc);
#else // 기본
	predicted_taken = BTFN(pc);
#endif

	int next_pc;

	if (predicted_taken) { // 예측: Taken
		int btb_idx = pc % (sizeof(BTB) / sizeof(BTB[0]));
		if (BTB[btb_idx] != 0) { 
			next_pc = BTB[btb_idx]; 
		}
		else { 
			next_pc = pc + 4; 
		}
	}
	else { // 예측: Not Taken
		next_pc = pc + 4; 
	}
	pc = next_pc; 
	g_if_id_latch[0].valid = 1; // IF/ID 유효
}

// Instruction Decode (ID) 단계 
void decode() { // ID
	if (g_if_id_latch[1].valid == 0) {
		g_id_ex_latch[0].valid = 0;
		return;
	}
	inst = g_if_id_latch[1].inst;

	int opcode = (inst >> 26) & 0x3f; // Opcode
	int rs = (inst >> 21) & 0x1f;     // rs
	int rt = (inst >> 16) & 0x1f;     // rt
	int rd = (inst >> 11) & 0x1f;     // rd
	int shamt = (inst >> 6) & 0x1f;   // shamt
	unsigned int funct = inst & 0x3f; // funct
	unsigned int imm = inst & 0xffff; // imm (16-bit)
	unsigned int jtarget = 0;         // J-target 필드
	unsigned int absolute_jump_addr = 0; // J 점프 절대주소

	if (opcode == 0x2 || opcode == 0x3) { // j, jal
		jtarget = inst & 0x3ffffff;
		jumpinsts++;
		absolute_jump_addr = (g_if_id_latch[1].pc_4 & 0xF0000000) | (jtarget << 2); // 절대주소 계산
	}

	control(opcode, funct); // 제어 신호 생성
	reg(rs, rt);       

	if (g_id_ex_latch[0].ex.zero) { // Zero-extend
		g_id_ex_latch[0].simm = imm & 0x0000FFFF;
	}
	else { // Sign-extend
		if ((imm >> 15) & 0x1) {
			g_id_ex_latch[0].simm = (0xFFFF << 16) | imm;
		}
		else {
			g_id_ex_latch[0].simm = imm & 0x0000FFFF;
		}
	}

	g_id_ex_latch[0].pc_4 = g_if_id_latch[1].pc_4;
	g_id_ex_latch[0].rs = rs;
	g_id_ex_latch[0].rt = rt;
	g_id_ex_latch[0].rd = rd;
	g_id_ex_latch[0].shamt = shamt;
	g_id_ex_latch[0].imm = imm;
	g_id_ex_latch[0].original_opcode = opcode; // 원본 Opcode 저장
	g_id_ex_latch[0].funct = funct;            // funct 저장
	g_id_ex_latch[0].jump_target_absolute = absolute_jump_addr;
	g_id_ex_latch[0].valid = 1; // ID/EX 유효
}


// Execute (EX) 단계
void execute() { // EX
	if (g_id_ex_latch[1].valid == 0) {
		g_ex_mem_latch[0].valid = 0;
		return;
	}

	int current_pc_at_if = g_id_ex_latch[1].pc_4 - 4;
	int original_opcode = g_id_ex_latch[1].original_opcode;
	// int funct_ex = g_id_ex_latch[1].funct; // 디버깅 
	int rs_ex = g_id_ex_latch[1].rs;
	int rt_ex = g_id_ex_latch[1].rt;
	int rd_ex = g_id_ex_latch[1].rd;
	int simm_ex = g_id_ex_latch[1].simm;
	int imm_original_ex = g_id_ex_latch[1].imm & 0xFFFF;
	unsigned int jump_target_addr_ex = g_id_ex_latch[1].jump_target_absolute;

	int data1_operand_before_fwd = g_id_ex_latch[1].Rdata1; // 디버깅용
	int data2_operand_before_fwd = g_id_ex_latch[1].Rdata2; // 디버깅용
	int data1_operand = g_id_ex_latch[1].Rdata1; 
	int data2_operand = g_id_ex_latch[1].Rdata2; 

	// Forward rs
	if (g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && g_ex_mem_latch[1].regDst != 0 && g_ex_mem_latch[1].regDst == rs_ex) {
		data1_operand = g_ex_mem_latch[1].aluResult;
	}
	else if (g_mem_wb_latch[1].valid && g_mem_wb_latch[1].wb.RegWrite && g_mem_wb_latch[1].regDst != 0 && g_mem_wb_latch[1].regDst == rs_ex) {
		if (!(g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && g_ex_mem_latch[1].regDst != 0 && g_ex_mem_latch[1].regDst == rs_ex)) { // EX/MEM 우선
			if (g_mem_wb_latch[1].wb.memtoReg) data1_operand = g_mem_wb_latch[1].mReadData;
			else if (g_mem_wb_latch[1].wb.j_and_l_toReg) data1_operand = g_mem_wb_latch[1].pc_4;
			else data1_operand = g_mem_wb_latch[1].aluResult;
		}
	}

	// Forward rt
	int rt_needs_forwarding = (!g_id_ex_latch[1].ex.ALUSrc && original_opcode == 0 && !g_id_ex_latch[1].ex.jr) ||
		(original_opcode == 0x2b) || (g_id_ex_latch[1].ex.branch);
	if (rt_needs_forwarding) {
		if (g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && g_ex_mem_latch[1].regDst != 0 && g_ex_mem_latch[1].regDst == rt_ex) {
			data2_operand = g_ex_mem_latch[1].aluResult;
		}
		else if (g_mem_wb_latch[1].valid && g_mem_wb_latch[1].wb.RegWrite && g_mem_wb_latch[1].regDst != 0 && g_mem_wb_latch[1].regDst == rt_ex) {
			if (!(g_ex_mem_latch[1].valid && g_ex_mem_latch[1].wb.RegWrite && g_ex_mem_latch[1].regDst != 0 && g_ex_mem_latch[1].regDst == rt_ex)) { // EX/MEM 우선
				if (g_mem_wb_latch[1].wb.memtoReg) data2_operand = g_mem_wb_latch[1].mReadData;
				else if (g_mem_wb_latch[1].wb.j_and_l_toReg) data2_operand = g_mem_wb_latch[1].pc_4;
				else data2_operand = g_mem_wb_latch[1].aluResult;
			}
		}
	}

	// EX/MEM 래치 기본 설정
	g_ex_mem_latch[0].pc_4 = g_id_ex_latch[1].pc_4;
	g_ex_mem_latch[0].m = g_id_ex_latch[1].m;
	g_ex_mem_latch[0].wb = g_id_ex_latch[1].wb;
	g_ex_mem_latch[0].valid = 1;
	g_ex_mem_latch[0].aluResult = 0;
	g_ex_mem_latch[0].writeData = data2_operand;

	// 목적지 레지스터 설정
	if (g_id_ex_latch[1].ex.RegDst == 0) {
		g_ex_mem_latch[0].regDst = rt_ex;
	}
	else if (g_id_ex_latch[1].ex.RegDst == 1) {
		g_ex_mem_latch[0].regDst = rd_ex;
	}
	else if (g_id_ex_latch[1].ex.RegDst == 2) { // JAL
		g_ex_mem_latch[0].regDst = 31; // $ra
	}
	else {
		g_ex_mem_latch[0].regDst = 0;
	}

	// JUMP/JAL/JR 처리
	if (g_id_ex_latch[1].m.PCSrc1 == 1) { // j 또는 jal
		unsigned int target_for_jump = jump_target_addr_ex;
		if (original_opcode == 0x2) { // J
			g_ex_mem_latch[0].valid = 0; 
		}
		Flush_All(target_for_jump); // PC 변경, 플러시
		return;
	}

	if (g_id_ex_latch[1].ex.jr) { // jr
		Flush_All(data1_operand);
		g_ex_mem_latch[0].valid = 0; 
		return;
	}

	// 그 외 명령어 ALU 연산
	int alu_actual_second_operand;
	if (g_id_ex_latch[1].ex.ALUSrc) {
		if (g_id_ex_latch[1].ex.zero) {
			if (original_opcode == 0xf) alu_actual_second_operand = imm_original_ex << 16; // lui
			else alu_actual_second_operand = imm_original_ex & 0x0000FFFF;
		}
		else {
			alu_actual_second_operand = simm_ex;
		}
	}
	else {
		alu_actual_second_operand = data2_operand;
	}

	ALU(data1_operand, alu_actual_second_operand);

	// 분기 명령어 처리 
	if (g_id_ex_latch[1].ex.branch) {
		int target_address = g_id_ex_latch[1].pc_4 + (simm_ex << 2);

#if defined(PREDICTOR_ALWAYS_NOT_TAKEN)
		AlwaysNotTakenPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_ALWAYS_TAKEN)
		AlwaysTakenPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_BTFN)
		BTFNPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_ONE_BIT)
		OneBitBranchPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_TWO_BIT)
		TwoBitBranchPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_GLOBAL_TWO_LEVEL) // Gshare
		TwoLevelPredictCheck(target_address, becond, current_pc_at_if);
#elif defined(PREDICTOR_LOCAL_TWO_LEVEL)
		LocalTwoLevelPredictCheck(target_address, becond, current_pc_at_if);
#else
		BTFNPredictCheck(target_address, becond, current_pc_at_if);
#endif
	}
}


// Memory Access (MEM) - 보조
int mem(int address, int writeData_param) { // Mem 연산
	if (g_ex_mem_latch[1].valid == 0) {
		g_mem_wb_latch[0].valid = 0;
		return 0;
	}

	if (g_ex_mem_latch[1].m.memoryWrite == 1) { // sw
		int mem_idx = address / 4;
		if (address >= 0 && (address % 4 == 0) && mem_idx >= 0 && mem_idx < (sizeof(memory) / sizeof(memory[0]))) {
			memory[mem_idx] = writeData_param;
		}
		memoryops++;
	}
	else if (g_ex_mem_latch[1].m.memoryRead == 1) { // lw
		int mem_idx = address / 4;
		if (address >= 0 && (address % 4 == 0) && mem_idx >= 0 && mem_idx < (sizeof(memory) / sizeof(memory[0]))) {
			g_mem_wb_latch[0].mReadData = memory[mem_idx];
		}
		else {
			g_mem_wb_latch[0].mReadData = 0;
		}
		memoryops++;
	}
	g_mem_wb_latch[0].valid = 1; // MEM/WB 유효
	return 0;
}

// Memory Access (MEM) 단계 
void memaccess() { // MEM
	if (g_ex_mem_latch[1].valid == 0) {
		g_mem_wb_latch[0].valid = 0;
		return;
	}
	mem(g_ex_mem_latch[1].aluResult, g_ex_mem_latch[1].writeData);

	g_mem_wb_latch[0].aluResult = g_ex_mem_latch[1].aluResult;
	g_mem_wb_latch[0].pc_4 = g_ex_mem_latch[1].pc_4;
	g_mem_wb_latch[0].regDst = g_ex_mem_latch[1].regDst;
	g_mem_wb_latch[0].wb = g_ex_mem_latch[1].wb;
}

// Write Back (WB) 단계 
void wb() { // WB
	if (g_mem_wb_latch[1].valid == 0) {
		return;
	}

	if (g_mem_wb_latch[1].wb.RegWrite == 1) {
		if (g_mem_wb_latch[1].regDst != 0) { // $zero 제외
			if (g_mem_wb_latch[1].wb.j_and_l_toReg == 1) { // jal
				Reg[g_mem_wb_latch[1].regDst] = g_mem_wb_latch[1].pc_4; // $ra <= PC+4
			}
			else if (g_mem_wb_latch[1].wb.memtoReg == 1) { // lw
				Reg[g_mem_wb_latch[1].regDst] = g_mem_wb_latch[1].mReadData; // Reg <= Mem
			}
			else { // R-type, I-type (산술/논리)
				Reg[g_mem_wb_latch[1].regDst] = g_mem_wb_latch[1].aluResult; // Reg <= ALU
			}
		}
	}
}

// Latch Update 
void Latch_Update() { // 래치 값 이동
	// IF/ID
	if (g_if_id_latch[0].valid == 1) {
		g_if_id_latch[1] = g_if_id_latch[0];
	}
	else {
		g_if_id_latch[1].valid = 0;
	}
	g_if_id_latch[0].valid = 0;

	// ID/EX
	if (g_id_ex_latch[0].valid == 1) {
		g_id_ex_latch[1] = g_id_ex_latch[0];
	}
	else {
		g_id_ex_latch[1].valid = 0;
	}
	g_id_ex_latch[0].valid = 0;

	// EX/MEM
	if (g_ex_mem_latch[0].valid == 1) {
		g_ex_mem_latch[1] = g_ex_mem_latch[0];
	}
	else {
		g_ex_mem_latch[1].valid = 0;
	}
	g_ex_mem_latch[0].valid = 0;

	// MEM/WB
	if (g_mem_wb_latch[0].valid == 1) {
		g_mem_wb_latch[1] = g_mem_wb_latch[0];
	}
	else {
		g_mem_wb_latch[1].valid = 0;
	}
	g_mem_wb_latch[0].valid = 0;
}