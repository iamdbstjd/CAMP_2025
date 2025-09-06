#!/usr/bin/env python3
def parse_operand(op, registers):
    if op.startswith("0x"):
        return int(op, 16)
    elif op.startswith("R"):
        if op in registers:
            return registers[op]
        else:
            raise ValueError("레지스터 {}가 존재하지 않습니다.".format(op))
    else:
        raise ValueError("잘못된 피연산자 형식: " + op)

def main():
    registers = {"R" + str(i): 0 for i in range(10)}
    try:
        with open("input.txt", "r") as f:
            instructions = [line.strip() for line in f if line.strip()]
    except Exception as e:
        print("입력 파일 읽기 오류:", e)
        return

    ip = 0
    instruction_count = len(instructions)
    while ip < instruction_count:
        line = instructions[ip]
        parts = line.split()
        if not parts:
            ip += 1
            continue
        opcode = parts[0]
        operand1 = parts[1] if len(parts) > 1 else None
        operand2 = parts[2] if len(parts) > 2 else None
        try:
            if opcode == '+':
                val1 = parse_operand(operand1, registers)
                val2 = parse_operand(operand2, registers)
                result = val1 + val2
                registers["R0"] = result
                print(f"[명령어 {ip+1}]: {line}")
                print(f"R0에 결과 저장: {result} = {val1} + {val2}")
            elif opcode == '-':
                val1 = parse_operand(operand1, registers)
                val2 = parse_operand(operand2, registers)
                result = val1 - val2
                registers["R0"] = result
                print(f"[명령어 {ip+1}]: {line}")
                print(f"R0에 결과 저장: {result} = {val1} - {val2}")
            elif opcode == '*':
                val1 = parse_operand(operand1, registers)
                val2 = parse_operand(operand2, registers)
                result = val1 * val2
                registers["R0"] = result
                print(f"[명령어 {ip+1}]: {line}")
                print(f"R0에 결과 저장: {result} = {val1} * {val2}")
            elif opcode == '/':
                val1 = parse_operand(operand1, registers)
                val2 = parse_operand(operand2, registers)
                if val2 == 0:
                    print(f"[명령어 {ip+1}]: {line} -> 오류: 0으로 나누기")
                else:
                    result = val1 // val2
                    registers["R0"] = result
                    print(f"[명령어 {ip+1}]: {line}")
                    print(f"R0에 결과 저장: {result} = {val1} / {val2}")
            elif opcode == 'M':
                if not operand1.startswith("R"):
                    print(f"[명령어 {ip+1}]: {line} -> 오류: 대상 {operand1}은 레지스터가 아님")
                else:
                    value = parse_operand(operand2, registers)
                    registers[operand1] = value
                    print(f"[명령어 {ip+1}]: {line}")
                    print(f"{operand1}에 값 {value} 이동")
            elif opcode == 'J':
                target = parse_operand(operand1, registers)
                if target < 1 or target > instruction_count:
                    print(f"[명령어 {ip+1}]: {line} -> 오류: 점프 대상 {target} 범위 초과")
                else:
                    print(f"[명령어 {ip+1}]: {line} -> 점프: 명령어 {target}로 이동")
                    ip = target - 1
                    continue
            elif opcode == 'C':
                val1 = parse_operand(operand1, registers)
                val2 = parse_operand(operand2, registers)
                if val1 >= val2:
                    registers["R0"] = 0
                    print(f"[명령어 {ip+1}]: {line}")
                    print(f"비교 결과: {val1} >= {val2} 이므로 R0 = 0")
                else:
                    registers["R0"] = 1
                    print(f"[명령어 {ip+1}]: {line}")
                    print(f"비교 결과: {val1} < {val2} 이므로 R0 = 1")
            elif opcode == 'B':
                if registers["R0"] == 1:
                    target = parse_operand(operand1, registers)
                    if target < 1 or target > instruction_count:
                        print(f"[명령어 {ip+1}]: {line} -> 오류: 분기 대상 {target} 범위 초과")
                    else:
                        print(f"[명령어 {ip+1}]: {line} -> 분기: 조건 만족, 명령어 {target}로 이동")
                        ip = target - 1
                        continue
                else:
                    print(f"[명령어 {ip+1}]: {line} -> 분기 조건 미달, R0 = {registers['R0']}")
            elif opcode == 'H':
                print(f"[명령어 {ip+1}]: H -> 종료 명령, 프로그램 종료")
                break
            else:
                print(f"[명령어 {ip+1}]: {line} -> 오류: 알 수 없는 opcode '{opcode}'")
        except Exception as e:
            print(f"[명령어 {ip+1}]: {line} -> 예외 발생: {e}")
        print("현재 레지스터 상태:", registers)
        print("----------------------------------------")
        ip += 1

if __name__ == "__main__":
    main()
