#include "CPUCore.h"
#include <iostream>
#include <iomanip>
#include <bitset>

//Status Register flags
#define SR_CCR_CARRY 0
#define SR_CCR_OVERFLOW 1
#define SR_CCR_ZERO 2
#define SR_CCR_NEGATIVE 3
#define SR_CCR_EXTEND 4
#define SR_INT0 8
#define SR_INT1 9
#define SR_INT2 10
#define SR_SUPERVISOR_MODE 13
#define SR_TRACE_MODE 15

//Instructions
#define ADD 0xD000
#define ADDA 0xD000
#define ADDI 0x0600
#define ADDQ 0x5000
#define Bcc 0x6000
#define CLR 0x4200
#define CMP_B 0xB000
#define CMP_W 0xB040
#define CMP_L 0xB080
#define JMP 0x4EC0
#define LEA 0x41C0
#define MOVE_B 0x1000
#define MOVE_W 0x3000
#define MOVE_L 0x2000
#define MOVEQ 0x7000
#define NOP 0x4E71
#define TRAP 0x4E40

//Addressing modes
#define ADDRESS_MODE_DATA_REGISTER_DIRECT 0
#define ADDRESS_MODE_ADDRESS_REGISTER_DIRECT 1
#define ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT 2
#define ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT 3
#define ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT 4
#define ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT 5
#define ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX 6
//Mode = 7 and address register = :
#define ADDRESS_MODE_OTHERS 7
#define ADDRESS_MODE_ABSOLUTE_SHORT 0
#define ADDRESS_MODE_ABSOLUTE_LONG 1
#define ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT 2
#define ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX 3
#define ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER 4

//Size codes
#define SIZE_BYTE 0
#define SIZE_WORD 1
#define SIZE_LONG 2

//Index size codes
#define INDEX_SIZE_WORD 0
#define INDEX_SIZE_LONG 0x8

//Conditional tests
#define CONDITIONAL_TRUE 0
#define CONDITIONAL_FALSE 1
#define CONDITIONAL_HIGH 2
#define CONDITIONAL_LOW_OR_SAME 3
#define CONDITIONAL_CARRY_CLEAR 4
#define CONDITIONAL_CARRY_SET 5
#define CONDITIONAL_NOT_EQUAL 6
#define CONDITIONAL_EQUAL 7
#define CONDITIONAL_OVERFLOW_CLEAR 8
#define CONDITIONAL_OVERFLOW_SET 9
#define CONDITIONAL_PLUS 10
#define CONDITIONAL_MINUS 11
#define CONDITIONAL_GREATER_OR_EQUAL 12
#define CONDITIONAL_LESS_THAN 13
#define CONDITIONAL_GREATER_THAN 14
#define CONDITIONAL_LESS_OR_EQUAL 15

using namespace std;

CPUCore::CPUCore(Memory *memory, int model = 68000)
{
	//TODO: Check for valid model number
	this->model = (models)model;
	this->memory = memory;

	D[0] = 0;
	D[1] = 0;
	D[2] = 0;
	D[3] = 0;
	D[4] = 0;
	D[5] = 0;
	D[6] = 0;
	D[7] = 0;
	A[0] = 0;
	A[1] = 0;
	A[2] = 0;
	A[3] = 0;
	A[4] = 0;
	A[5] = 0;
	A[6] = 0;
	SP = 0x00FFFFFF;
	SR = 1 << SR_SUPERVISOR_MODE;
	PC = 0;
}


CPUCore::~CPUCore()
{
}

bool CPUCore::startNextCycle()
{
	uint16_t instruction = memory->readWordFromMemory(PC);
	if (decodeInstruction(instruction)) {
		PC += 2;
		return true;
	}
	else {
		return false;
	}
}


// Decodes and executes instruction. Returns true when successful and false otherwise
bool CPUCore::decodeInstruction(uint16_t instruction)
{
	uint32_t data = 0;
	uint32_t data2 = 0;
	uint32_t result = 0;
	bool mostSignificantBitSource = 0;
	bool mostSignificantBitDestination = 0;
	bool mostSignificantBitResult = 0;
	int16_t displacement = 0;
	int32_t longDisplacement = 0;
	uint32_t absoluteAddress = 0;
	uint8_t indexRegister = 0;
	uint8_t indexSize = 0;

	if (debugMode)
		cout << hex << "The instruction is: " << instruction << dec << endl;
	
	// CLR (Clear an Operand)
	if ((instruction & 0xFF00) == CLR) {
		SR |= 1 << SR_CCR_ZERO;
		SR &= ~(1 << SR_CCR_NEGATIVE);
		SR &= ~(1 << SR_CCR_OVERFLOW);
		SR &= ~(1 << SR_CCR_CARRY);
		
		int size = ((instruction >> 6) & 3);
		int mode = ((instruction >> 3) & 7);
		int reg = (instruction & 7);

		if (debugMode) {
			cout << "WE HAVE A CLEAR" << endl;
			cout << "Size is: " << size << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Address register is: " << reg << endl << endl;
		}

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			if (size == SIZE_BYTE)
				writeByteToDataRegister(0, reg);
			else if (size == SIZE_WORD)
				writeWordToDataRegister(0, reg);
			else
				writeLongToDataRegister(0, reg);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_BYTE)
				memory->writeByteToMemory(0, A[reg]);
			else if (size == SIZE_WORD)
				memory->writeWordToMemory(0, A[reg]);
			else
				memory->writeLongToMemory(0, A[reg]);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_BYTE) {
				memory->writeByteToMemory(0, A[reg]);
				A[reg]++;
			}
			else if (size == SIZE_WORD) {
				memory->writeWordToMemory(0, A[reg]);
				A[reg] += 2;
			}
			else {
				memory->writeLongToMemory(0, A[reg]);
				A[reg] += 4;
			}
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_BYTE) {
				A[reg]--;
				memory->writeByteToMemory(0, A[reg]);
			} 
			else if (size == SIZE_WORD) {
				A[reg] -= 2;
				memory->writeWordToMemory(0, A[reg]);
			}
			else {
				A[reg] -= 4;
				memory->writeLongToMemory(0, A[reg]);
			}
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_BYTE)
				memory->writeByteToMemory(0, A[reg], displacement);
			else if (size == SIZE_WORD)
				memory->writeWordToMemory(0, A[reg], displacement);
			else
				memory->writeLongToMemory(0, A[reg], displacement);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				} else
					longDisplacement += D[indexRegister];
			} else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			
			if (size == SIZE_BYTE)
				memory->writeByteToMemory(0, A[reg], longDisplacement);
			else if (size == SIZE_WORD)
				memory->writeWordToMemory(0, A[reg], longDisplacement);
			else
				memory->writeLongToMemory(0, A[reg], longDisplacement);
			return true;
			break;
		case ADDRESS_MODE_OTHERS:
			switch (reg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_BYTE)
					memory->writeByteToMemory(0, absoluteAddress);
				else if (size == SIZE_WORD)
					memory->writeWordToMemory(0, absoluteAddress);
				else
					memory->writeLongToMemory(0, absoluteAddress);
				return true;
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_BYTE)
					memory->writeByteToMemory(0, absoluteAddress);
				else if (size == SIZE_WORD)
					memory->writeWordToMemory(0, absoluteAddress);
				else
					memory->writeLongToMemory(0, absoluteAddress);
				PC += 2;
				return true;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}
	}

	// JMP (Jump)
	if ((instruction & 0xFFC0) == JMP) {
		int mode = ((instruction >> 3) & 7);
		int reg = (instruction & 7);

		if (debugMode) {
			cout << "JUMPING" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Address register is: " << reg << endl << endl;
		}

		switch (mode) {
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			PC = A[reg] - 2;
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			PC = A[reg] + displacement - 2;
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}

			PC = A[reg] + longDisplacement - 2;
			return true;
			break;
		case ADDRESS_MODE_OTHERS:
			switch (reg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				PC = absoluteAddress - 2;
				return true;
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				PC = absoluteAddress - 2;
				PC += 2;
				return true;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				PC += displacement - 2;;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				PC += longDisplacement - 2;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;
		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}
	}

	// MOVE.B (Move byte from source to destination)
	if ((instruction & 0xF000) == MOVE_B) {
		SR &= ~(1 << SR_CCR_OVERFLOW);
		SR &= ~(1 << SR_CCR_CARRY);
		
		int sourceMode = ((instruction >> 3) & 7);
		int sourceReg = (instruction & 7);
		int destinationMode = ((instruction >> 6) & 7);
		int destinationReg = ((instruction >> 9) & 7);

		if (debugMode) {
			cout << "WE HAVE A MOVE BYTE" << endl;
			cout << "Source mode is: " << sourceMode << endl;
			cout << "Source address register is: " << sourceReg << endl;
			cout << "Destination mode is: " << destinationMode << endl;
			cout << "Destination address register is: " << destinationReg << endl << endl;
		}

		switch (sourceMode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			data = D[sourceReg];
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			data = memory->readByteFromMemory(A[sourceReg]);
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			data = memory->readByteFromMemory(A[sourceReg]);
			A[sourceReg]++;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			A[sourceReg]--;
			data = memory->readByteFromMemory(A[sourceReg]);
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			data = memory->readByteFromMemory(A[sourceReg], displacement);
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			data = memory->readByteFromMemory(A[sourceReg], longDisplacement);
			break;
		case ADDRESS_MODE_OTHERS:
			switch (sourceReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				data = memory->readByteFromMemory(absoluteAddress);
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				data = memory->readByteFromMemory(absoluteAddress);
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				data = memory->readByteFromMemory(PC, displacement);
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				data = memory->readByteFromMemory(PC, longDisplacement);
				break;
			case ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER:
				PC += 2;
				data = memory->readWordFromMemory(PC);
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		data == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		((data >> 7) & 1) == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);
		
		switch (destinationMode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			writeByteToDataRegister(data, destinationReg);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			memory->writeByteToMemory(data, A[destinationReg]);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			memory->writeByteToMemory(data, A[destinationReg]);
			A[destinationReg]++;
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			A[destinationReg]--;
			memory->writeByteToMemory(data, A[destinationReg]);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			cout << "Displacement: " << displacement << endl;
			memory->writeByteToMemory(data, A[destinationReg], displacement);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			memory->writeByteToMemory(data, A[destinationReg], longDisplacement);
			return true;
			break;
		case ADDRESS_MODE_OTHERS:
			switch (destinationReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				memory->writeByteToMemory(data, absoluteAddress);
				return true;
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				memory->writeByteToMemory(data, absoluteAddress);
				PC += 2;
				return true;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}
	}

	// MOVE.W/MOV.L (Move word or long from source to destination)
	if (((instruction & 0xF000) == MOVE_W) || ((instruction & 0xF000) == MOVE_L)) {
		int size = SIZE_LONG;
		if ((instruction & MOVE_W) == MOVE_W)
			size = SIZE_WORD;

		SR &= ~(1 << SR_CCR_OVERFLOW);
		SR &= ~(1 << SR_CCR_CARRY);
		
		int sourceMode = ((instruction >> 3) & 7);
		int sourceReg = (instruction & 7);
		int destinationMode = ((instruction >> 6) & 7);
		int destinationReg = ((instruction >> 9) & 7);

		if (debugMode) {
			cout << "WE HAVE A MOVE" << endl;
			cout << "Source mode is: " << sourceMode << endl;
			cout << "Source address register is: " << sourceReg << endl;
			cout << "Destination mode is: " << destinationMode << endl;
			cout << "Destination address register is: " << destinationReg << endl << endl;
		}

		switch (sourceMode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			data = D[sourceReg];
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			data = A[sourceReg];
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_WORD)
				data = memory->readWordFromMemory(A[sourceReg]);
			else
				data = memory->readLongFromMemory(A[sourceReg]);
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_WORD) {
				data = memory->readWordFromMemory(A[sourceReg]);
				A[sourceReg] += 2;
			}
			else {
				data = memory->readLongFromMemory(A[sourceReg]);
				A[sourceReg] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_WORD) {
				A[sourceReg] -= 2;
				data = memory->readWordFromMemory(A[sourceReg]);
			}
			else {
				A[sourceReg] -= 4;
				data = memory->readLongFromMemory(A[sourceReg]);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_WORD)
				data = memory->readWordFromMemory(A[sourceReg], displacement);
			else
				data = memory->readLongFromMemory(A[sourceReg], displacement);
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_WORD)
				data = memory->readWordFromMemory(A[sourceReg], longDisplacement);
			else
				data = memory->readLongFromMemory(A[sourceReg], longDisplacement);
			break;
		case ADDRESS_MODE_OTHERS:
			switch (sourceReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_WORD) {
					data = memory->readWordFromMemory(absoluteAddress);
				}
				else {
					data = memory->readLongFromMemory(absoluteAddress);
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_WORD) {
					data = memory->readWordFromMemory(absoluteAddress);
				}
				else {
					data = memory->readLongFromMemory(absoluteAddress);
				}
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				if (size == SIZE_WORD) {
					data = memory->readWordFromMemory(PC, displacement);
				}
				else {
					data = memory->readLongFromMemory(PC, displacement);
					PC += 2;
				}
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				if (size == SIZE_WORD) {
					data = memory->readWordFromMemory(PC, longDisplacement);
				}
				else {
					data = memory->readLongFromMemory(PC, longDisplacement);
					PC += 2;
				}
				break;
			case ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER:
				PC += 2;
				if (size == SIZE_WORD) {
					data = memory->readWordFromMemory(PC);
				}
				else {
					data = memory->readLongFromMemory(PC);
					PC += 2;
				}
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		data == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);

		if (size == SIZE_WORD)
			((data >> 15) & 1) == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);
		else
			((data >> 31) & 1) == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);

		switch (destinationMode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			if (size == SIZE_WORD)
				writeWordToDataRegister(data, destinationReg);
			else
				writeLongToDataRegister(data, destinationReg);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			if (size == SIZE_WORD)
				writeWordToAddressRegister(data, destinationReg);
			else
				writeLongToAddressRegister(data, destinationReg);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_WORD)
				memory->writeWordToMemory(data, A[destinationReg]);
			else
				memory->writeLongToMemory(data, A[destinationReg]);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_WORD) {
				memory->writeWordToMemory(data, A[destinationReg]);
				A[destinationReg] += 2;
			}
			else {
				memory->writeLongToMemory(data, A[destinationReg]);
				A[destinationReg] += 4;
			}
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_WORD) {
				A[destinationReg] += 2;
				memory->writeWordToMemory(data, A[destinationReg]);
			}
			else {
				A[destinationReg] += 4;
				memory->writeLongToMemory(data, A[destinationReg]);
			}
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_WORD)
				memory->writeWordToMemory(data, A[destinationReg], displacement);
			else
				memory->writeLongToMemory(data, A[destinationReg], displacement);
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_WORD)
				memory->writeWordToMemory(data, A[destinationReg], longDisplacement);
			else
				memory->writeLongToMemory(data, A[destinationReg], longDisplacement);
			return true;
			break;
		case ADDRESS_MODE_OTHERS:
			switch (destinationReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_WORD)
					memory->writeWordToMemory(data, absoluteAddress);
				else
					memory->writeLongToMemory(data, absoluteAddress);
				return true;
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_WORD)
					memory->writeWordToMemory(data, absoluteAddress);
				else
					memory->writeLongToMemory(data, absoluteAddress);
				PC += 2;
				return true;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}
	}

	// MOVEQ (Move quick)
	if ((instruction & 0xF000) == MOVEQ) {

		SR &= ~(1 << SR_CCR_OVERFLOW);
		SR &= ~(1 << SR_CCR_CARRY);
		data = instruction & 0xFF;
		int destinationReg = ((instruction >> 9) & 7);
		if (debugMode) {
			cout << "WE HAVE A MOVE QUICK" << endl;
			cout << "Data is: " << hex << data << dec << endl;
			cout << "Destination register is: " << destinationReg << endl;
		}

		data == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		((data >> 7) & 0x1) == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);

		writeByteToDataRegister(data, destinationReg);

		return true;
	}

	// TRAP
	if ((instruction & 0xFFF0) == TRAP) {
		uint8_t vector = instruction & 15;
		if (debugMode)
			cout << "TRAP" << endl;
		char character = 0;
		unsigned int characterIndex = 0;
		
		switch (vector) {
		case 15:
			/* IO (Compatible with Easy68k)
			0	 Display string at (A1), D1.W bytes long (max 255) with carriage return and line feed (CR, LF). (see task 13)
			1	 Display string at (A1), D1.W bytes long (max 255) without CR, LF. (see task 14)
			2	 Read string from keyboard and store at (A1), NULL terminated, length retuned in D1.W (max 80)
			3	 Display signed number in D1.L in decimal in smallest field. (see task 15 & 20)
			4	 Read a number from the keyboard into D1.L.
			5	 Read single character from the keyboard into D1.B.
			6	 Display single character in D1.B.
			7
			Set D1.B to 1 if keyboard input is pending, otherwise set to 0.
			Use code 5 to read pending key.

			8	 Return time in hundredths of a second since midnight in D1.L.
			9	 Terminate the program. (Halts the simulator)
			10
			Print the NULL terminated string at (A1) to the default printer. (Not Teesside compatible.)
			Always send a Form Feed character to end printing. (See below.)
			11
			Position the cursor at ROW, COL.
			The high byte of D1.W holds the COL number (0-79),
			The low byte holds the ROW number (0-31).
			0,0 is top left 79,31 is the bottom right.
			Out of range coordinates are ignored.
			Clear Screen : Set D1.W to $FF00.
			12
			Keyboard Echo.
			D1.B = 0 to turn off keyboard echo.
			D1.B = non zero to enable it (default).
			Echo is restored on 'Reset' or when a new file is loaded.
			13	 Display the NULL terminated string at (A1) with CR, LF.
			14	 Display the NULL terminated string at (A1) without CR, LF.
			15
			Display the unsigned number in D1.L converted to number base (2 through 36) contained in D2.B.
			For example, to display D1.L in base16 put 16 in D2.B
			Values of D2.B outside the range 2 to 36 inclusive are ignored.
			16	 Adjust display properties
			D1.B = 0 to turn off the display of the input prompt.
			D1.B = 1 to turn on the display of the input prompt. (default)
			D1.B = 2 do not display a line feed when Enter pressed during Trap task #2 input
			D1.B = 3 display a line feed when Enter key pressed during Trap task #2 input (default)
			Other values of D1 reserved for future use.
			Input prompt display is enabled by default and by 'Reset' or when a new file is loaded.
			17
			Combination of Trap codes 14 & 3.
			Display the NULL terminated string at (A1) without CR, LF then
			Display the decimal number in D1.L.
			18	 Combination of Trap codes 14 & 4.
			Display the NULL terminated string at (A1) without CR, LF then
			Read a number from the keyboard into D1.L.
			19	 Returns current state of up to 4 specified keys or returns key scan code.
			Pre: D1.L = four 1-byte key codes
			Post: D1.L contains four 1-byte Booleans.
			$FF = corresponding key is pressed, $00 = corresponding key not pressed.
			Pre: D1.L = $00000000
			Post: D1.B contains key code of last key pressed
			20	 Display signed number in D1.L in decimal in field D2.B columns wide.
			21	 Set Font Color
			D1.L = color as $00BBGGRR
			BB is amount of blue from $00 to $FF
			GG is amount of green from $00 to $FF
			RR is amount of red from $00 to $FF
			D2.B = style by bits,  0 = off, 1 = on
			bit0 is Bold
			bit1 is Italic
			bit2 is Underline
			bit3 is StrikeOut
			22
			Read char at Row,Col of text screen.
			Pre: D1.L = High 16 bits = Row
			Low 16 bits = Col
			Post: D1.B contains ASCII code of character.
			*/
			switch (D[0] & 0xFF) {
			case 4:
				int number;
				cin >> number;
				writeLongToDataRegister(number, 1);
				return true;
				break;
			case 9:
				return false;
				break;
			case 13:
				character = memory->readByteFromMemory(A[1]);
				characterIndex = 0;
				while (character != 0) {
					cout << character;
					characterIndex++;
					character = memory->readByteFromMemory(A[1] + characterIndex);
				}
				cout << endl;
				return true;
				break;
			case 14:
				character = memory->readByteFromMemory(A[1]);
				characterIndex = 0;
				while (character != 0) {
					cout << character;
					characterIndex++;
					character = memory->readByteFromMemory(A[1] + characterIndex);
				}
				return true;
				break;
			default:
				cout << "Unknown IO task" << endl;
				break;
			}
		default:
			cout << "Unknown TRAP task" << endl;
		}
	}

	// NOP (No Operation)
	if (instruction == NOP) {
		if (debugMode)
			cout << "NO OPERATION" << endl;

		return true;
	}

	// LEA (Load Effective Addreww)
	if ((instruction & 0xF1C0) == LEA) {

		int sourceMode = ((instruction >> 3) & 7);
		int sourceReg = (instruction & 7);
		int destinationReg = ((instruction >> 9) & 7);

		if (debugMode) {
			cout << "LOADING EFFECTIVE ADDRESS" << endl;
			cout << "Source mode is: " << sourceMode << endl;
			cout << "Source address register is: " << sourceReg << endl;
			cout << "Destination address register is: " << destinationReg << endl << endl;
		}

		switch (sourceMode) {
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			A[destinationReg] = A[sourceReg];
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			A[destinationReg] = A[sourceReg] + displacement;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			A[destinationReg] = A[sourceReg] + longDisplacement;
			break;
		case ADDRESS_MODE_OTHERS:
			switch (sourceReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				A[destinationReg] = absoluteAddress;
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				A[destinationReg] = absoluteAddress;
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				A[destinationReg] = PC + displacement;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				A[destinationReg] = PC + longDisplacement;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		return true;
	}

	// ADD (Add Binary)
	if ((instruction & 0xF000) == ADD) {
		int size = (instruction >> 6) & 7;

		if (size == 3 || size == 7)
			goto AddA;

		int mode = (instruction >> 3) & 7;
		int dataReg = (instruction >> 9) & 7;
		int addressRegister = instruction & 7;

		bool registerIsSource = false;

		if (size > 3)
			registerIsSource = true;

		if (debugMode) {
			cout << "WE HAVE AN ADD" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Data register is: " << dataReg << endl;
			cout << "Address register is: " << addressRegister << endl;
			if (registerIsSource)
				cout << "Register is source." << endl << endl;
			else
				cout << "Register is not source." << endl << endl;
		}

		if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
			data = (uint8_t)D[dataReg];
			if (registerIsSource)
				mostSignificantBitSource = (data >> 7) & 1;
			else 
				mostSignificantBitDestination = (data >> 7) & 1;
		}
		else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
			data = (uint16_t)D[dataReg];
			if (registerIsSource)
				mostSignificantBitSource = (data >> 15) & 1;
			else
				mostSignificantBitDestination = (data >> 15) & 1;
		}
		else {
			data = D[dataReg];
			if (registerIsSource)
				mostSignificantBitSource = (data >> 31) & 1;
			else
				mostSignificantBitDestination = (data >> 31) & 1;
		}

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
				data2 = (uint8_t)D[addressRegister];
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					writeByteToDataRegister((uint8_t)result, addressRegister);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister((uint8_t)result, dataReg);
				}
			}
			else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
				data2 = (uint16_t)D[addressRegister];
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					writeWordToDataRegister((uint16_t)result, addressRegister);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister((uint16_t)result, dataReg);
				}
			}
			else {
				data2 = D[addressRegister];
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					writeLongToDataRegister(result, addressRegister);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
				cout << "Invalid addressing mode." << endl;
				return false;
			} 
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)A[addressRegister];
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					writeWordToDataRegister((uint16_t)result, addressRegister);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister((uint16_t)result, dataReg);
				}
			}
			else if ((size - 4) == SIZE_WORD) {
				cout << "Invalid addressing mode." << endl;
				return false;
			}
			else if (size == SIZE_LONG) {
				data2 = A[addressRegister];
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					writeLongToDataRegister(result, addressRegister);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			else {
				cout << "Invalid addressing mode." << endl;
				return false;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					memory->writeByteToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					memory->writeWordToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					memory->writeLongToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					memory->writeByteToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
				A[addressRegister]++;
			}
			else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					memory->writeWordToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
				A[addressRegister] += 2;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					memory->writeLongToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
				A[addressRegister] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_BYTE) {
				A[addressRegister]--;
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					memory->writeByteToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
			}
			else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
				A[addressRegister] -= 2;
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					memory->writeWordToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
			}
			else {
				A[addressRegister] -= 4;
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					memory->writeLongToMemory(result, A[addressRegister]);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister], displacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					memory->writeByteToMemory(result, A[addressRegister], displacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
			}
			else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], displacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					memory->writeWordToMemory(result, A[addressRegister], displacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], displacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					memory->writeLongToMemory(result, A[addressRegister], displacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister], longDisplacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 7) & 1;
					memory->writeByteToMemory(result, A[addressRegister], longDisplacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
			}
			else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], longDisplacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 15) & 1;
					memory->writeWordToMemory(result, A[addressRegister], longDisplacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], longDisplacement);
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				if (registerIsSource) {
					mostSignificantBitDestination = (data2 >> 31) & 1;
					memory->writeLongToMemory(result, A[addressRegister], longDisplacement);
				}
				else {
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
				}
			}
			break;
		case ADDRESS_MODE_OTHERS:
			switch (addressRegister) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 7) & 1;
						memory->writeByteToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 7) & 1;
						writeByteToDataRegister(result, dataReg);
					}
				}
				else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 15) & 1;
						memory->writeWordToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 15) & 1;
						writeWordToDataRegister(result, dataReg);
					}
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 31) & 1;
						memory->writeLongToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 31) & 1;
						writeLongToDataRegister(result, dataReg);
					}
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 7) & 1;
						memory->writeByteToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 7) & 1;
						writeByteToDataRegister(result, dataReg);
					}
				}
				else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 15) & 1;
						memory->writeWordToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 15) & 1;
						writeWordToDataRegister(result, dataReg);
					}
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					if (registerIsSource) {
						mostSignificantBitDestination = (data2 >> 31) & 1;
						memory->writeLongToMemory(result, absoluteAddress);
					}
					else {
						mostSignificantBitSource = (data2 >> 31) & 1;
						writeLongToDataRegister(result, dataReg);
					}
				}
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC, displacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, displacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, displacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC, longDisplacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, longDisplacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, longDisplacement);
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			case ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER:
				PC += 2;
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC);
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
					writeByteToDataRegister(result, dataReg);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC);
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
					writeWordToDataRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC);
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					writeLongToDataRegister(result, dataReg);
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
			(uint8_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
			(uint16_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else {
			result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}

		mostSignificantBitResult == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);

		if (mostSignificantBitDestination == 1 && mostSignificantBitResult == 0)
			SR |= 1 << SR_CCR_CARRY;
		else
			SR &= ~(1 << SR_CCR_CARRY);

		if (mostSignificantBitSource == mostSignificantBitDestination)
			mostSignificantBitDestination != mostSignificantBitResult ? SR |= 1 << SR_CCR_OVERFLOW : SR &= ~(1 << SR_CCR_OVERFLOW);

		((SR >> SR_CCR_CARRY) & 1) == 1 ? SR |= 1 << SR_CCR_EXTEND : SR &= ~(1 << SR_CCR_EXTEND);

		return true;
	}
	
	// ADDA (Add Address)
	if ((instruction & 0xF000) == ADD) {
		AddA:
		int size = (instruction >> 6) & 7;

		if (size == 3)
			size == SIZE_WORD;
		else if (size == SIZE_LONG)
			size == SIZE_LONG;
		else {
			cout << "Invalid addressing mode." << endl;
			return false;
		}

		int mode = (instruction >> 3) & 7;
		int dataReg = (instruction >> 9) & 7;
		int addressRegister = instruction & 7;

		if (debugMode) {
			cout << "WE HAVE AN ADDA" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Destination address register is: " << dataReg << endl;
			cout << "Address register is: " << addressRegister << endl;
		}

		if (size == SIZE_WORD)
			data = (uint16_t)A[dataReg];
		else
			data = A[dataReg];

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_WORD) {
				data2 = (uint16_t)D[addressRegister];
				result = data + data2;
				writeWordToAddressRegister((uint16_t)result, dataReg);
			}
			else {
				data2 = D[addressRegister];
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_WORD) {
				data2 = (uint16_t)A[addressRegister];
				result = data + data2;
				writeWordToAddressRegister((uint16_t)result, dataReg);
			}
			else {
				data2 = A[addressRegister];
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				writeWordToAddressRegister(result, dataReg);
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				writeWordToAddressRegister(result, dataReg);
				A[addressRegister] += 2;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
				A[addressRegister] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_WORD) {
				A[addressRegister] -= 2;
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data + data2;
				writeWordToAddressRegister(result, dataReg);
			}
			else {
				A[addressRegister] -= 4;
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], displacement);
				result = data + data2;
				writeWordToAddressRegister(result, dataReg);
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], displacement);
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], longDisplacement);
				result = data + data2;
				writeWordToAddressRegister(result, dataReg);
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], longDisplacement);
				result = data + data2;
				writeLongToAddressRegister(result, dataReg);
			}
			break;
		case ADDRESS_MODE_OTHERS:
			switch (addressRegister) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data + data2;
					writeWordToAddressRegister(result, dataReg);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data + data2;
					writeLongToAddressRegister(result, dataReg);
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data + data2;
					writeWordToAddressRegister(result, dataReg);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data + data2;
					writeLongToAddressRegister(result, dataReg);
				}
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, displacement);
					result = data + data2;
					writeWordToAddressRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, displacement);
					result = data + data2;
					writeLongToAddressRegister(result, dataReg);
					PC += 2;
				}
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, longDisplacement);
					result = data + data2;
					writeWordToAddressRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, longDisplacement);
					result = data + data2;
					writeLongToAddressRegister(result, dataReg);
					PC += 2;
				}
				break;
			case ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER:
				PC += 2;
				if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC);
					result = data + data2;
					writeWordToAddressRegister(result, dataReg);
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC);
					result = data + data2;
					writeLongToAddressRegister(result, dataReg);
					PC += 2;
				}
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		return true;
	}

	// ADDI (Add Immediate)
	if ((instruction & 0xFF00) == ADDI) {
		int size = (instruction >> 6) & 3;

		int mode = ((instruction >> 3) & 7);
		int destinationReg = (instruction & 7);

		if (debugMode) {
			cout << "WE HAVE AN ADD IMMEDIATE" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Destination address register is: " << destinationReg << endl << endl;
		}

		PC += 2;

		if (size == SIZE_BYTE) {
			data = memory->readByteFromMemory(PC + 1);
			mostSignificantBitSource = (data >> 7) & 1;
		}
		else if (size == SIZE_WORD) {
			data = memory->readWordFromMemory(PC);
			mostSignificantBitSource = (data >> 15) & 1;
		}
		else {
			data = memory->readLongFromMemory(PC);
			mostSignificantBitSource = (data >> 31) & 1;
			PC += 2;
		}

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE) {
				data2 = (uint8_t)D[destinationReg];
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				writeByteToDataRegister((uint8_t)result, destinationReg);
			}
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)D[destinationReg];
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				writeWordToDataRegister((uint16_t)result, destinationReg);
			}
			else {
				data2 = D[destinationReg];
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				writeLongToDataRegister(result, destinationReg);
			}
			
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
				A[destinationReg]++;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
				A[destinationReg] += 2;
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
				A[destinationReg] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_BYTE) {
				A[destinationReg]--;
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
			}
			else if (size == SIZE_WORD) {
				A[destinationReg] -= 2;
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
			}
			else {
				A[destinationReg] -= 4;
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg], displacement);
			} else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg], displacement);
			} else {
				data2 = memory->readLongFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg], displacement);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg], longDisplacement);
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg], longDisplacement);
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg], longDisplacement);
			}
			break;
		case ADDRESS_MODE_OTHERS:
			switch (destinationReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 7) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					memory->writeByteToMemory(result, absoluteAddress);
				} else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 15) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					memory->writeWordToMemory(result, absoluteAddress);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 31) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					memory->writeLongToMemory(result, absoluteAddress);
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 7) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					memory->writeByteToMemory(result, absoluteAddress);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 15) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					memory->writeWordToMemory(result, absoluteAddress);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 31) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					memory->writeLongToMemory(result, absoluteAddress);
				}
				PC += 2;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		if (size == SIZE_BYTE) {
			(uint8_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else if (size == SIZE_WORD) {
			(uint16_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else {
			result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}

		mostSignificantBitResult == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);
		
		if (mostSignificantBitDestination == 1 && mostSignificantBitResult == 0)
			SR |= 1 << SR_CCR_CARRY;
		else
			SR &= ~(1 << SR_CCR_CARRY);

		if (mostSignificantBitSource == mostSignificantBitDestination)
			mostSignificantBitDestination != mostSignificantBitResult ? SR |= 1 << SR_CCR_OVERFLOW : SR &= ~(1 << SR_CCR_OVERFLOW);	

		((SR >> SR_CCR_CARRY) & 1) == 1 ? SR |= 1 << SR_CCR_EXTEND : SR &= ~(1 << SR_CCR_EXTEND);

		return true;
	}

	// ADDQ (Add Quick)
	if ((instruction & 0xF100) == ADDQ) {
		int size = (instruction >> 6) & 3;

		int mode = (instruction >> 3) & 7;
		int destinationReg = (instruction & 7);

		data = (instruction >> 9) & 7;
		mostSignificantBitSource = (data >> 2) & 1;

		if (debugMode) {
			cout << "WE HAVE AN ADD QUICK" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Destination address register is: " << destinationReg << endl << endl;
		}

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE) {
				data2 = (uint8_t)D[destinationReg];
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				writeByteToDataRegister((uint8_t)result, destinationReg);
			}
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)D[destinationReg];
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				writeWordToDataRegister((uint16_t)result, destinationReg);
			}
			else {
				data2 = D[destinationReg];
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				writeLongToDataRegister(result, destinationReg);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			if (size == SIZE_BYTE) {
				cout << "Invalid addressing mode." << endl;
				return false;
			}
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)A[destinationReg];
				result = data + data2;
				writeWordToAddressRegister((uint16_t)result, destinationReg);
			}
			else {
				data2 = A[destinationReg];
				result = data + data2;
				writeLongToAddressRegister(result, destinationReg);
			}
			return true;
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
				A[destinationReg]++;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
				A[destinationReg] += 2;
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
				A[destinationReg] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_BYTE) {
				A[destinationReg]--;
				data2 = memory->readByteFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg]);
			}
			else if (size == SIZE_WORD) {
				A[destinationReg] -= 2;
				data2 = memory->readWordFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg]);
			}
			else {
				A[destinationReg] -= 4;
				data2 = memory->readLongFromMemory(A[destinationReg]);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg]);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg], displacement);
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg], displacement);
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg], displacement);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg], displacement);
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 7) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 7) & 1;
				memory->writeByteToMemory(result, A[destinationReg], longDisplacement);
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 15) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 15) & 1;
				memory->writeWordToMemory(result, A[destinationReg], longDisplacement);
			}
			else {
				data2 = memory->readLongFromMemory(A[destinationReg], longDisplacement);
				mostSignificantBitDestination = (data2 >> 31) & 1;
				result = data + data2;
				mostSignificantBitResult = (result >> 31) & 1;
				memory->writeLongToMemory(result, A[destinationReg], longDisplacement);
			}
			break;
		case ADDRESS_MODE_OTHERS:
			switch (destinationReg) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 7) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					memory->writeByteToMemory(result, absoluteAddress);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 15) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					memory->writeWordToMemory(result, absoluteAddress);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 31) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					memory->writeLongToMemory(result, absoluteAddress);
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 7) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 7) & 1;
					memory->writeByteToMemory(result, absoluteAddress);
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 15) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 15) & 1;
					memory->writeWordToMemory(result, absoluteAddress);
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					mostSignificantBitDestination = (data2 >> 31) & 1;
					result = data + data2;
					mostSignificantBitResult = (result >> 31) & 1;
					memory->writeLongToMemory(result, absoluteAddress);
				}
				PC += 2;
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		if (size == SIZE_BYTE) {
			(uint8_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else if (size == SIZE_WORD) {
			(uint16_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else {
			result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}

		mostSignificantBitResult == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);

		if (mostSignificantBitDestination == 1 && mostSignificantBitResult == 0)
			SR |= 1 << SR_CCR_CARRY;
		else
			SR &= ~(1 << SR_CCR_CARRY);

		if (mostSignificantBitSource == mostSignificantBitDestination)
			mostSignificantBitDestination != mostSignificantBitResult ? SR |= 1 << SR_CCR_OVERFLOW : SR &= ~(1 << SR_CCR_OVERFLOW);

		((SR >> SR_CCR_CARRY) & 1) == 1 ? SR |= 1 << SR_CCR_EXTEND : SR &= ~(1 << SR_CCR_EXTEND);

		return true;
	}

	// CMP (Compare)
	if (((instruction & 0xF1C0) == CMP_B) || ((instruction & 0xF1C0) == CMP_W) || ((instruction & 0xF1C0) == CMP_L)) {
		int size = (instruction >> 6) & 7;
		int mode = (instruction >> 3) & 7;
		int dataReg = (instruction >> 9) & 7;
		int addressRegister = instruction & 7;

		if (debugMode) {
			cout << "WE HAVE A COMPARE" << endl;
			cout << "Mode is: " << mode << endl;
			cout << "Data register is: " << dataReg << endl;
			cout << "Address register is: " << addressRegister << endl;
		}

		if (size == SIZE_BYTE) {
			data = (uint8_t)D[dataReg];
			mostSignificantBitDestination = (data >> 7) & 1;
		}
		else if (size == SIZE_WORD) {
			data = (uint16_t)D[dataReg];
			mostSignificantBitDestination = (data >> 15) & 1;
		}
		else {
			data = D[dataReg];
			mostSignificantBitDestination = (data >> 31) & 1;
		}

		switch (mode) {
		case ADDRESS_MODE_DATA_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE) {
				data2 = (uint8_t)D[addressRegister];
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
			}
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)D[addressRegister];
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				data2 = D[addressRegister];
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_DIRECT:
			cout << "Data: " << data << endl;
			if (size == SIZE_BYTE) {
				cout << "Invalid addressing mode." << endl;
				return false;
			}
			else if (size == SIZE_WORD) {
				data2 = (uint16_t)A[addressRegister];
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				data2 = A[addressRegister];
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_POSTINCREMENT:
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
				A[addressRegister]++;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
				A[addressRegister] += 2;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
				A[addressRegister] += 4;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_PREDECREMENT:
			if (size == SIZE_BYTE) {
				A[addressRegister]--;
				data2 = memory->readByteFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
			}
			else if (size == SIZE_WORD) {
				A[addressRegister] -= 2;
				data2 = memory->readWordFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				A[addressRegister] -= 4;
				data2 = memory->readLongFromMemory(A[addressRegister]);
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_DISPLACEMENT:
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
			if (debugMode)
				cout << "Displacement: " << displacement << endl;
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister], displacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], displacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], displacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_ADDRESS_REGISTER_INDIRECT_WITH_INDEX:
			PC += 2;
			indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
			indexSize = memory->readByteFromMemory(PC) & 0x0F;
			longDisplacement = memory->readByteFromMemory(PC + 1);
			if (indexRegister <= 7) {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)D[indexRegister];
				}
				else
					longDisplacement += D[indexRegister];
			}
			else {
				if (indexSize == INDEX_SIZE_WORD) {
					longDisplacement += (int16_t)A[indexRegister - 8];
				}
				else
					longDisplacement += D[indexRegister - 8];
			}
			if (size == SIZE_BYTE) {
				data2 = memory->readByteFromMemory(A[addressRegister], longDisplacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 7) & 1;
				mostSignificantBitSource = (data2 >> 7) & 1;
			}
			else if (size == SIZE_WORD) {
				data2 = memory->readWordFromMemory(A[addressRegister], longDisplacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 15) & 1;
				mostSignificantBitSource = (data2 >> 15) & 1;
			}
			else {
				data2 = memory->readLongFromMemory(A[addressRegister], longDisplacement);
				result = data - data2;
				mostSignificantBitResult = (result >> 31) & 1;
				mostSignificantBitSource = (data2 >> 31) & 1;
			}
			break;
		case ADDRESS_MODE_OTHERS:
			switch (addressRegister) {
			case ADDRESS_MODE_ABSOLUTE_SHORT:
				PC += 2;
				absoluteAddress = memory->readWordFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
				}
				break;
			case ADDRESS_MODE_ABSOLUTE_LONG:
				PC += 2;
				absoluteAddress = memory->readLongFromMemory(PC);
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
				}
				else {
					data2 = memory->readLongFromMemory(absoluteAddress);
					result = data - data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
				}
				PC += 2;
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_DISPLACEMENT:
				PC += 2;
				displacement = memory->readWordFromMemory(PC);
				if (debugMode)
					cout << "Displacement: " << displacement << endl;
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC, displacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, displacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, displacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			case ADDRESS_MODE_PROGRAM_COUNTER_WITH_INDEX:
				PC += 2;
				indexRegister = (memory->readByteFromMemory(PC) >> 4) & 0x0F;
				indexSize = memory->readByteFromMemory(PC) & 0x0F;
				longDisplacement = memory->readByteFromMemory(PC + 1);
				if (indexRegister <= 7) {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)D[indexRegister];
					}
					else
						longDisplacement += D[indexRegister];
				}
				else {
					if (indexSize == INDEX_SIZE_WORD) {
						longDisplacement += (int16_t)A[indexRegister - 8];
					}
					else
						longDisplacement += D[indexRegister - 8];
				}
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC, longDisplacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC, longDisplacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC, longDisplacement);
					result = data - data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			case ADDRESS_MODE_IMMEDIATE_OR_STATUS_REGISTER:
				PC += 2;
				if (size == SIZE_BYTE) {
					data2 = memory->readByteFromMemory(PC);
					result = data - data2;
					mostSignificantBitResult = (result >> 7) & 1;
					mostSignificantBitSource = (data2 >> 7) & 1;
				}
				else if (size == SIZE_WORD) {
					data2 = memory->readWordFromMemory(PC);
					result = data - data2;
					mostSignificantBitResult = (result >> 15) & 1;
					mostSignificantBitSource = (data2 >> 15) & 1;
				}
				else if (size == SIZE_LONG) {
					data2 = memory->readLongFromMemory(PC);
					result = data - data2;
					mostSignificantBitResult = (result >> 31) & 1;
					mostSignificantBitSource = (data2 >> 31) & 1;
					PC += 2;
				}
				else {
					cout << "Invalid addressing mode" << endl;
					return false;
				}
				break;
			default:
				cout << "Invalid addressing mode" << endl;
				return false;
				break;
			}
			break;

		default:
			cout << "Unrecognised addressing mode" << endl;
			return false;
			break;
		}

		if (size == SIZE_BYTE || (size - 4) == SIZE_BYTE) {
			(uint8_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else if (size == SIZE_WORD || (size - 4) == SIZE_WORD) {
			(uint16_t)result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}
		else {
			result == 0 ? SR |= 1 << SR_CCR_ZERO : SR &= ~(1 << SR_CCR_ZERO);
		}

		mostSignificantBitResult == 1 ? SR |= 1 << SR_CCR_NEGATIVE : SR &= ~(1 << SR_CCR_NEGATIVE);

		if (mostSignificantBitDestination == 0 && mostSignificantBitResult == 1)
			SR |= 1 << SR_CCR_CARRY;
		else
			SR &= ~(1 << SR_CCR_CARRY);

		if (mostSignificantBitSource != mostSignificantBitDestination)
			mostSignificantBitDestination != mostSignificantBitResult ? SR |= 1 << SR_CCR_OVERFLOW : SR &= ~(1 << SR_CCR_OVERFLOW);

		return true;
	}

	// Bcc (Branch Conditionally)
	if ((instruction & 0xF000) == Bcc) {
		int condition = (instruction >> 8) & 0xF;

		displacement = instruction & 0xFF;

		if (displacement == 0) {
			PC += 2;
			displacement = memory->readWordFromMemory(PC);
		}

		if (debugMode) {
			cout << "We have a conditional branch" << endl;
			cout << "Displacement: " << hex << displacement << endl;
		}

		switch (condition) {
		case CONDITIONAL_CARRY_CLEAR:
			if (((SR >> SR_CCR_CARRY) & 1) == 0)
				PC += displacement;
			break;
		case CONDITIONAL_CARRY_SET:
			if (((SR >> SR_CCR_CARRY) & 1) == 1)
				PC += displacement;
			break;
		case CONDITIONAL_EQUAL:
			if (((SR >> SR_CCR_ZERO) & 1) == 1)
				PC += displacement;
			break;
		case CONDITIONAL_GREATER_OR_EQUAL:
			if ((((SR >> SR_CCR_NEGATIVE) & 1) == 0) && (((SR >> SR_CCR_OVERFLOW) & 1) == 0))
				PC += displacement;
			else if ((((SR >> SR_CCR_NEGATIVE) & 1) == 1) && (((SR >> SR_CCR_OVERFLOW) & 1) == 1))
				PC += displacement;
			break;
		case CONDITIONAL_GREATER_THAN:
			if (((SR >> SR_CCR_ZERO) & 1) == 1)
				break;
			if ((((SR >> SR_CCR_NEGATIVE) & 1) == 0) && (((SR >> SR_CCR_OVERFLOW) & 1) == 0))
				PC += displacement;
			else if ((((SR >> SR_CCR_NEGATIVE) & 1) == 1) && (((SR >> SR_CCR_OVERFLOW) & 1) == 1))
				PC += displacement;
			break;
		case CONDITIONAL_HIGH:
			if ((((SR >> SR_CCR_CARRY) & 1) == 0) && (((SR >> SR_CCR_ZERO) & 1) == 0))
				PC += displacement;
			break;
		case CONDITIONAL_LESS_OR_EQUAL:
			if ((((SR >> SR_CCR_NEGATIVE) & 1) == 1) && (((SR >> SR_CCR_OVERFLOW) & 1) == 0))
				PC += displacement;
			else if ((((SR >> SR_CCR_NEGATIVE) & 1) == 0) && (((SR >> SR_CCR_OVERFLOW) & 1) == 1))
				PC += displacement;
			else if (((SR >> SR_CCR_ZERO) & 1) == 1)
				PC += displacement;
			break;
		case CONDITIONAL_LOW_OR_SAME:
			if ((((SR >> SR_CCR_CARRY) & 1) == 1) && (((SR >> SR_CCR_ZERO) & 1) == 1))
				PC += displacement;
			break;
		case CONDITIONAL_LESS_THAN:
			if ((((SR >> SR_CCR_NEGATIVE) & 1) == 1) && (((SR >> SR_CCR_OVERFLOW) & 1) == 0))
				PC += displacement;
			else if ((((SR >> SR_CCR_NEGATIVE) & 1) == 0) && (((SR >> SR_CCR_OVERFLOW) & 1) == 1))
				PC += displacement;
			break;
		case CONDITIONAL_MINUS:
			if (((SR >> SR_CCR_NEGATIVE) & 1) == 1)
				PC += displacement;
			break;
		case CONDITIONAL_NOT_EQUAL:
			if (((SR >> SR_CCR_ZERO) & 1) == 0)
				PC += displacement;
			break;
		case CONDITIONAL_PLUS:
			if (((SR >> SR_CCR_NEGATIVE) & 1) == 0)
				PC += displacement;
			break;
		case CONDITIONAL_OVERFLOW_CLEAR:
			if (((SR >> SR_CCR_OVERFLOW) & 1) == 0)
				PC += displacement;
			break;
		case CONDITIONAL_OVERFLOW_SET:
			if (((SR >> SR_CCR_OVERFLOW) & 1) == 1)
				PC += displacement;
			break;
		default:
			cout << "Illegal condition code" << endl;
			return false;
		}

		return true;
	}

	// Illegal instruction
	cout << "Illegal instruction!" << endl;
	return false;
}

void CPUCore::writeByteToDataRegister(uint8_t data, int reg)
{
	D[reg] &= 0xFFFFFF00;
	D[reg] |= (uint32_t)data;
}

void CPUCore::writeWordToDataRegister(uint16_t data, int reg)
{
	D[reg] &= 0xFFFF0000;
	D[reg] |= (uint32_t)data;
}

void CPUCore::writeLongToDataRegister(uint32_t data, int reg)
{
	D[reg] = data;
}

void CPUCore::writeByteToAddressRegister(uint8_t data, int reg)
{
	A[reg] &= 0xFFFFFF00;
	A[reg] |= (uint32_t)data;
}

void CPUCore::writeWordToAddressRegister(uint16_t data, int reg)
{
	A[reg] &= 0xFFFF0000;
	A[reg] |= (uint32_t)data;
}

void CPUCore::writeLongToAddressRegister(uint32_t data, int reg)
{
	A[reg] = data;
}

void CPUCore::setProgramCounter(unsigned int memoryLocation) 
{
	PC = memoryLocation;
}

void CPUCore::displayInfo()
{
	cout << "Model: Motorola MC" << model << std::uppercase << endl << endl;
	cout << setfill('-') << setw(83) << "-" << endl;
	cout << setfill(' ') << std::left << setw(17) << "Register" << setw(17) << "Decimal" << setw(17) << "Hex" << setw(17) << "Binary" << endl;
	cout << setfill('-') << setw(83) << "-" << endl;
	cout << setfill(' ') << std::left << setw(17) << "D0" << setw(17) << dec << D[0] << setw(17) << hex << D[0] << setw(17) << bitset<32>(D[0]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D1" << setw(17) << dec << D[1] << setw(17) << hex << D[1] << setw(17) << bitset<32>(D[1]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D2" << setw(17) << dec << D[2] << setw(17) << hex << D[2] << setw(17) << bitset<32>(D[2]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D3" << setw(17) << dec << D[3] << setw(17) << hex << D[3] << setw(17) << bitset<32>(D[3]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D4" << setw(17) << dec << D[4] << setw(17) << hex << D[4] << setw(17) << bitset<32>(D[4]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D5" << setw(17) << dec << D[5] << setw(17) << hex << D[5] << setw(17) << bitset<32>(D[5]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D6" << setw(17) << dec << D[6] << setw(17) << hex << D[6] << setw(17) << bitset<32>(D[6]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "D7" << setw(17) << dec << D[7] << setw(17) << hex << D[7] << setw(17) << bitset<32>(D[7]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A0" << setw(17) << dec << A[0] << setw(17) << hex << A[0] << setw(17) << bitset<32>(A[0]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A1" << setw(17) << dec << A[1] << setw(17) << hex << A[1] << setw(17) << bitset<32>(A[1]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A2" << setw(17) << dec << A[2] << setw(17) << hex << A[2] << setw(17) << bitset<32>(A[2]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A3" << setw(17) << dec << A[3] << setw(17) << hex << A[3] << setw(17) << bitset<32>(A[3]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A4" << setw(17) << dec << A[4] << setw(17) << hex << A[4] << setw(17) << bitset<32>(A[4]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A5" << setw(17) << dec << A[5] << setw(17) << hex << A[5] << setw(17) << bitset<32>(A[5]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "A6" << setw(17) << dec << A[6] << setw(17) << hex << A[6] << setw(17) << bitset<32>(A[6]) << endl;
	cout << setfill(' ') << std::left << setw(17) << "SP" << setw(17) << dec << SP << setw(17) << hex << SP << setw(17) << bitset<32>(SP) << endl;
	cout << setfill(' ') << std::left << setw(17) << "PC" << setw(17) << dec << PC << setw(17) << hex << PC << setw(17) << bitset<32>(PC) << endl;
	cout << setfill(' ') << std::left << setw(17) << "SR" << setw(17) << dec << SR << setw(17) << hex << SR << setw(17) << bitset<32>(SR) << endl;
	cout << "                                                                   T S  III   XNZVC" << dec << endl << endl;
}
