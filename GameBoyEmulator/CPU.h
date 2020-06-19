#pragma once
#include "Memory.h"
#include "Instruction.h"
#include "Register.h"
#include <iostream>
#include <cstdint>
using namespace std;

enum CpuState{ RUNNING, INTERRUPT, STOP, HALT };

class CPU
{
	//Attributes
private:
	int CLOCK;
	Memory* memory;
	Register A, F, B, C, D, E, H, L;
	uint16_t programCounter = 0, stackPointer;
	CpuState cpuState = RUNNING;
	bool interruptsEnabled = true;
public:
	//Methods
	bool executeInstruction(Instruction instructionToExecute, uint16_t &PC, uint8_t nextOpCode, uint8_t* memory, int &cyclesLeft);
private:
	bool executeCBPrefixInstruction(Instruction instructionToExecute, uint16_t &PC, uint8_t* memory, int &cyclesLeft);
public:
	CPU(Memory* memPtr, int clock);
	void stepCPU();
	void setCpuState(CpuState newState);
	CpuState getCpuState();
	bool getInteruptStatus();
	void setInteruptStatus(bool newIntStatus);
};

CPU::CPU(Memory* memPtr, int clock)
{
	this->memory = memPtr;
	this->CLOCK = clock;
}
bool CPU::getInteruptStatus()
{
	return this->interruptsEnabled;
}
void CPU::setInteruptStatus(bool newIntStatus)
{
	this->interruptsEnabled = newIntStatus;
}
void CPU::stepCPU()
{
	//execute boot rom

	//execute rom code
	int cartSize = this->memory->getCartRomSize();
	uint8_t* cartRom = this->memory->getCartRom();
	bool instructionCaught = true;
	int cyclesLeft = 0;
	while(instructionCaught)
	{
		Instruction instruction;
		instruction.setOpCode(this->memory->read(this->programCounter));
		instructionCaught = this->executeInstruction(instruction, this->programCounter, cartRom[this->programCounter + 1], cartRom, cyclesLeft);
	}
}
void CPU::setCpuState(CpuState newState)
{
	this->cpuState = newState;
}
CpuState CPU::getCpuState()
{
	return this->cpuState;
}
bool CPU::executeInstruction(Instruction instruction, uint16_t &PC, uint8_t nextOpCode, uint8_t* memory, int &cyclesLeft)
{
	Instruction instructionPrefix;
	bool instructionCaught = false;
	uint8_t reg1 = 0, reg2 = 0, result = 0, msb = 0, lsb = 0;
	uint16_t reg116 = 0, reg216 = 0, addr = 0;


	switch (instruction.getOpCode())
	{
		// NOP Length: 1 Cycles 1 Opcode: 0x00 Flags: ----
	case (uint8_t)0x00: instruction.setMnemonic("NOP");// DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		instructionCaught = true; 
		break;
		// LD BC d16 Length: 3 Cycles 12 Opcode: 0x01 Flags: ----
	case (uint8_t)0x01: instruction.setMnemonic("LD BC d16");// DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->B.setValue(this->memory->read(PC));
		PC++;
		this->C.setValue(this->memory->read(PC));
		instructionCaught = true;
		break;
		// LD (BC), A Length: 1 Cycles 8 Opcode: 0x02 Flags: ----
	case (uint8_t)0x02: instruction.setMnemonic("LD (BC), A");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = (this->B.getValue() << 8 | (this->C.getValue() & 0xFF));
		this->memory->write(addr, this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// INC BC Length: 1 Cycles 8 Opcode: 0x03 Flags: ----
	case (uint8_t)0x03: instruction.setMnemonic("INC BC");//DONE
		cout << instruction.getMnemonic() << endl;
		//add B to A
		reg116 = (this->B.getValue()) << 8 | (this->C.getValue() & 0xFF);
		//INC Register
		reg116++;
		//set top 8 bits to B register
		this->B.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->C.setValue((uint8_t)reg116);
		instructionCaught = true;
		break;
		// INC B Length: 1 Cycles 4 Opcode: 0x04 Flags: Z0H-
	case (uint8_t)0x04: instruction.setMnemonic("INC B"); //DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->B.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->B.getValue();
		reg1++;
		this->B.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() & 0b10111111);
		//check for zero flag
		if(this->B.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// DEC B Length: 1 Cycles 4 Opcode: 0x05 Flags: Z1H-
	case (uint8_t)0x05: instruction.setMnemonic("DEC B");// DONE
		cout << instruction.getMnemonic() << endl;
		if (((this->B.getValue() & 0xF) + -1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->B.getValue();
		reg1--;
		this->B.setValue(reg1);
		this->F.setValue(this->F.getValue() & 0b10111111);
		if (this->B.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// LD B d8 Length: 2 Cycles 8 Opcode: 0x06 Flags: ----
	case (uint8_t)0x06: instruction.setMnemonic("LD B d8");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(memory[PC]);
		PC++;
		instructionCaught = true;
		break;
		// RLCA Length: 1 Cycles 4 Opcode: 0x07 Flags: 000C
	case (uint8_t)0x07: instruction.setMnemonic("RLCA");//DONE
		cout << instruction.getMnemonic() << endl;
		//set carry flag
		if (this->A.getValue() & 0b10000000 == 0x80)
			this->F.setValue(this->F.getValue() | 0x10);
		else
			this->F.setValue(this->F.getValue() | 0x7F);
		//get lowest bit
		lsb = this->A.getValue() & 1;
		this->A.setValue(this->A.getValue() >> 1 | lsb << 8);
		if(this->A.getValue() == 0x00)
			this->F.setValue(this->F.getValue() | 0x80);
		this->F.setValue(this->F.getValue() & 0x9F);
		instructionCaught = true;
		break;
		// LD (a16), SP Length: 3 Cycles 20 Opcode: 0x08 Flags: ----
	case (uint8_t)0x08: instruction.setMnemonic("LD (a16), SP");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->stackPointer = memory[PC] << 8 | (memory[PC + 1] & 0xFF);
		PC++;
		instructionCaught = true;
		break;
		// ADD HL BC Length: 1 Cycles 8 Opcode: 0x09 Flags: -0HC
	case (uint8_t)0x09: instruction.setMnemonic("ADD HL BC");//DONE
		cout << instruction.getMnemonic() << endl;
		//combine registers
		//HL
		reg116 = (this->H.getValue()) << 8 | (this->L.getValue() & 0xFF);
		//BC
		reg216 = (this->B.getValue()) << 8 | (this->C.getValue() & 0xFF);
		//check for half carry bit 11
		if((((reg116 & 0xFFF)+(reg216 & 0xFFF)) & 0x1000) == 0x1000)
			this->F.setValue(this->F.getValue() | 0b00100000);
		//check for carry (bit 15)
		if ((((((unsigned int)reg116) & 0xFFFF) + (((unsigned int)reg216) & 0xFFFF)) & 0x10000) == 0x10000)
			this->F.setValue(this->F.getValue() | 0b00010000);
		//set subtraction flag to 0
		this->F.setValue(this->F.getValue() & 0b10110000);
		//perform addtion
		result = reg116 + reg216;
		//store results.
		//set top 8 bits to B register
		this->B.setValue((uint8_t)(reg216 >> 8));
		//cutoff B bits
		reg216 = reg216 << 8;
		//shift C bits back
		reg216 = reg216 >> 8;
		//set C to lower half
		this->C.setValue((uint8_t)reg216);
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// LD A, (BC) Length: 1 Cycles 8 Opcode: 0x0A Flags: ----
	case (uint8_t)0x0A: instruction.setMnemonic("LD A, (BC)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->B.getValue() << 8 | (this->C.getValue() & 0xFF);
		this->A.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// DEC BC Length: 1 Cycles 8 Opcode: 0x0B Flags: ----
	case (uint8_t)0x0B: instruction.setMnemonic("DEC BC");//DONE
		cout << instruction.getMnemonic() << endl;
		//get both values
		//shift B left 7
		//add B to A
		reg116 = this->B.getValue() << 8 | (this->C.getValue() & 0xFF);
		reg116--;
		//set top 8 bits to B register
		this->B.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->C.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC C Length: 1 Cycles 4 Opcode: 0x0C Flags: Z0H-
	case (uint8_t)0x0C: instruction.setMnemonic("INC C");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->C.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->C.getValue();
		reg1++;
		this->C.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() & 0b10111111);
		//check for zero flag
		if (this->C.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// DEC C Length: 1 Cycles 4 Opcode: 0x0D Flags: Z1H-
	case (uint8_t)0x0D: instruction.setMnemonic("DEC C");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->C.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->C.getValue();
		reg1--;
		this->C.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() | 0b01000000);
		//check for zero flag
		if (this->C.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// LD C, d8 Length: 1 Cycles 4 Opcode: 0x0E Flags: ----
	case (uint8_t)0x0E: instruction.setMnemonic("LD C, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->C.setValue(memory[PC]);
		PC++;
		instructionCaught = true;
		break;
		// RRCA Length: 1 Cycles 4 Opcode: 0x0F Flags: Z00C
	case (uint8_t)0x0F: instruction.setMnemonic("RRCA");//DONE
		cout << instruction.getMnemonic() << endl;
		//Handle Carry
		lsb = this->A.getValue() & 0x1;
		if (lsb == 1)
			this->F.setValue(this->F.getValue() | 0b00010000);
		else
			this->F.setValue(this->F.getValue() & 0b11100000);
		//shift right
		reg1 = this->A.getValue() << 1;
		//populate msb
		reg1 = reg1 | lsb >> 8;
		this->A.setValue(reg1);
		//check zero flag
		if(reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//reset sub and half carry
		this->F.setValue(this->F.getValue() & 0b10010000);
		PC++;
		instructionCaught = true;
		break;
		// STOP 0 Length: 2 Cycles 4 Opcode: 0x10 Flags: ----
	case (uint8_t)0x10: instruction.setMnemonic("STOP 0");//DONE
		cout << instruction.getMnemonic() << endl;
		this->setCpuState(STOP);
		PC++;
		instructionCaught = true;
		break;
		// LD DE, d16 Length: 3 Cycles 12 Opcode: 0x11 Flags: ----
	case (uint8_t)0x11: instruction.setMnemonic("LD DE, d16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->D.setValue(memory[PC]);
		PC++;
		this->E.setValue(memory[PC]);
		instructionCaught = true;
		break;
		// LD (DE), A Length: 1 Cycles 8 Opcode: 0x12 Flags: ----
	case (uint8_t)0x12: instruction.setMnemonic("LD (DE), A");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->D.getValue() << 8 | (this->E.getValue() & 0xFF);
		this->memory->write(addr, this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// INC DE Length: 1 Cycles 8 Opcode: 0x13 Flags: ----
	case (uint8_t)0x13: instruction.setMnemonic("INC DE");//DONE
		cout << instruction.getMnemonic() << endl;
		//get both values
		//shift D left 7
		//add D to E
		reg116 = this->D.getValue() << 8 | (this->E.getValue() & 0xFF);
		//DEC Register
		reg116++;
		//set top 8 bits to B register
		this->D.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->E.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC D Length: 1 Cycles 4 Opcode: 0x14 Flags: Z0H-
	case (uint8_t)0x14: instruction.setMnemonic("INC D");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->D.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->D.getValue();
		reg1++;
		this->D.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() & 0b10111111);
		//check for zero flag
		if (this->D.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// DEC D Length: 1 Cycles 4 Opcode: 0x15 Flags: Z1H-
	case (uint8_t)0x15: instruction.setMnemonic("DEC D");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->D.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->D.getValue();
		reg1--;
		this->D.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() | 0b01000000);
		//check for zero flag
		if (this->D.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// LD D, d8 Length: 2 Cycles 8 Opcode: 0x16 Flags: ----
	case (uint8_t)0x16: instruction.setMnemonic("LD D, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->D.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// RLA Length: 1 Cycles 4 Opcode: 0x17 Flags: Z00C
	case (uint8_t)0x17: instruction.setMnemonic("RLA");//DONE
		cout << instruction.getMnemonic() << endl;
		//msb to carry
		msb = this->A.getValue() << 7;
		if (msb == 0x01)
			this->F.setValue(this->F.getValue() | 0b00010000);
		else
			this->F.setValue(this->F.getValue() & 0b11100000);
		reg1 = this->A.getValue() >> 1;
		if(reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() & 0b00010000);
		this->A.setValue(reg1);
		instructionCaught = true;
		break;
		// JR r8 Length: 2 Cycles 12 Opcode: 0x18 Flags: ----
	case (uint8_t)0x18: instruction.setMnemonic("JR r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		//read in offset
		reg1 = this->memory->read(PC);
		reg116 = 0;
		if ((reg1 & 0x80) == 0x80)
		{
			//sign extend offset
			reg116 = 0xFF00 | reg1;
		}
		else
			reg116 = 0x0000 | reg1;
		//perform "unsigned" addition to program counter
		PC = PC + reg116;
		//do not increment program counter
		instructionCaught = true;
		break;
		// ADD HL, DE Length: 1 Cycles 8 Opcode: 0x19 Flags: -0HC
	case (uint8_t)0x19: instruction.setMnemonic("ADD HL, DE");//DONE
		cout << instruction.getMnemonic() << endl;
		//combine registers
		//HL
		reg116 = (this->H.getValue()) << 8 | (this->L.getValue() & 0xFF);
		//BC
		reg216 = (this->D.getValue()) << 8 | (this->E.getValue() & 0xFF);
		//check for half carry bit 11
		if ((((reg116 & 0xFFF) + (reg216 & 0xFFF)) & 0x1000) == 0x1000)
			this->F.setValue(this->F.getValue() | 0b00100000);
		//check for carry (bit 15)
		if ((((((unsigned int)reg116) & 0xFFFF) + (((unsigned int)reg216) & 0xFFFF)) & 0x10000) == 0x10000)
			this->F.setValue(this->F.getValue() | 0b00010000);
		//set subtraction flag to 0
		this->F.setValue(this->F.getValue() & 0b10110000);
		//perform addtion
		result = reg116 + reg216;
		//store results.
		//set top 8 bits to B register
		this->D.setValue((uint8_t)(reg216 >> 8));
		//cutoff B bits
		reg216 = reg216 << 8;
		//shift C bits back
		reg216 = reg216 >> 8;
		//set C to lower half
		this->E.setValue((uint8_t)reg216);
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		instructionCaught = true;
		break;
		// LD A, (DE) Length: 1 Cycles 8 Opcode: 0x1A Flags: ----
	case (uint8_t)0x1A: instruction.setMnemonic("LD A, (DE)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->D.getValue() << 8 | (this->E.getValue() & 0xFF);
		this->A.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// DEC DE Length: 1 Cycles 8 Opcode: 0x1B Flags: ----
	case (uint8_t)0x1B: instruction.setMnemonic("DEC DE");//DONE
		cout << instruction.getMnemonic() << endl;
		//get both values
		//shift D left 7
		//add D to E
		reg116 = this->D.getValue() << 8 | (this->E.getValue() & 0xFF);
		//DEC Register
		reg116--;
		//set top 8 bits to B register
		this->D.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->C.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC E Length: 1 Cycles 4 Opcode: 0x1C Flags: Z0H-
	case (uint8_t)0x1C: instruction.setMnemonic("INC E");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->E.getValue();
		if (((reg1 & 0x0F) + 1) == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//increment
		reg1++;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b00110000);
		this->E.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// DEC E Length: 1 Cycles 4 Opcode: 0x1D Flags: Z1H-
	case (uint8_t)0x1D: instruction.setMnemonic("DEC E");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->E.getValue();
		if (((reg1 & (uint8_t)0x0F) - (uint8_t)1) == (uint8_t)0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//decrement
		reg1--;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() & 0b01000000);
		this->F.setValue(this->F.getValue() & 0b01110000);
		this->E.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// LD E, d8 Length: 2 Cycles 8 Opcode: 0x1E Flags: ----
	case (uint8_t)0x1E: instruction.setMnemonic("LD E, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->E.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// RRA Length: 1 Cycles 4 Opcode: 0x1F Flags: 000C
	case (uint8_t)0x1F: instruction.setMnemonic("RRA");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		lsb = reg1 & (uint8_t)0x1;
		if (lsb == 0x0)
			this->F.setValue(this->F.getValue() | 0b00010000);
		else
			this->F.setValue(this->F.getValue() & 0b11100000);
		this->F.setValue(this->F.getValue() & 0b00010000);
		PC++;
		instructionCaught = true;
		break;
		// JR NZ, r8 Length: 2 Cycles 12/8 Opcode: 0x20 Flags: ----
	case (uint8_t)0x20: instruction.setMnemonic("JR NZ, r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0b10000000) == 0x00)
		{
			//get Data
			reg1 = this->memory->read(PC);
			//Extend Sign
			if ((reg1 & 0x80) == 0x80)
			{
				//sign extend offset
				reg116 = 0xFF00 | reg1;
			}
			else
				reg116 = 0x0000 | reg1;
			//perform jump
			PC = PC + reg116;
		}
		else
			PC++;
		instructionCaught = true;
		break;
		// LD HL, d16 Length: 3 Cycles 12 Opcode: 0x21 Flags: ----
	case (uint8_t)0x21: instruction.setMnemonic("LD HL, d16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->H.setValue(this->memory->read(PC));
		PC++;
		this->L.setValue(this->memory->read(PC));
		instructionCaught = true;
		PC++;
		break;
		// LD (HL+), A Length: 1 Cycles 8 Opcode: 0x22 Flags: ----
	case (uint8_t)0x22: instruction.setMnemonic("LD (HL+), A");//DONE
		cout << instruction.getMnemonic() << endl;
		//perform load//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(reg116, this->A.getValue());
		//DEC Register
		reg116++;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC HL Length: 1 Cycles 8 Opcode: 0x23 Flags: ----
	case (uint8_t)0x23: instruction.setMnemonic("INC HL");//DONE
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		//DEC Register
		reg116++;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		break;
		// INC H Length: 1 Cycles 4 Opcode: 0x24 Flags: Z0H-
	case (uint8_t)0x24: instruction.setMnemonic("INC H");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->H.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->H.getValue();
		reg1++;
		this->H.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() & 0b10111111);
		//check for zero flag
		if (this->H.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// DEC H Length: 1 Cycles 4 Opcode: 0x25 Flags: Z1H-
	case (uint8_t)0x25: instruction.setMnemonic("DEC H");//DONE
		cout << instruction.getMnemonic() << endl;
		//half carry
		if (((this->H.getValue() & 0xF) + 1) & 0x10 == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		reg1 = this->H.getValue();
		reg1--;
		this->H.setValue(reg1);
		//set subtraction flag zero
		this->F.setValue(this->F.getValue() | 0b01000000);
		//check for zero flag
		if (this->H.getValue() == (uint8_t)0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		instructionCaught = true;
		break;
		// LD H, d8 Length: 2 Cycles 8 Opcode: 0x26 Flags: ----
	case (uint8_t)0x26: instruction.setMnemonic("LD H, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->H.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// DAA Length: 1 Cycles 4 Opcode: 0x27 Flags: Z-0C
	case (uint8_t)0x27: instruction.setMnemonic("DAA");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if (this->F.getValue() & 0b01000000 == 0)
		{
			//carry
			if (this->F.getValue() & 0b00010000 == 1 || reg1 > 0x99)
			{
				reg1 += 0x60;
				this->F.setValue(this->F.getValue() | 0b00010000);
			}
			//halfcarry
			if (this->F.getValue() & 0b00100000 == 1 || (reg1 & 0xF) > 0x09)
			{
				reg1 += 0x06;
			}
		}
		else
		{
			//carry
			if (this->F.getValue() & 0b00010000 == 1)
			{
				reg1 -= 0x60;
			}
			//halfcarry
			if (this->F.getValue() & 0b00100000 == 1)
			{
				reg1 -= 0x06;
			}
		}
		if(reg1 == 0)
			this->F.setValue(this->F.getValue() | 0b10000000);
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b11010000);
		PC++;
		instructionCaught = true;
		break;
		// JR Z, r8 Length: 2 Cycles 12/8 Opcode: 0x28 Flags: ----
	case (uint8_t)0x28: instruction.setMnemonic("JR Z, r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if (this->F.getValue() & 0b10000000 == 0x80)
		{
			//get value
			reg1 = this->memory->read(PC);
			reg116 = 0;
			//check sign & Extend
			if (reg1 & 0x80 == 1)
			{
				reg116 = 0xFF00 | reg1;
			}
			else
			{
				reg116 = 0x0000 | reg1;
			}
		}
		else
			PC++;
		instructionCaught = true;
		break;
		// ADD HL, HL Length: 1 Cycles 8 Opcode: 0x29 Flags: -0HC
	case (uint8_t)0x29: instruction.setMnemonic("ADD HL,HL");//DONE
		cout << instruction.getMnemonic() << endl;
		//combine registers
		//HL
		reg116 = (this->H.getValue()) << 8 | (this->L.getValue() & 0xFF);
		//check for half carry bit 11
		if ((((reg116 & 0xFFF) + (reg116 & 0xFFF)) & 0x1000) == 0x1000)
			this->F.setValue(this->F.getValue() | 0b00100000);
		//check for carry (bit 15)
		if ((((((unsigned int)reg116) & 0xFFFF) + (((unsigned int)reg116) & 0xFFFF)) & 0x10000) == 0x10000)
			this->F.setValue(this->F.getValue() | 0b00010000);
		//set subtraction flag to 0
		this->F.setValue(this->F.getValue() & 0b10110000);
		//perform addtion
		result = reg116 + reg116;
		//store results.
		//set top 8 bits to H register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff H bits
		reg116 = reg116 << 8;
		//shift L bits back
		reg116 = reg116 >> 8;
		//set L to lower half
		this->L.setValue((uint8_t)reg116);
		instructionCaught = true;
		break;
		// LD A, (HL+) Length: 1 Cycles 8 Opcode: 0x2A Flags: ----
	case (uint8_t)0x2A: instruction.setMnemonic("LD A, (HL+)");//DONE
		cout << instruction.getMnemonic() << endl;
		//perform load//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->A.setValue(this->memory->read(reg116));
		//DEC Register
		reg116++;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// DEC HL Length: 1 Cycles 8 Opcode: 0x2B Flags: ----
	case (uint8_t)0x2B: instruction.setMnemonic("DEC HL");//DONE
		cout << instruction.getMnemonic() << endl;
		//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		//DEC Register
		reg116--;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC L Length: 1 Cycles 4 Opcode: 0x2C Flags: Z0H-
	case (uint8_t)0x2C: instruction.setMnemonic("INC L");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->L.getValue();
		if (((reg1 & 0x0F) + 1) == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//increment
		reg1++;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b00110000);
		//store value
		this->L.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// DEC L Length: 1 Cycles 4 Opcode: 0x2D Flags: Z1H-
	case (uint8_t)0x2D: instruction.setMnemonic("DEC L");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->L.getValue();
		if (((reg1 & (uint8_t)0x0F) - (uint8_t)1) == (uint8_t)0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//decrement
		reg1--;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() & 0b01000000);
		this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->L.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// LD L, d8 Length: 2 Cycles 8 Opcode: 0x2E Flags: ----
	case (uint8_t)0x2E: instruction.setMnemonic("LD L, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->L.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// CPL Length: 1 Cycles 4 Opcode: 0x2F Flags: -11-
	case (uint8_t)0x2F: instruction.setMnemonic("CPL");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() ^ 0b11111111);
		this->F.setValue(this->F.getValue() | 0b01100000);
		instructionCaught = true;
		break;
		// JR NC, r8 Length: 2 Cycles 12/8 Opcode: 0x30 Flags: ----
	case (uint8_t)0x30: instruction.setMnemonic("JR NC, r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0b00010000) == 0x00)
		{
			//get Data
			reg1 = this->memory->read(PC);
			//Extend Sign
			if ((reg1 & 0x80) == 0x80)
			{
				//sign extend offset
				reg116 = 0xFF00 | reg1;
			}
			else
				reg116 = 0x0000 | reg1;
			//perform jump
			PC += reg116;
		}
		else
			PC++;
		instructionCaught = true;
		break;
		// LD HL, d16 Length: 3 Cycles 12 Opcode: 0x31 Flags: ----
	case (uint8_t)0x31: instruction.setMnemonic("LD HL, d16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->H.setValue(this->memory->read(PC));
		PC++;
		this->L.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// LD (HL-), A Length: 1 Cycles 8 Opcode: 0x32 Flags: ----
	case (uint8_t)0x32: instruction.setMnemonic("LD (HL-), A");//DONE
		cout << instruction.getMnemonic() << endl;
		//perform load//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(reg116, this->A.getValue());
		//DEC Register
		reg116--;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// INC SP Length: 1 Cycles 8 Opcode: 0x33 Flags: ----
	case (uint8_t)0x33: instruction.setMnemonic("INC SP");//DONE
		cout << instruction.getMnemonic() << endl;
		this->stackPointer++;
		PC++;
		instructionCaught = true;
		break;
		// INC (HL) Length: 1 Cycles 12 Opcode: 0x34 Flags: Z0H-
	case (uint8_t)0x34: instruction.setMnemonic("INC (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		//get HL Addres
		addr = this->H.getValue() >> 8 | (this->L.getValue() & 0xFF);
		//get value from ram
		reg1 = this->memory->read(addr);
		//check half carry
		if (((this->F.getValue() & 0xF) + 1) == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b10010000);
		//increment
		reg1++;
		//check zero & set sub flag
		if(reg1 == 0 )
			this->F.setValue(this->F.getValue() | 0b10000000);
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() & 0b10110000);
		//store value
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// DEC (HL) Length: 1 Cycles 8 Opcode: 0x35 Flags: Z1H-
	case (uint8_t)0x35: instruction.setMnemonic("DEC (HL)");//DONE
		//get HL Addres
		addr = this->H.getValue() >> 8 | (this->L.getValue() & 0xFF);
		//get value from ram
		reg1 = this->memory->read(addr);
		//check half carry
		if (((this->F.getValue() & 0xF) - 1) == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//increment
		reg1--;
		//check zero & set sub flag
		if (reg1 == 0)
			this->F.setValue(this->F.getValue() | 0b10000000);
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store value
		this->memory->write(addr, reg1);
		PC++;
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// LD (HL), d8 Length: 2 Cycles 12 Opcode: 0x36 Flags: ----
	case (uint8_t)0x36: instruction.setMnemonic("LD (HL), d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// SCF Length: 1 Cycles 8 Opcode: 0x37 Flags: -001
	case (uint8_t)0x37: instruction.setMnemonic("SCF");//DONE
		cout << instruction.getMnemonic() << endl;
		this->F.setValue(this->F.getValue() | 0b00010000);
		this->F.setValue(this->F.getValue() & 0b10010000);
		instructionCaught = true;
		break;
		// JR C, r8 Length: 2 Cycles 12/8 Opcode: 0x38 Flags: ----
	case (uint8_t)0x38: instruction.setMnemonic("JR C, r8");//DONE
		PC++;
		if (this->F.getValue() & 0x10 == 0x10)
		{
			reg1 = this->memory->read(PC);
			if ((reg1 & 0x80) == 0x80)
			{
				//sign extend
				reg116 = 0xFF00 | reg1;
			}
			else
			{
				reg116 = 0x0000 | reg1;
			}
			//perform arithmetic
			PC += reg116;
		}
		else
		{
			PC++;
		}
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// ADD HL, SP Length: 1 Cycles 8 Opcode: 0x39 Flags: -0HC
	case (uint8_t)0x39: instruction.setMnemonic("ADD HL, SP");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = this->H.getValue() >> 8 | (this->L.getValue() & 0xFF);
		//check for half carry bit 11
		if ((((reg116 & 0xFFF) + (stackPointer & 0xFFF)) & 0x1000) == 0x1000)
			this->F.setValue(this->F.getValue() | 0b00100000);
		//check for carry (bit 15)
		if ((((((unsigned int)reg116) & 0xFFFF) + (((unsigned int)stackPointer) & 0xFFFF)) & 0x10000) == 0x10000)
			this->F.setValue(this->F.getValue() | 0b00010000);
		//set subtraction flag to 0
		this->F.setValue(this->F.getValue() & 0b10110000);
		reg116 += stackPointer;
		//store value
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// LD A, (HL-) Length: 1 Cycles 8 Opcode: 0x3A Flags: ----
	case (uint8_t)0x3A: instruction.setMnemonic("LD A, (HL-)");//DONE
		cout << instruction.getMnemonic() << endl;
		//perform load//get both values
		//shift D left 7
		//add D to E
		reg116 = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->A.setValue(this->memory->read(reg116));
		//DEC Register
		reg116--;
		//set top 8 bits to B register
		this->H.setValue((uint8_t)(reg116 >> 8));
		//cutoff B bits
		reg116 = reg116 << 8;
		//shift C bits back
		reg116 = reg116 >> 8;
		//set C to lower half
		this->L.setValue((uint8_t)reg116);
		PC++;
		instructionCaught = true;
		break;
		// DEC SP Length: 1 Cycles 8 Opcode: 0x3B Flags: ----
	case (uint8_t)0x3B: instruction.setMnemonic("DEC SP");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer--;
		PC++;
		instructionCaught = true;
		break;
		// INC A Length: 1 Cycles 4 Opcode: 0x3C Flags: Z0H-
	case (uint8_t)0x3C: instruction.setMnemonic("INC A");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->A.getValue();
		if (((reg1 & 0x0F) + 1) == 0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//increment
		reg1++;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b00110000);
		//store
		this->A.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// DEC A Length: 1 Cycles 4 Opcode: 0x3D Flags: Z1H-
	case (uint8_t)0x3D: instruction.setMnemonic("DEC A");//DONE
		cout << instruction.getMnemonic() << endl;
		//check half carry
		reg1 = this->A.getValue();
		if (((reg1 & (uint8_t)0x0F) - (uint8_t)1) == (uint8_t)0x10)
			this->F.setValue(this->F.getValue() | 0b00100000);
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//decrement
		reg1--;
		//check zero and set sub flag
		if (reg1 == 0x00)
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() & 0b01000000);
		this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(reg1);
		//increment ProgramCounter
		PC++;
		instructionCaught = true;
		break;
		// LD A, d8 Length: 2 Cycles 8 Opcode: 0x3E Flags: ----
	case (uint8_t)0x3E: instruction.setMnemonic("LD A, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		this->A.setValue(this->memory->read(PC));
		PC++;
		instructionCaught = true;
		break;
		// CCF Length: 1 Cycles 4 Opcode: 0x3F Flags: -00C
	case (uint8_t)0x3F: instruction.setMnemonic("CCF");//DONE
		cout << instruction.getMnemonic() << endl;
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// LD B, B Length: 1 Cycles 4 Opcode: 0x40 Flags: ----
	case (uint8_t)0x40: instruction.setMnemonic("LD B, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD B, C Length: 1 Cycles 4 Opcode: 0x41 Flags: ----
	case (uint8_t)0x41: instruction.setMnemonic("LD B, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD B, D Length: 1 Cycles 4 Opcode: 0x42 Flags: ----
	case (uint8_t)0x42: instruction.setMnemonic("LD B, D");//DONE
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// LD B, E Length: 1 Cycles 4 Opcode: 0x43 Flags: ----
	case (uint8_t)0x43: instruction.setMnemonic("LD B, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD B, H Length: 1 Cycles 4 Opcode: 0x44 Flags: ----
	case (uint8_t)0x44: instruction.setMnemonic("LD B, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD B, L Length: 1 Cycles 4 Opcode: 0x45 Flags: ----
	case (uint8_t)0x45: instruction.setMnemonic("LD B, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD B, (HL) Length: 1 Cycles 8 Opcode: 0x40 Flags: ----
	case (uint8_t)0x46: instruction.setMnemonic("LD B, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->B.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD B, A Length: 1 Cycles 4 Opcode: 0x47 Flags: ----
	case (uint8_t)0x47: instruction.setMnemonic("LD B, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, B Length: 1 Cycles 4 Opcode: 0x48 Flags: ----
	case (uint8_t)0x48: instruction.setMnemonic("LD C, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, C Length: 1 Cycles 4 Opcode: 0x49 Flags: ----
	case (uint8_t)0x49: instruction.setMnemonic("LD C, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, D Length: 1 Cycles 4 Opcode: 0x4A Flags: ----
	case (uint8_t)0x4A: instruction.setMnemonic("LD C, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, E Length: 1 Cycles 4 Opcode: 0x4B Flags: ----
	case (uint8_t)0x4B: instruction.setMnemonic("LD C, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, H Length: 1 Cycles 4 Opcode: 0x4C Flags: ----
	case (uint8_t)0x4C: instruction.setMnemonic("LD C, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD C, L Length: 1 Cycles 4 Opcode: 0x4D Flags: ----
	case (uint8_t)0x4D: instruction.setMnemonic("LD C, L");//DONE
		this->C.setValue(this->L.getValue());
		PC++;
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// LD C, (HL) Length: 1 Cycles 8 Opcode: 0x4E Flags: ----
	case (uint8_t)0x4E: instruction.setMnemonic("LD C, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->C.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD C, A Length: 1 Cycles 4 Opcode: 0x4F Flags: ----
	case (uint8_t)0x4F: instruction.setMnemonic("LD C, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, B Length: 1 Cycles 4 Opcode: 0x50 Flags: ----
	case (uint8_t)0x50: instruction.setMnemonic("LD D, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, C Length: 1 Cycles 4 Opcode: 0x51 Flags: ----
	case (uint8_t)0x51: instruction.setMnemonic("LD D, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, D Length: 1 Cycles 4 Opcode: 0x52 Flags: ----
	case (uint8_t)0x52: instruction.setMnemonic("LD D, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, E Length: 1 Cycles 4 Opcode: 0x53 Flags: ----
	case (uint8_t)0x53: instruction.setMnemonic("LD D, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, H Length: 1 Cycles 4 Opcode: 0x54 Flags: ----
	case (uint8_t)0x54: instruction.setMnemonic("LD D, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, L Length: 1 Cycles 4 Opcode: 0x55 Flags: ----
	case (uint8_t)0x55: instruction.setMnemonic("LD D, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD D, (HL) Length: 1 Cycles 8 Opcode: 0x56 Flags: ----
	case (uint8_t)0x56: instruction.setMnemonic("LD D, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->D.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD D, A Length: 1 Cycles 4 Opcode: 0x57 Flags: ----
	case (uint8_t)0x57: instruction.setMnemonic("LD D, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, B Length: 1 Cycles 4 Opcode: 0x58 Flags: ----
	case (uint8_t)0x58: instruction.setMnemonic("LD E, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, C Length: 1 Cycles 4 Opcode: 0x59 Flags: ----
	case (uint8_t)0x59: instruction.setMnemonic("LD E, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, D Length: 1 Cycles 4 Opcode: 0x5A Flags: ----
	case (uint8_t)0x5A: instruction.setMnemonic("LD E, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, E Length: 1 Cycles 4 Opcode: 0x5B Flags: ----
	case (uint8_t)0x5B: instruction.setMnemonic("LD E, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, H Length: 1 Cycles 4 Opcode: 0x5C Flags: ----
	case (uint8_t)0x5C: instruction.setMnemonic("LD E, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, L Length: 1 Cycles 4 Opcode: 0x5D Flags: ----
	case (uint8_t)0x5D: instruction.setMnemonic("LD E, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD E, (HL) Length: 1 Cycles 4 Opcode: 0x5E Flags: ----
	case (uint8_t)0x5E: instruction.setMnemonic("LD E, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->E.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD E, A Length: 1 Cycles 4 Opcode: 0x5F Flags: ----
	case (uint8_t)0x5F: instruction.setMnemonic("LD E, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, B Length: 1 Cycles 4 Opcode: 0x60 Flags: ----
	case (uint8_t)0x60: instruction.setMnemonic("LD H, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, C Length: 1 Cycles 4 Opcode: 0x61 Flags: ----
	case (uint8_t)0x61: instruction.setMnemonic("LD H, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, D Length: 1 Cycles 4 Opcode: 0x62 Flags: ----
	case (uint8_t)0x62: instruction.setMnemonic("LD H, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, E Length: 1 Cycles 4 Opcode: 0x63 Flags: ----
	case (uint8_t)0x63: instruction.setMnemonic("LD H, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, H Length: 1 Cycles 4 Opcode: 0x64 Flags: ----
	case (uint8_t)0x64: instruction.setMnemonic("LD H, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, L Length: 1 Cycles 4 Opcode: 0x65 Flags: ----
	case (uint8_t)0x65: instruction.setMnemonic("LD H, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD H, (HL) Length: 1 Cycles 8 Opcode: 0x66 Flags: ----
	case (uint8_t)0x66: instruction.setMnemonic("LD H, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->H.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD H, A Length: 1 Cycles 4 Opcode: 0x67 Flags: ----
	case (uint8_t)0x67: instruction.setMnemonic("LD H, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, B Length: 1 Cycles 4 Opcode: 0x68 Flags: ----
	case (uint8_t)0x68: instruction.setMnemonic("LD L, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, C Length: 1 Cycles 4 Opcode: 0x69 Flags: ----
	case (uint8_t)0x69: instruction.setMnemonic("LD L, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, D Length: 1 Cycles 4 Opcode: 0x6A Flags: ----
	case (uint8_t)0x6A: instruction.setMnemonic("LD L, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, E Length: 1 Cycles 4 Opcode: 0x6B Flags: ----
	case (uint8_t)0x6B: instruction.setMnemonic("LD L, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, H Length: 1 Cycles 4 Opcode: 0x6C Flags: ----
	case (uint8_t)0x6C: instruction.setMnemonic("LD L, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, L Length: 1 Cycles 4 Opcode: 0x6D Flags: ----
	case (uint8_t)0x6D: instruction.setMnemonic("LD L, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD L, (HL) Length: 1 Cycles 8 Opcode: 0x6E Flags: ----
	case (uint8_t)0x6E: instruction.setMnemonic("LD L, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->L.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD L, A Length: 1 Cycles 4 Opcode: 0x6F Flags: ----
	case (uint8_t)0x6F: instruction.setMnemonic("LD L, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), B Length: 1 Cycles 8 Opcode: 0x70 Flags: ----
	case (uint8_t)0x70: instruction.setMnemonic("LD (HL), B");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), C Length: 1 Cycles 8 Opcode: 0x71 Flags: ----
	case (uint8_t)0x71: instruction.setMnemonic("LD (HL), C");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), D Length: 1 Cycles 8 Opcode: 0x71 Flags: ----
	case (uint8_t)0x72: instruction.setMnemonic("LD (HL), D");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), E Length: 1 Cycles 8 Opcode: 0x73 Flags: ----
	case (uint8_t)0x73: instruction.setMnemonic("LD (HL), E");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), H Length: 1 Cycles 8 Opcode: 0x74 Flags: ----
	case (uint8_t)0x74: instruction.setMnemonic("LD (HL), H");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD (HL), L Length: 1 Cycles 8 Opcode: 0x75 Flags: ----
	case (uint8_t)0x75: instruction.setMnemonic("LD (HL), L");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// HALT Length: 1 Cycles 8 Opcode: 0x76 Flags: ----
	case (uint8_t)0x76: instruction.setMnemonic("HALT");
		cout << instruction.getMnemonic() << endl;
		this->cpuState = HALT;
		instructionCaught = true;
		break;
		// LD (HL), A Length: 1 Cycles 8 Opcode: 0x77 Flags: ----
	case (uint8_t)0x77: instruction.setMnemonic("LD (HL), A");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->memory->write(addr, this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, B Length: 1 Cycles 4 Opcode: 0x78 Flags: ----
	case (uint8_t)0x78: instruction.setMnemonic("LD A, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->B.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, C Length: 1 Cycles 4 Opcode: 0x79 Flags: ----
	case (uint8_t)0x79: instruction.setMnemonic("LD A, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, D Length: 1 Cycles 4 Opcode: 0x7A Flags: ----
	case (uint8_t)0x7A: instruction.setMnemonic("LD A, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->D.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, E Length: 1 Cycles 4 Opcode: 0x7B Flags: ----
	case (uint8_t)0x7B: instruction.setMnemonic("LD A, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, H Length: 1 Cycles 4 Opcode: 0x7C Flags: ----
	case (uint8_t)0x7C: instruction.setMnemonic("LD A, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->H.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, L Length: 1 Cycles 4 Opcode: 0x7D Flags: ----
	case (uint8_t)0x7D: instruction.setMnemonic("LD A, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// LD A, (HL) Length: 1 Cycles 8 Opcode: 0x7E Flags: ----
	case (uint8_t)0x7E: instruction.setMnemonic("LD A, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = this->H.getValue() << 8 | (this->L.getValue() & 0xFF);
		this->A.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// LD A, A Length: 1 Cycles 4 Opcode: 0x7F Flags: ----
	case (uint8_t)0x7F: instruction.setMnemonic("LD A, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// ADD A, B Length: 1 Cycles 4 Opcode: 0x80 Flags: Z0HC
	case (uint8_t)0x80: instruction.setMnemonic("ADD A, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, C Length: 1 Cycles 4 Opcode: 0x81 Flags: Z0HC
	case (uint8_t)0x81: instruction.setMnemonic("ADD A, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, D Length: 1 Cycles 4 Opcode: 0x82 Flags: Z0HC
	case (uint8_t)0x82: instruction.setMnemonic("ADD A, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, E Length: 1 Cycles 4 Opcode: 0x83 Flags: Z0HC
	case (uint8_t)0x83: instruction.setMnemonic("ADD A, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, H Length: 1 Cycles 4 Opcode: 0x84 Flags: Z0HC
	case (uint8_t)0x84: instruction.setMnemonic("ADD A, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, L Length: 1 Cycles 4 Opcode: 0x85 Flags: Z0HC
	case (uint8_t)0x85: instruction.setMnemonic("ADD A, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, (HL) Length: 1 Cycles 4 Opcode: 0x86 Flags: Z0HC
	case (uint8_t)0x86: instruction.setMnemonic("ADD A, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read(this->H.getValue() >> 8 | (this->L.getValue() & 0x00FF));
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADD A, A Length: 1 Cycles 4 Opcode: 0x87 Flags: Z0HC
	case (uint8_t)0x87: instruction.setMnemonic("ADD A, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, B Length: 1 Cycles 4 Opcode: 0x88 Flags: Z0HC
	case (uint8_t)0x88: instruction.setMnemonic("ADC A, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, C Length: 1 Cycles 4 Opcode: 0x89 Flags: Z0HC
	case (uint8_t)0x89: instruction.setMnemonic("ADC A, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, D Length: 1 Cycles 4 Opcode: 0x8A Flags: Z0HC
	case (uint8_t)0x8A: instruction.setMnemonic("ADC A, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, E Length: 1 Cycles 4 Opcode: 0x8B Flags: Z0HC
	case (uint8_t)0x8B: instruction.setMnemonic("ADC A, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, H Length: 1 Cycles 4 Opcode: 0x8C Flags: Z0HC
	case (uint8_t)0x8C: instruction.setMnemonic("ADC A, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, L Length: 1 Cycles 4 Opcode: 0x8D Flags: Z0HC
	case (uint8_t)0x8D: instruction.setMnemonic("ADC A, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, (HL) Length: 1 Cycles 8 Opcode: 0x8E Flags: Z0HC
	case (uint8_t)0x8E: instruction.setMnemonic("ADC A, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue() >> 8 | (this->L.getValue() & 0xFF);
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// ADC A, A Length: 1 Cycles 4 Opcode: 0x8F Flags: Z0HC
	case (uint8_t)0x8F: instruction.setMnemonic("ADC A, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB B Length: 1 Cycles 4 Opcode: 0x90 Flags: Z1HC
	case (uint8_t)0x90: instruction.setMnemonic("SUB B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB C Length: 1 Cycles 4 Opcode: 0x91 Flags: Z1HC
	case (uint8_t)0x91: instruction.setMnemonic("SUB C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB D Length: 1 Cycles 4 Opcode: 0x92 Flags: Z1HC
	case (uint8_t)0x92: instruction.setMnemonic("SUB D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB E Length: 1 Cycles 4 Opcode: 0x93 Flags: Z1HC
	case (uint8_t)0x93: instruction.setMnemonic("SUB E");//DONE
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// SUB H Length: 1 Cycles 4 Opcode: 0x94 Flags: Z1HC
	case (uint8_t)0x94: instruction.setMnemonic("SUB H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB L Length: 1 Cycles 4 Opcode: 0x95 Flags: Z1HC
	case (uint8_t)0x95: instruction.setMnemonic("SUB L");//DONE
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// SUB (HL) Length: 1 Cycles 8 Opcode: 0x96 Flags: Z1HC
	case (uint8_t)0x96: instruction.setMnemonic("SUB (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SUB A Length: 1 Cycles 4 Opcode: 0x97 Flags: Z1HC
	case (uint8_t)0x97: instruction.setMnemonic("SUB A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, B Length: 1 Cycles 4 Opcode: 0x98 Flags: Z1HC
	case (uint8_t)0x98: instruction.setMnemonic("SBC A, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, C Length: 1 Cycles 4 Opcode: 0x99 Flags: Z1HC
	case (uint8_t)0x99: instruction.setMnemonic("SBC A, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, D Length: 1 Cycles 4 Opcode: 0x9A Flags: Z1HC
	case (uint8_t)0x9A: instruction.setMnemonic("SBC A, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, E Length: 1 Cycles 4 Opcode: 0x9B Flags: Z1HC
	case (uint8_t)0x9B: instruction.setMnemonic("SBC A, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, H Length: 1 Cycles 4 Opcode: 0x9C Flags: Z1HC
	case (uint8_t)0x9C: instruction.setMnemonic("SBC A, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, L Length: 1 Cycles 4 Opcode: 0x9D Flags: Z1HC
	case (uint8_t)0x9D: instruction.setMnemonic("SBC A, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, (HL) Length: 1 Cycles 8 Opcode: 0x9E Flags: Z1HC
	case (uint8_t)0x9E: instruction.setMnemonic("SBC A, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SBC A, A Length: 1 Cycles 4 Opcode: 0x9F Flags: Z1HC
	case (uint8_t)0x9F: instruction.setMnemonic("SBC A, A");//DONE
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		break;
		// AND B Length: 1 Cycles 4 Opcode: 0xA0 Flags: Z010
	case (uint8_t)0xA0: instruction.setMnemonic("AND B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND C Length: 1 Cycles 4 Opcode: 0xA1 Flags: Z010
	case (uint8_t)0xA1: instruction.setMnemonic("AND C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND D Length: 1 Cycles 4 Opcode: 0xA2 Flags: Z010
	case (uint8_t)0xA2: instruction.setMnemonic("AND D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND E Length: 1 Cycles 4 Opcode: 0xA3 Flags: Z010
	case (uint8_t)0xA3: instruction.setMnemonic("AND E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND H Length: 1 Cycles 4 Opcode: 0xA4 Flags: Z010
	case (uint8_t)0xA4: instruction.setMnemonic("AND H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND L Length: 1 Cycles 4 Opcode: 0xA5 Flags: Z010
	case (uint8_t)0xA5: instruction.setMnemonic("AND L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND (HL) Length: 1 Cycles 8 Opcode: 0xA0 Flags: Z010
	case (uint8_t)0xA6: instruction.setMnemonic("AND (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// AND A Length: 1 Cycles 4 Opcode: 0xA7 Flags: Z010
	case (uint8_t)0xA7: instruction.setMnemonic("AND A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		result = reg1 & reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		this->F.setValue(this->F.getValue() & 0b00100000);
		PC++;
		instructionCaught = true;
		break;
		// XOR B Length: 1 Cycles 4 Opcode: 0xA8 Flags: Z000
	case (uint8_t)0xA8: instruction.setMnemonic("XOR B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR C Length: 1 Cycles 4 Opcode: 0xA9 Flags: Z000
	case (uint8_t)0xA9: instruction.setMnemonic("XOR C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR D Length: 1 Cycles 4 Opcode: 0xAA Flags: Z000
	case (uint8_t)0xAA: instruction.setMnemonic("XOR D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR E Length: 1 Cycles 4 Opcode: 0xAB Flags: Z000
	case (uint8_t)0xAB: instruction.setMnemonic("XOR E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR H Length: 1 Cycles 4 Opcode: 0xAC Flags: Z000
	case (uint8_t)0xAC: instruction.setMnemonic("XOR H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR L Length: 1 Cycles 4 Opcode: 0xAD Flags: Z000
	case (uint8_t)0xAD: instruction.setMnemonic("XOR L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR (HL) Length: 1 Cycles 8 Opcode: 0xAE Flags: Z000
	case (uint8_t)0xAE: instruction.setMnemonic("XOR (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// XOR A Length: 1 Cycles 4 Opcode: 0xAF Flags: Z000
	case (uint8_t)0xAF: instruction.setMnemonic("XOR A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		result = reg1 ^ reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR B Length: 1 Cycles 4 Opcode: 0xB0 Flags: Z000
	case (uint8_t)0xB0: instruction.setMnemonic("OR B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR C Length: 1 Cycles 4 Opcode: 0xB1 Flags: Z000
	case (uint8_t)0xB1: instruction.setMnemonic("OR C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR D Length: 1 Cycles 4 Opcode: 0xB2 Flags: Z000
	case (uint8_t)0xB2: instruction.setMnemonic("OR D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR E Length: 1 Cycles 4 Opcode: 0xB3 Flags: Z000
	case (uint8_t)0xB3: instruction.setMnemonic("OR E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR H Length: 1 Cycles 4 Opcode: 0xB4 Flags: Z000
	case (uint8_t)0xB4: instruction.setMnemonic("OR H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR L Length: 1 Cycles 4 Opcode: 0xB5 Flags: Z000
	case (uint8_t)0xB5: instruction.setMnemonic("OR L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR (HL) Length: 1 Cycles 8 Opcode: 0xB6 Flags: Z000
	case (uint8_t)0xB6: instruction.setMnemonic("OR (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// OR A Length: 1 Cycles 4 Opcode: 0xB7 Flags: Z000
	case (uint8_t)0xB7: instruction.setMnemonic("OR A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		result = reg1 | reg2;
		this->A.setValue(result);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		PC++;
		instructionCaught = true;
		break;
		// CP B Length: 1 Cycles 4 Opcode: 0xB8 Flags: Z1HC
	case (uint8_t)0xB8: instruction.setMnemonic("CP B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->B.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}		
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP C Length: 1 Cycles 4 Opcode: 0xB9 Flags: Z1HC
	case (uint8_t)0xB9: instruction.setMnemonic("CP C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->C.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP D Length: 1 Cycles 4 Opcode: 0xB8 Flags: Z1HC
	case (uint8_t)0xBA: instruction.setMnemonic("CP D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->D.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP E Length: 1 Cycles 4 Opcode: 0xBB Flags: Z1HC
	case (uint8_t)0xBB: instruction.setMnemonic("CP E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->E.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP H Length: 1 Cycles 4 Opcode: 0xBC Flags: Z1HC
	case (uint8_t)0xBC: instruction.setMnemonic("CP H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->H.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP L Length: 1 Cycles 4 Opcode: 0xBD Flags: Z1HC
	case (uint8_t)0xBD: instruction.setMnemonic("CP L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->L.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP (HL) Length: 1 Cycles 8 Opcode: 0xBE Flags: Z1HC
	case (uint8_t)0xBE: instruction.setMnemonic("CP (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->memory->read((this->H.getValue() >> 8) | (this->L.getValue() & 0xFF));
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// CP A Length: 1 Cycles 4 Opcode: 0xBF Flags: Z1HC
	case (uint8_t)0xBF: instruction.setMnemonic("CP A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue(), reg2 = this->A.getValue();
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RET NZ Length: 1 Cycles 20/8 Opcode: 0xC0 Flags: ----
	case (uint8_t)0xC0: instruction.setMnemonic("RET NZ");//DONE
		cout << instruction.getMnemonic() << endl;
		instructionCaught = true;
		if ((this->F.getValue() & 0x80) == 0x00)
		{
			PC = ((this->memory->read(stackPointer + 1) >> 8) & 0xFF00) | (this->memory->read(stackPointer) & 0x00FF);
			stackPointer += 2;
		}
		else
		{
			PC++;
		}
		break;
		// POP BC Length: 1 Cycles 12 Opcode: 0xC1 Flags: ----
	case (uint8_t)0xC1: instruction.setMnemonic("POP BC");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = ((this->memory->read(stackPointer + 1) >> 8) & 0xFF00) | (this->memory->read(stackPointer) & 0x00FF);
		this->B.setValue((reg116 & 0xFF00) << 8);
		this->C.setValue(reg116 & 0x00FF);
		stackPointer += 2;
		PC++;
		instructionCaught = true;
		break;
		// JP NZ, a16 Length: 3 Cycles 16/12 Opcode: 0xC2 Flags: ----
	case (uint8_t)0xC2: instruction.setMnemonic("JP NZ, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x80) == 0x00)
		{
			PC = (((this->memory->read(PC + 1)) >> 8) & 0xFF00 | ((PC) & 0x00FF));
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// JP a16 Length: 3 Cycles 16 Opcode: 0xC3 Flags: ----
	case (uint8_t)0xC3: instruction.setMnemonic("JP a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		PC = (((this->memory->read(PC + 1)) >> 8) & 0xFF00 | ((PC) & 0x00FF));
		instructionCaught = true;
		break;
		// CALL NZ, a16 Length: 3 Cycles 24/12 Opcode: 0xC4 Flags: ----
	case (uint8_t)0xC4: instruction.setMnemonic("CALL NZ, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x80) == 0x00)
		{
			//push return address to stack
			stackPointer--;
			this->memory->write(stackPointer, (PC & 0xFF00) << 8);
			stackPointer--;
			this->memory->write(stackPointer, (PC & 0x00FF));
			//jump to called function
			PC = (((this->memory->read(PC)) >> 8) & 0xFF00 | ((PC + 1) & 0x00FF));
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// PUSH BC Length: 1 Cycles 16 Opcode: 0xC5 Flags: ----
	case (uint8_t)0xC5: instruction.setMnemonic("PUSH BC");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer--;
		this->memory->write(stackPointer, this->B.getValue());
		stackPointer--;
		this->memory->write(stackPointer, this->C.getValue());
		PC++;
		instructionCaught = true;
		break;
		// ADD A, d8 Length: 2 Cycles 8 Opcode: 0xC6 Flags: Z0HC
	case (uint8_t)0xC6: instruction.setMnemonic("ADD A, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->A.getValue(), reg2 = this->memory->read(PC);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 00H Length: 1 Cycles 16 Opcode: 0xC7 Flags: ----
	case (uint8_t)0xC7: instruction.setMnemonic("RST 00H");//DONE
		cout << instruction.getMnemonic() << endl;
		//push PC onto Stack
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0000;
		instructionCaught = true;
		break;
		// RET Z Length: 1 Cycles 20/8 Opcode: 0xC8 Flags: ----
	case (uint8_t)0xC8: instruction.setMnemonic("RET Z");//DONE
		cout << instruction.getMnemonic() << endl;
		if (this->F.getValue() & 0x80 == 0x80)
		{
			//pop address from stack
			PC = ((this->memory->read(stackPointer + 1) & 0xFF00) >> 8) | (this->memory->read(stackPointer) & 0x00FF);
			stackPointer += 2;
		}
		else
			PC++;
		instructionCaught = true;
		break;
		// RET Length: 1 Cycles 16 Opcode: 0xC9 Flags: ----
	case (uint8_t)0xC9: instruction.setMnemonic("RET");//DONE
		cout << instruction.getMnemonic() << endl;
		//pop address from stack
		PC = ((this->memory->read(stackPointer + 1) & 0xFF00) >> 8) | (this->memory->read(stackPointer) & 0x00FF);
		stackPointer += 2;
		instructionCaught = true;
		break;
		// JP Z, a16 Length: 3 Cycles 16/12 Opcode: 0xCA Flags: ----
	case (uint8_t)0xCA: instruction.setMnemonic("JP Z, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if (this->F.getValue() & 0x80 == 0x80)
		{
			PC = ((this->memory->read(PC + 1) & 0xFF00) >> 8) | (this->memory->read(PC) & 0x00FF);
		}
		else
			PC += 2;
		instructionCaught = true;
		break;
		// PREFIX CB Length: 1 Cycles 4 Opcode: 0xCB Flags: ----
	case (uint8_t)0xCB: instruction.setMnemonic("PREFIX CB");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		instructionPrefix.setOpCode(nextOpCode);
		instructionCaught = executeCBPrefixInstruction(instructionPrefix, PC, memory, cyclesLeft);
		break;
		// CALL Z, a16 Length: 3 Cycles 16/12 Opcode: 0xCC Flags: ----
	case (uint8_t)0xCC: instruction.setMnemonic("CALL Z, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x80) == 0x80)
		{
			//push return address to stack
			stackPointer--;
			this->memory->write(stackPointer, (PC & 0xFF00) << 8);
			stackPointer--;
			this->memory->write(stackPointer, (PC & 0x00FF));
			//jump to called function
			PC = (((this->memory->read(PC + 1)) >> 8) & 0xFF00 | ((PC) & 0x00FF));
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// CALL a16 Length: 3 Cycles 16/12 Opcode: 0xCD Flags: ----
	case (uint8_t)0xCD: instruction.setMnemonic("CALL a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		//push return address to stack
		stackPointer--;
		this->memory->write(stackPointer, (PC & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (PC & 0x00FF));
		//jump to called function
		PC = (((this->memory->read(PC + 1)) >> 8) & 0xFF00 | ((PC) & 0x00FF));
		instructionCaught = true;
		break;
		// ADC A, d8 Length: 2 Cycles 8 Opcode: 0xCE Flags: Z0HC
	case (uint8_t)0xCE: instruction.setMnemonic("ADC A, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->A.getValue(), reg2 = this->memory->read(PC);
		lsb = ((this->F.getValue() & 0b00010000) == 0x10);
		//check half carry
		if (((reg1 & 0xF) + (reg2 & 0xF) + (lsb & 0xF)) == 0x10)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		//check carry
		if (((reg1 & 0xFF) + (reg2 & 0xFF) + (lsb & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 + reg2 + lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 08H Length: 1 Cycles 16 Opcode: 0xCF Flags: ----
	case (uint8_t)0xCF: instruction.setMnemonic("RST 08H");//DONE
		cout << instruction.getMnemonic() << endl;
		//push PC onto Stack
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0008;
		instructionCaught = true;
		break;
		// RET NC Length: 1 Cycles 20/8 Opcode: 0xD0 Flags: ----
	case (uint8_t)0xD0: instruction.setMnemonic("RET NC");//DONE
		cout << instruction.getMnemonic() << endl;
		if ((this->F.getValue() & 0x10) == 0x00)
		{
			//pop return address from stack pointer
			PC = (this->memory->read(stackPointer + 1) >> 8) & 0xFF00 | (this->memory->read(stackPointer) & 0x00FF);
			//adjust Stack Pointer
			stackPointer += 2;
		}
		else
		{
			PC++;
		}
		instructionCaught = true;
		break;
		// POP DE Length: 1 Cycles 12 Opcode: 0xD1 Flags: ----
	case (uint8_t)0xD1: instruction.setMnemonic("POP DE");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->memory->read(stackPointer));
		stackPointer++;
		this->D.setValue(this->memory->read(stackPointer));
		stackPointer++;
		PC++;
		instructionCaught = true;
		break;
		// JP NC, a16 Length: 3 Cycles 16/12 Opcode: 0xD2 Flags: ----
	case (uint8_t)0xD2: instruction.setMnemonic("JP NC, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x10) == 0x00)
		{
			//jump
			PC = (this->memory->read(PC + 1) >> 8) & 0xFF00 | (this->memory->read(PC) & 0x00FF);
		}
		else
		{
			PC++;
		}
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xD3:
		instructionCaught = true;
		break;
		// CALL NC, a16 Length: 3 Cycles 24/12 Opcode: 0xD4 Flags: ----
	case (uint8_t)0xD4: instruction.setMnemonic("CALL NC, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x10) == 0x00)
		{
			//push address of next instruction to stack
			reg116 = PC + 2;
			stackPointer--;
			this->memory->write(stackPointer, (reg116 & 0xFF00) >> 8);
			stackPointer--;
			this->memory->write(stackPointer, (reg116 & 0x00FF));
			//Jump to immeadiate address
			PC = (this->memory->read(PC + 1) >> 8) & 0xFF00 | (this->memory->read(PC) & 0x00FF);
			
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// PUSH DE Length: 1 Cycles 16 Opcode: 0xD5 Flags: ----
	case (uint8_t)0xD5: instruction.setMnemonic("PUSH DE");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer--;
		this->memory->write(stackPointer, this->D.getValue());
		stackPointer--;
		this->memory->write(stackPointer, this->E.getValue());
		PC++;
		instructionCaught = true;
		break;
		// SUB d8 Length: 2 Cycles 8 Opcode: 0xD6 Flags: Z1HC
	case (uint8_t)0xD6: instruction.setMnemonic("SUB d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->A.getValue(), reg2 = this->memory->read(PC);
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 10H Length: 1 Cycles 16 Opcode: 0xD7 Flags: ----
	case (uint8_t)0xD7: instruction.setMnemonic("RST 10H");//DONE
		cout << instruction.getMnemonic() << endl;
		//push PC onto Stack
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0010;
		instructionCaught = true;
		break;
		// RET C Length: 1 Cycles 20/8 Opcode: 0xD8 Flags: ----
	case (uint8_t)0xD8: instruction.setMnemonic("RET C");
		cout << instruction.getMnemonic() << endl;
		if ((this->F.getValue() & 0x10) == 0x10)
		{
			//pop address of next instruction from stack
			//Jump to immeadiate address
			PC = (this->memory->read(stackPointer + 1) >> 8) & 0xFF00 | (this->memory->read(stackPointer) & 0x00FF);
			stackPointer += 2;

		}
		else
		{
			PC++;
		}
		instructionCaught = true;
		break;
		// RET I Length: 1 Cycles 16 Opcode: 0xD9 Flags: ----
	case (uint8_t)0xD9: instruction.setMnemonic("RET I");//DONE
		cout << instruction.getMnemonic() << endl;
		this->setInteruptStatus(true);
		PC = (this->memory->read(stackPointer + 1) >> 8) & 0xFF00 | (this->memory->read(stackPointer) & 0x00FF);
		stackPointer += 2;
		PC++;
		instructionCaught = true;
		break;
		// JP C, a16 Length: 3 Cycles 16/12 Opcode: 0xDA Flags: ----
	case (uint8_t)0xDA: instruction.setMnemonic("JP C, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x10) == 0x10)
		{
			//Jump to immeadiate address
			PC = (this->memory->read(PC + 1) >> 8) & 0xFF00 | (this->memory->read(PC) & 0x00FF);
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xDB:
		instructionCaught = true;
		break;
		// CALL C, a16 Length: 3 Cycles 24/12 Opcode: 0xDC Flags: ----
	case (uint8_t)0xDC: instruction.setMnemonic("CALL C, a16");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		if ((this->F.getValue() & 0x10) == 0x10)
		{
			//push address of the next instructiont to the program counter
			reg116 = PC + 2;
			stackPointer--;
			this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
			stackPointer--;
			this->memory->write(stackPointer, (reg116 & 0x00FF));
			//Jump to immeadiate address
			PC = (this->memory->read(PC + 1) >> 8) & 0xFF00 | (this->memory->read(PC) & 0x00FF);
		}
		else
		{
			PC += 2;
		}
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xDD:
		instructionCaught = true;
		break;
		// SBC A, d8 Length: 2 Cycles 8 Opcode: 0xDE Flags: Z1HC
	case (uint8_t)0xDE: instruction.setMnemonic("SBC A, d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->A.getValue(), reg2 = this->memory->read(PC);
		lsb = (this->F.getValue() & 0b00010000 == 0x10);
		//check half carry for borrow
		if ((reg1 & 0xF) - ((reg2 & 0xF) + (lsb & 0xF)) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - ((reg2 & 0xFF) + (lsb & 0xFF)) < 0))
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//add
		result = reg1 - reg2 - lsb;
		//check zero
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 18H Length: 1 Cycles 16 Opcode: 0xDF Flags: ----
	case (uint8_t)0xDF: instruction.setMnemonic("RST 18H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0018;
		instructionCaught = true;
		break;
		// LDH (a8), A Length: 2 Cycles 12 Opcode: 0xE0 Flags: ----
	case (uint8_t)0xE0: instruction.setMnemonic("LDA (a8), A");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		this->memory->write((0xFF00 | (reg1 & 0x00FF)), this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// POP HL Length: 1 Cycles 12 Opcode: 0xE1 Flags: ----
	case (uint8_t)0xE1: instruction.setMnemonic("POP HL");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->memory->read(stackPointer));
		stackPointer++;
		this->H.setValue(this->memory->read(stackPointer));
		stackPointer++;
		PC++;
		instructionCaught = true;
		break;
		// LD (C), A Length: 1 Cycles 8 Opcode: 0xE2 Flags: ----
	case (uint8_t)0xE2: instruction.setMnemonic("LDA (C), A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->memory->write((0xFF00 | (this->C.getValue() & 0x00FF)), this->A.getValue());
		PC++;
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xE3:
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xE4:
		instructionCaught = true;
		break;
		// PUSH HL Length: 1 Cycles 16 Opcode: 0xE5 Flags: ----
	case (uint8_t)0xE5: instruction.setMnemonic("PUSH HL");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer--;
		this->memory->write(stackPointer, this->H.getValue());
		stackPointer--;
		this->memory->write(stackPointer, this->L.getValue());
		PC++;
		instructionCaught = true;
		break;
		// AND d8 Length: 2 Cycles 8 Opcode: 0xE6 Flags: Z010
	case (uint8_t)0xE6: instruction.setMnemonic("AND d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		result = this->A.getValue() & reg1;
		//check flags
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10100000);
		//store result
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 20H Length: 1 Cycles 16 Opcode: 0xE7 Flags: ----
	case (uint8_t)0xE7: instruction.setMnemonic("RST 20H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0020;
		instructionCaught = true;
		break;
		// ADD SP, r8 Length: 2 Cycles 16 Opcode: 0xE8 Flags: 00HC
	case (uint8_t)0xE8: instruction.setMnemonic("ADD SP, r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		//sign extend immediate
		if ((reg1 & 0x80) == 0x10)
		{
			reg116 = 0xFF00 | (reg1 & 0x00FF);
		}
		else
			reg116 = 0x0000 | (reg1 & 0x00FF);
		//check half carry
		if (((stackPointer & 0xFF) + (reg116 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//Check Carry
		if (((stackPointer & 0xFFFF) + (reg116 & 0xFFFF)) == 0x10000)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b11100000);
		//Set zero and sub flag
		this->F.setValue(this->F.getValue() & 0b00110000);
		//perform addition
		stackPointer += reg116;
		PC++;
		instructionCaught = true;
		break;
		// JP (HL) Length: 1 Cycles 4 Opcode: 0xE9 Flags: ----
	case (uint8_t)0xE9: instruction.setMnemonic("JP (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		PC = (this->H.getValue() >> 8) & 0xFF00 | (this->L.getValue() & 0x00FF);
		instructionCaught = true;
		break;
		// LD (a16), A Length: 3 Cycles 16 Opcode: 0xEA Flags: ----
	case (uint8_t)0xEA: instruction.setMnemonic("LD (a16), A");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		addr = (this->memory->read(PC + 1) >> 8) & 0xFF00 | (this->memory->read(PC) & 0x00FF);
		this->memory->write(addr, this->A.getValue());
		PC += 2;
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xEB:
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xEC:
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xED:
		instructionCaught = true;
		break;
		// XOR d8 Length: 2 Cycles 8 Opcode: 0xEE Flags: Z000
	case (uint8_t)0xEE: instruction.setMnemonic("XOR d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		result = this->A.getValue() ^ reg1;
		//check flags
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		//store result
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 28H Length: 1 Cycles 16 Opcode: 0xEF Flags: ----
	case (uint8_t)0xEF: instruction.setMnemonic("RST 28H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0028;
		instructionCaught = true;
		break;
		// LDH A, (a8) Length: 2 Cycles 12 Opcode: 0xF0 Flags: ----
	case (uint8_t)0xF0: instruction.setMnemonic("LDH A, (a8)");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		addr = 0xFF00 | (reg1 & 0x00FF);
		this->A.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// POP AF Length: 1 Cycles 12 Opcode: 0xF1 Flags: ZNHC
	case (uint8_t)0xF1: instruction.setMnemonic("POP AF");//DONE
		cout << instruction.getMnemonic() << endl;
		this->F.setValue(this->memory->read(stackPointer));
		stackPointer++;
		this->A.setValue(this->memory->read(stackPointer));
		stackPointer++;
		PC++;
		instructionCaught = true;
		break;
		// LD A, (C) Length: 2 Cycles 8 Opcode: 0xF2 Flags: ----
	case (uint8_t)0xF2: instruction.setMnemonic("LD A, (C)");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		addr = 0xFF00 | (reg1 & 0x00FF);
		this->A.setValue(this->memory->read(addr));
		PC++;
		instructionCaught = true;
		break;
		// DI Length: 1 Cycles 4 Opcode: 0xF3 Flags: ----
	case (uint8_t)0xF3: instruction.setMnemonic("DI");//DONE
		cout << instruction.getMnemonic() << endl;
		this->setInteruptStatus(false);
		PC++;
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xF4:
		instructionCaught = true;
		break;
		// PUSH AF Length: 1 Cycles 16 Opcode: 0xF5 Flags: ----
	case (uint8_t)0xF5: instruction.setMnemonic("PUSH AF");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer--;
		this->memory->write(stackPointer, this->A.getValue());
		stackPointer--;
		this->memory->write(stackPointer, this->F.getValue());
		PC++;
		instructionCaught = true;
		break;
		// OR d8 Length: 2 Cycles 8 Opcode: 0xF6 Flags: Z000
	case (uint8_t)0xF6: instruction.setMnemonic("OR d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		result = this->A.getValue() | reg1;
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b00000000);
		this->F.setValue(this->F.getValue() & 0b10000000);
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// RST 30H Length: 1 Cycles 16 Opcode: 0xF7 Flags: ----
	case (uint8_t)0xF7: instruction.setMnemonic("RST 30H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0030;
		instructionCaught = true;
		break;
		// LD HL, SP + r8 Length: 2 Cycles 12 Opcode: 0xF8 Flags: 00HC
	case (uint8_t)0xF8: instruction.setMnemonic("LD HL, PC+r8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->memory->read(PC);
		//sign extend immediate
		if ((reg1 & 0x80) == 0x10)
		{
			reg116 = 0xFF00 | (reg1 & 0x00FF);
		}
		else
			reg116 = 0x0000 | (reg1 & 0x00FF);
		//check half carry
		if (((stackPointer & 0xFF) + (reg116 & 0xFF)) == 0x100)
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b11010000);
		//Check Carry
		if (((stackPointer & 0xFFFF) + (reg116 & 0xFFFF)) == 0x10000)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b11100000);
		//Set zero and sub flag
		this->F.setValue(this->F.getValue() & 0b00110000);
		reg216 = stackPointer + reg116;
		this->H.setValue((reg216 & 0xFF00) << 8);
		this->L.setValue(reg216 & 0x00FF);
		PC++;
		instructionCaught = true;
		break;
		// LD SP, HL Length: 1 Cycles 8 Opcode: 0xF9 Flags: ----
	case (uint8_t)0xF9: instruction.setMnemonic("LD SP, HL");//DONE
		cout << instruction.getMnemonic() << endl;
		stackPointer = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		PC++;
		instructionCaught = true;
		break;
		// LD A, (a16) Length: 3 Cycles 16 Opcode: 0xFA Flags: ----
	case (uint8_t)0xFA: instruction.setMnemonic("LD A, (a16)");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		addr = ((this->memory->read(PC + 1) >> 8) & 0xFF00) | (this->memory->read(PC) & 0x00FF);
		this->A.setValue(this->memory->read(addr));
		PC += 2;
		instructionCaught = true;
		break;
		// EI Length: 1 Cycles 4 Opcode: 0xFB Flags: ----
	case (uint8_t)0xFB: instruction.setMnemonic("EI");//DONE
		cout << instruction.getMnemonic() << endl;
		this->setInteruptStatus(true);
		PC++;
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xFC:
		instructionCaught = true;
		break;
		// UNUSED
	case (uint8_t)0xFD:
		instructionCaught = true;
		break;
		// CP d8 Length: 2 Cycles 8 Opcode: 0xFE Flags: Z1HC
	case (uint8_t)0xFE: instruction.setMnemonic("CP d8");//DONE
		cout << instruction.getMnemonic() << endl;
		PC++;
		reg1 = this->A.getValue(), reg2 = this->memory->read(PC);
		//check half carry for borrow
		if ((reg1 & 0xF) - (reg2 & 0xF) < 0)
		{
			this->F.setValue(this->F.getValue() & 0b11010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() | 0b00100000);
		}
		//check carry
		if (((reg1 & 0xFF) - (reg2 & 0xFF)) < 0)
		{
			this->F.setValue(this->F.getValue() | 0b00010000);
		}
		else
		{
			this->F.setValue(this->F.getValue() & 0b11100000);
		}
		//check zero
		if (reg1 == reg2)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() & 0b01110000);
		//set sub flag
		this->F.setValue(this->F.getValue() | 0b01000000);
		//store
		this->A.setValue(result);
		PC++;
		PC++;
		instructionCaught = true;
		break;
		//  RST 38H Length: 1 Cycles: 16 Opcode: 0xFF Flags: ----
	case (uint8_t)0xFF: instruction.setMnemonic("RST 38H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg116 = PC + 1;
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0xFF00) << 8);
		stackPointer--;
		this->memory->write(stackPointer, (reg116 & 0x00FF));
		PC = 0x0038;
		instructionCaught = true;
		break;
	}
	if (!instructionCaught)
	{
		cout << "instrution unsupported opcode" << endl;
	}
	return instructionCaught;
}
bool CPU::executeCBPrefixInstruction(Instruction instruction, uint16_t &PC, uint8_t* memory, int &cyclesLeft)
{
	bool instructionCaught = false;
	uint8_t reg1 = 0, reg2 = 0, result = 0, msb = 0, lsb = 0;
	uint16_t reg116 = 0, reg216 = 0, addr = 0;
	switch (instruction.getOpCode())
	{
		// RLC B Length: 2 Cycles 8 Opcode: 0x00 Flags: Z00C
	case (uint8_t)0x00: instruction.setMnemonic("RLC B");
		cout << instruction.getMnemonic() << endl;

		PC++;
		instructionCaught = true;
		break;
		// RLC C Length: 2 Cycles 8 Opcode: 0x01 Flags: Z00C
	case (uint8_t)0x01: instruction.setMnemonic("RLC C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC D Length: 2 Cycles 8 Opcode: 0x02 Flags: Z00C
	case (uint8_t)0x02: instruction.setMnemonic("RLC D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC E Length: 2 Cycles 8 Opcode: 0x03 Flags: Z00C
	case (uint8_t)0x03: instruction.setMnemonic("RLC E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC H Length: 2 Cycles 8 Opcode: 0x04 Flags: Z00C
	case (uint8_t)0x04: instruction.setMnemonic("RLC H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC L Length: 2 Cycles 8 Opcode: 0x05 Flags: Z00C
	case (uint8_t)0x05: instruction.setMnemonic("RLC L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC (HL) Length: 2 Cycles 16 Opcode: 0x06 Flags: Z00C
	case (uint8_t)0x06: instruction.setMnemonic("RLC (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RLC A Length: 2 Cycles 8 Opcode: 0x07 Flags: Z00C
	case (uint8_t)0x07: instruction.setMnemonic("RLC A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC B Length: 2 Cycles 8 Opcode: 0x08 Flags: Z00C
	case (uint8_t)0x08: instruction.setMnemonic("LD (a16), PC");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC C Length: 2 Cycles 8 Opcode: 0x09 Flags: Z00C
	case (uint8_t)0x09: instruction.setMnemonic("RRC C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC D Length: 2 Cycles 8 Opcode: 0x0A Flags: Z00C
	case (uint8_t)0x0A: instruction.setMnemonic("RRC D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC E Length: 2 Cycles 8 Opcode: 0x0B Flags: Z00C
	case (uint8_t)0x0B: instruction.setMnemonic("RRC E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC H Length: 2 Cycles 8 Opcode: 0x0C Flags: Z00C
	case (uint8_t)0x0C: instruction.setMnemonic("RRC H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC L Length: 2 Cycles 8 Opcode: 0x0D Flags: Z00C
	case (uint8_t)0x0D: instruction.setMnemonic("RRC L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC (HL) Length: 2 Cycles 16 Opcode: 0x0E Flags: Z00C
	case (uint8_t)0x0E: instruction.setMnemonic("RRC (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RRC A Length: 2 Cycles 8 Opcode: 0x0F Flags: Z00C
	case (uint8_t)0x0F: instruction.setMnemonic("RRC A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL B Length: 2 Cycles 8 Opcode: 0x10 Flags: Z00C
	case (uint8_t)0x10: instruction.setMnemonic("RL B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL C Length: 2 Cycles 8 Opcode: 0x11 Flags: Z00C
	case (uint8_t)0x11: instruction.setMnemonic("RL C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL D Length: 2 Cycles 8 Opcode: 0x12 Flags: Z00C
	case (uint8_t)0x12: instruction.setMnemonic("RL D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL E Length: 2 Cycles 8 Opcode: 0x13 Flags: Z00C
	case (uint8_t)0x13: instruction.setMnemonic("RL E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL H Length: 2 Cycles 8 Opcode: 0x14 Flags: Z00C
	case (uint8_t)0x14: instruction.setMnemonic("RL H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL L Length: 2 Cycles 8 Opcode: 0x15 Flags: Z00C
	case (uint8_t)0x15: instruction.setMnemonic("RL L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL (HL) Length: 2 Cycles 16 Opcode: 0x16 Flags: Z00C
	case (uint8_t)0x16: instruction.setMnemonic("RL (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RL A Length: 2 Cycles 8 Opcode: 0x17 Flags: Z00C
	case (uint8_t)0x17: instruction.setMnemonic("RL A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR B Length: 2 Cycles 8 Opcode: 0x18 Flags: Z00C
	case (uint8_t)0x18: instruction.setMnemonic("RR B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR C Length: 2 Cycles 8 Opcode: 0x19 Flags: Z00C
	case (uint8_t)0x19: instruction.setMnemonic("RR C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR D Length: 2 Cycles 8 Opcode: 0x1A Flags: Z00C
	case (uint8_t)0x1A: instruction.setMnemonic("RR D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR E Length: 2 Cycles 8 Opcode: 0x1B Flags: Z00C
	case (uint8_t)0x1B: instruction.setMnemonic("RR E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR H Length: 2 Cycles 8 Opcode: 0x1C Flags: Z00C
	case (uint8_t)0x1C: instruction.setMnemonic("RR H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR L Length: 2 Cycles 8 Opcode: 0x1D Flags: Z00C
	case (uint8_t)0x1D: instruction.setMnemonic("RR L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR (HL) Length: 2 Cycles 8 Opcode: 0x1E Flags: Z00C
	case (uint8_t)0x1E: instruction.setMnemonic("RR (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// RR A Length: 2 Cycles 8 Opcode: 0x1F Flags: Z00C
	case (uint8_t)0x1F: instruction.setMnemonic("RR A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA B Length: 2 Cycles 8 Opcode: 0x20 Flags: Z00C
	case (uint8_t)0x20: instruction.setMnemonic("SLA B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA C Length: 2 Cycles 8 Opcode: 0x21 Flags: Z00C
	case (uint8_t)0x21: instruction.setMnemonic("SLA C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA D Length: 2 Cycles 8 Opcode: 0x22 Flags: Z00C
	case (uint8_t)0x22: instruction.setMnemonic("SLA D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA E Length: 2 Cycles 8 Opcode: 0x23 Flags: Z00C
	case (uint8_t)0x23: instruction.setMnemonic("SLA E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA H Length: 2 Cycles 8 Opcode: 0x24 Flags: Z00C
	case (uint8_t)0x24: instruction.setMnemonic("SLA H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA L Length: 2 Cycles 8 Opcode: 0x25 Flags: Z00C
	case (uint8_t)0x25: instruction.setMnemonic("SLA L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA (HL) Length: 2 Cycles 16 Opcode: 0x26 Flags: Z00C
	case (uint8_t)0x26: instruction.setMnemonic("SLA (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SLA A Length: 2 Cycles 8 Opcode: 0x27 Flags: Z00C
	case (uint8_t)0x27: instruction.setMnemonic("SLA A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA B Length: 2 Cycles 8 Opcode: 0x28 Flags: Z00C
	case (uint8_t)0x28: instruction.setMnemonic("SRA B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA C Length: 2 Cycles 8 Opcode: 0x29 Flags: Z00C
	case (uint8_t)0x29: instruction.setMnemonic("SRA C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA D Length: 2 Cycles 8 Opcode: 0x2A Flags: Z00C
	case (uint8_t)0x2A: instruction.setMnemonic("SRA D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA E Length: 2 Cycles 8 Opcode: 0x2B Flags: Z00C
	case (uint8_t)0x2B: instruction.setMnemonic("SRA E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA H Length: 2 Cycles 8 Opcode: 0x2C Flags: Z00C
	case (uint8_t)0x2C: instruction.setMnemonic("SRA H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA L Length: 2 Cycles 8 Opcode: 0x2D Flags: Z00C
	case (uint8_t)0x2D: instruction.setMnemonic("SRA L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA (HL) Length: 2 Cycles 16 Opcode: 0x2E Flags: Z00C
	case (uint8_t)0x2E: instruction.setMnemonic("SRA (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRA A Length: 2 Cycles 8 Opcode: 0x2F Flags: Z00C
	case (uint8_t)0x2F: instruction.setMnemonic("SRA A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SWAP B Length: 2 Cycles 8 Opcode: 0x30 Flags: Z000
	case (uint8_t)0x30: instruction.setMnemonic("SWAP B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue & 0x0F;
		reg2 = this->B.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->B.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP C Length: 2 Cycles 8 Opcode: 0x31 Flags: Z000
	case (uint8_t)0x31: instruction.setMnemonic("SWAP C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue & 0x0F;
		reg2 = this->C.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->C.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP D Length: 2 Cycles 8 Opcode: 0x32 Flags: Z000
	case (uint8_t)0x32: instruction.setMnemonic("SWAP D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue & 0x0F;
		reg2 = this->D.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->D.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP E Length: 2 Cycles 8 Opcode: 0x33 Flags: Z000
	case (uint8_t)0x33: instruction.setMnemonic("SWAP E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue & 0x0F;
		reg2 = this->E.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->E.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP H Length: 2 Cycles 8 Opcode: 0x34 Flags: Z000
	case (uint8_t)0x34: instruction.setMnemonic("SWAP H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue & 0x0F;
		reg2 = this->H.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->H.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP L Length: 2 Cycles 8 Opcode: 0x35 Flags: Z000
	case (uint8_t)0x35: instruction.setMnemonic("SWAP L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue & 0x0F;
		reg2 = this->L.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->L.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP (HL) Length: 2 Cycles 16 Opcode: 0x36 Flags: Z000
	case (uint8_t)0x36: instruction.setMnemonic("SWAP (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		lsb = this->memory->read(addr);
		reg1 = lsb & 0x0F;
		reg2 = lsb & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->memory->write(addr, result);
		PC++;
		instructionCaught = true;
		break;
		// SWAP A Length: 2 Cycles 8 Opcode: 0x37 Flags: Z000
	case (uint8_t)0x37: instruction.setMnemonic("SWAP A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue & 0x0F;
		reg2 = this->A.getValue & 0xF0;
		result = ((reg1 >> 4) & 0xF0) | ((reg2 << 4) & 0x0F);
		if (result == 0)
		{
			this->F.setValue(this->F.getValue() | 0b10000000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b00000000);
		this->F.setValue(this->F.getValue() | 0b10000000);
		this->A.setValue(result);
		PC++;
		instructionCaught = true;
		break;
		// SRL B Length: 2 Cycles 8 Opcode: 0x38 Flags: Z00C
	case (uint8_t)0x38: instruction.setMnemonic("SRL B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL C Length: 2 Cycles 8 Opcode: 0x39 Flags: Z00C
	case (uint8_t)0x39: instruction.setMnemonic("SRL C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL D Length: 2 Cycles 8 Opcode: 0x3A Flags: Z00C
	case (uint8_t)0x3A: instruction.setMnemonic("SRL D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL E Length: 2 Cycles 8 Opcode: 0x3B Flags: Z00C
	case (uint8_t)0x3B: instruction.setMnemonic("SRL E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL H Length: 2 Cycles 8 Opcode: 0x3C Flags: Z00C
	case (uint8_t)0x3C: instruction.setMnemonic("SRL H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL L Length: 2 Cycles 8 Opcode: 0x3D Flags: Z00C
	case (uint8_t)0x3D: instruction.setMnemonic("SRL L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL (HL) Length: 2 Cycles 16 Opcode: 0x3E Flags: Z00C
	case (uint8_t)0x3E: instruction.setMnemonic("SRL (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SRL A Length: 2 Cycles 8 Opcode: 0x3F Flags: Z00C
	case (uint8_t)0x3F: instruction.setMnemonic("SRL A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// BIT 0, B Length: 2 Cycles 8 Opcode: 0x40 Flags: Z01-
	case (uint8_t)0x40: instruction.setMnemonic("BIT 0, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if((reg1 & 0x01) == 0x01)
		{ 
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, C Length: 2 Cycles 8 Opcode: 0x41 Flags: Z01-
	case (uint8_t)0x41: instruction.setMnemonic("BIT 0, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, D Length: 2 Cycles 8 Opcode: 0x42 Flags: Z01-
	case (uint8_t)0x42: instruction.setMnemonic("BIT 0D, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, E Length: 2 Cycles 8 Opcode: 0x43 Flags: Z01-
	case (uint8_t)0x43: instruction.setMnemonic("BIT 0, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, H Length: 2 Cycles 8 Opcode: 0x44 Flags: Z01-
	case (uint8_t)0x44: instruction.setMnemonic("BIT 0, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, L Length: 2 Cycles 8 Opcode: 0x45 Flags: Z01-
	case (uint8_t)0x45: instruction.setMnemonic("BIT 0, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, (HL) Length: 2 Cycles 16 Opcode: 0x46 Flags: Z01-
	case (uint8_t)0x46: instruction.setMnemonic("LD B, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 0, A Length: 2 Cycles 8 Opcode: 0x47 Flags: Z01-
	case (uint8_t)0x47: instruction.setMnemonic("BIT 0, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x01) == 0x01)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, B Length: 2 Cycles 8 Opcode: 0x48 Flags: Z01-
	case (uint8_t)0x48: instruction.setMnemonic("BIT 1, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, C Length: 2 Cycles 8 Opcode: 0x49 Flags: Z01-
	case (uint8_t)0x49: instruction.setMnemonic("BIT 1, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, D Length: 2 Cycles 8 Opcode: 0x4A Flags: Z01-
	case (uint8_t)0x4A: instruction.setMnemonic("BIT 1, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, E Length: 2 Cycles 8 Opcode: 0x4B Flags: Z01-
	case (uint8_t)0x4B: instruction.setMnemonic("BIT 1, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, H Length: 2 Cycles 8 Opcode: 0x4C Flags: Z01-
	case (uint8_t)0x4C: instruction.setMnemonic("BIT 1, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, L Length: 2 Cycles 8 Opcode: 0x4D Flags: Z01-
	case (uint8_t)0x4D: instruction.setMnemonic("BIT 1, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, (HL) Length: 2 Cycles 16 Opcode: 0x4E Flags: Z01-
	case (uint8_t)0x4E: instruction.setMnemonic("BIT 1, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 1, A Length: 2 Cycles 8 Opcode: 0x4F Flags: Z01-
	case (uint8_t)0x4F: instruction.setMnemonic("BIT 1, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x02) == 0x02)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, B Length: 2 Cycles 8 Opcode: 0x50 Flags: Z01-
	case (uint8_t)0x50: instruction.setMnemonic("BIT 2, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, C Length: 2 Cycles 8 Opcode: 0x51 Flags: Z01-
	case (uint8_t)0x51: instruction.setMnemonic("BIT 2, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, D Length: 2 Cycles 8 Opcode: 0x52 Flags: Z01-
	case (uint8_t)0x52: instruction.setMnemonic("BIT 2, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;;
		instructionCaught = true;
		break;
		// BIT 2, E Length: 2 Cycles 8 Opcode: 0x53 Flags: Z01-
	case (uint8_t)0x53: instruction.setMnemonic("BIT 2, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, H Length: 2 Cycles 8 Opcode: 0x54 Flags: Z01-
	case (uint8_t)0x54: instruction.setMnemonic("BIT 2, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, L Length: 2 Cycles 8 Opcode: 0x55 Flags: Z01-
	case (uint8_t)0x55: instruction.setMnemonic("BIT 2, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, (HL) Length: 2 Cycles 16 Opcode: 0x56 Flags: Z01-
	case (uint8_t)0x56: instruction.setMnemonic("BIT 2, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 2, A Length: 2 Cycles 8 Opcode: 0x57 Flags: Z01-
	case (uint8_t)0x57: instruction.setMnemonic("LD E, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x04) == 0x04)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, B Length: 2 Cycles 8 Opcode: 0x58 Flags: Z01-
	case (uint8_t)0x58: instruction.setMnemonic("BIT 3, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, C Length: 2 Cycles 8 Opcode: 0x59 Flags: Z01-
	case (uint8_t)0x59: instruction.setMnemonic("BIT 3, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, D Length: 2 Cycles 8 Opcode: 0x5A Flags: Z01-
	case (uint8_t)0x5A: instruction.setMnemonic("BIT 3, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, E Length: 2 Cycles 8 Opcode: 0x5B Flags: Z01-
	case (uint8_t)0x5B: instruction.setMnemonic("BIT 3, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, H Length: 2 Cycles 8 Opcode: 0x5C Flags: Z01-
	case (uint8_t)0x5C: instruction.setMnemonic("BIT 3, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, L Length: 2 Cycles 8 Opcode: 0x5D Flags: Z01-
	case (uint8_t)0x5D: instruction.setMnemonic("BIT 3, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, (HL) Length: 2 Cycles 16 Opcode: 0x5E Flags: Z01-
	case (uint8_t)0x5E: instruction.setMnemonic("BIT 3, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 3, A Length: 2 Cycles 8 Opcode: 0x5F Flags: Z01-
	case (uint8_t)0x5F: instruction.setMnemonic("BIT 3, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x08) == 0x08)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, B Length: 2 Cycles 8 Opcode: 0x60 Flags: Z01-
	case (uint8_t)0x60: instruction.setMnemonic("BIT 4, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, C Length: 2 Cycles 8 Opcode: 0x61 Flags: Z01-
	case (uint8_t)0x61: instruction.setMnemonic("BIT 4, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, D Length: 2 Cycles 8 Opcode: 0x62 Flags: Z01-
	case (uint8_t)0x62: instruction.setMnemonic("BIT 4, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, E Length: 2 Cycles 8 Opcode: 0x63 Flags: Z01-
	case (uint8_t)0x63: instruction.setMnemonic("BIT 4, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, H Length: 2 Cycles 8 Opcode: 0x64 Flags: Z01-
	case (uint8_t)0x64: instruction.setMnemonic("BIT 4, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, L Length: 2 Cycles 8 Opcode: 0x65 Flags: Z01-
	case (uint8_t)0x65: instruction.setMnemonic("BIT 4, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, (HL) Length: 2 Cycles 16 Opcode: 0x66 Flags: Z01-
	case (uint8_t)0x66: instruction.setMnemonic("BIT 4, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 4, A Length: 2 Cycles 8 Opcode: 0x67 Flags: Z01-
	case (uint8_t)0x67: instruction.setMnemonic("BIT 4, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x10) == 0x10)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, B Length: 2 Cycles 8 Opcode: 0x68 Flags: Z01-
	case (uint8_t)0x68: instruction.setMnemonic("BIT 5, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, C Length: 2 Cycles 8 Opcode: 0x69 Flags: Z01-
	case (uint8_t)0x69: instruction.setMnemonic("BIT 5, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, D Length: 2 Cycles 8 Opcode: 0x6A Flags: Z01-
	case (uint8_t)0x6A: instruction.setMnemonic("BIT 5, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, E Length: 2 Cycles 8 Opcode: 0x6B Flags: Z01-
	case (uint8_t)0x6B: instruction.setMnemonic("BIT 5, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, H Length: 2 Cycles 8 Opcode: 0x6C Flags: Z01-
	case (uint8_t)0x6C: instruction.setMnemonic("BIT 5, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, L Length: 2 Cycles 8 Opcode: 0x6D Flags: Z01-
	case (uint8_t)0x6D: instruction.setMnemonic("BIT 5, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, (HL) Length: 2 Cycles 8 Opcode: 0x6E Flags: Z01-
	case (uint8_t)0x6E: instruction.setMnemonic("BIT 5, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 5, A Length: 2 Cycles 8 Opcode: 0x6F Flags: Z01-
	case (uint8_t)0x6F: instruction.setMnemonic("BIT 5, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x20) == 0x20)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, B Length: 2 Cycles 8 Opcode: 0x70 Flags: Z01-
	case (uint8_t)0x70: instruction.setMnemonic("LD (HL), B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, C Length: 2 Cycles 8 Opcode: 0x71 Flags: Z01-
	case (uint8_t)0x71: instruction.setMnemonic("BIT 6, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, D Length: 2 Cycles 8 Opcode: 0x72 Flags: Z01-
	case (uint8_t)0x72: instruction.setMnemonic("BIT 6, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, E Length: 2 Cycles 8 Opcode: 0x73 Flags: Z01-
	case (uint8_t)0x73: instruction.setMnemonic("BIT 6, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, H Length: 2 Cycles 8 Opcode: 0x74 Flags: Z01-
	case (uint8_t)0x74: instruction.setMnemonic("BIT 6, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, L Length: 2 Cycles 16 Opcode: 0x75 Flags: Z01-
	case (uint8_t)0x75: instruction.setMnemonic("BIT 6, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, (HL) Length: 2 Cycles 16 Opcode: 0x76 Flags: Z01-
	case (uint8_t)0x76: instruction.setMnemonic("BIT 6, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 6, A Length: 2 Cycles 8 Opcode: 0x77 Flags: Z01-
	case (uint8_t)0x77: instruction.setMnemonic("BIT 6, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x40) == 0x40)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, B Length: 2 Cycles 8 Opcode: 0x78 Flags: Z01-
	case (uint8_t)0x78: instruction.setMnemonic("BIT 7, B");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->B.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, C Length: 2 Cycles 8 Opcode: 0x79 Flags: Z01-
	case (uint8_t)0x79: instruction.setMnemonic("BIT 7, C");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->C.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, D Length: 2 Cycles 8 Opcode: 0x7A Flags: Z01-
	case (uint8_t)0x7A: instruction.setMnemonic("BIT 7, D");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->D.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, E Length: 2 Cycles 8 Opcode: 0x7B Flags: Z01-
	case (uint8_t)0x7B: instruction.setMnemonic("BIT 7, E");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->E.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, H Length: 2 Cycles 8 Opcode: 0x7C Flags: Z01-
	case (uint8_t)0x7C: instruction.setMnemonic("BIT 7, H");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->H.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, L Length: 2 Cycles 8 Opcode: 0x7D Flags: Z01-
	case (uint8_t)0x7D: instruction.setMnemonic("BIT 7, L");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->L.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, (HL) Length: 2 Cycles 16 Opcode: 0x7E Flags: Z01-
	case (uint8_t)0x7E: instruction.setMnemonic("BIT 7, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// BIT 7, A Length: 2 Cycles 8 Opcode: 0x7F Flags: Z01-
	case (uint8_t)0x7F: instruction.setMnemonic("BIT 7, A");//DONE
		cout << instruction.getMnemonic() << endl;
		reg1 = this->A.getValue();
		if ((reg1 & 0x80) == 0x80)
		{
			this->F.setValue(this->F.getValue() & 0b00110000);
		}
		else
			this->F.setValue(this->F.getValue() | 0b10000000);
		this->F.setValue(this->F.getValue() | 0b00100000);
		this->F.setValue(this->F.getValue() & 0b10110000);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, B Length: 2 Cycles 8 Opcode: 0x80 Flags: ----
	case (uint8_t)0x80: instruction.setMnemonic("RES 0, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, C Length: 2 Cycles 8 Opcode: 0x81 Flags: ----
	case (uint8_t)0x81: instruction.setMnemonic("RES 0, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, D Length: 2 Cycles 8 Opcode: 0x82 Flags: ----
	case (uint8_t)0x82: instruction.setMnemonic("RES 0, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, E Length: 2 Cycles 8 Opcode: 0x83 Flags: ----
	case (uint8_t)0x83: instruction.setMnemonic("RES 0, E");
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, H Length: 2 Cycles 8 Opcode: 0x84 Flags: ----
	case (uint8_t)0x84: instruction.setMnemonic("RES 0, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, L Length: 2 Cycles 8 Opcode: 0x85 Flags: ----
	case (uint8_t)0x85: instruction.setMnemonic("RES 0, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, (HL) Length: 2 Cycles 16 Opcode: 0x86 Flags: ----
	case (uint8_t)0x86: instruction.setMnemonic("RES 0, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11111110;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 0, A Length: 2 Cycles 8 Opcode: 0x87 Flags: ----
	case (uint8_t)0x87: instruction.setMnemonic("RES 0, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11111110);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, B Length: 2 Cycles 8 Opcode: 0x88 Flags: ----
	case (uint8_t)0x88: instruction.setMnemonic("RES 1, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, C Length: 2 Cycles 8 Opcode: 0x89 Flags: ----
	case (uint8_t)0x89: instruction.setMnemonic("RES 1, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, D Length: 2 Cycles 8 Opcode: 0x8A Flags: ----
	case (uint8_t)0x8A: instruction.setMnemonic("RES 1, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, E Length: 2 Cycles 8 Opcode: 0x8B Flags: ----
	case (uint8_t)0x8B: instruction.setMnemonic("RES 1, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, H Length: 2 Cycles 8 Opcode: 0x8C Flags: ----
	case (uint8_t)0x8C: instruction.setMnemonic("RES 1, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, L Length: 2 Cycles 8 Opcode: 0x8D Flags: ----
	case (uint8_t)0x8D: instruction.setMnemonic("RES 1, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, (HL) Length: 2 Cycles 8 Opcode: 0x8E Flags: ----
	case (uint8_t)0x8E: instruction.setMnemonic("RES 1, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11111101;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 1, A Length: 2 Cycles 8 Opcode: 0x8F Flags: ----
	case (uint8_t)0x8F: instruction.setMnemonic("RES 1, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11111101);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, B Length: 2 Cycles 8 Opcode: 0x90 Flags: ----
	case (uint8_t)0x90: instruction.setMnemonic("RES 2, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, C Length: 2 Cycles 8 Opcode: 0x91 Flags: ----
	case (uint8_t)0x91: instruction.setMnemonic("RES 2, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, D Length: 2 Cycles 8 Opcode: 0x92 Flags: ----
	case (uint8_t)0x92: instruction.setMnemonic("RES 2, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, E Length: 2 Cycles 8 Opcode: 0x93 Flags: ----
	case (uint8_t)0x93: instruction.setMnemonic("RES 2, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, H Length: 2 Cycles 8 Opcode: 0x94 Flags: ----
	case (uint8_t)0x94: instruction.setMnemonic("RES 2, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, L Length: 2 Cycles 8 Opcode: 0x95 Flags: ----
	case (uint8_t)0x95: instruction.setMnemonic("RES 2, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, (HL) Length: 2 Cycles 16 Opcode: 0x96 Flags: ----
	case (uint8_t)0x96: instruction.setMnemonic("RES 2, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11111011;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 2, A Length: 2 Cycles 8 Opcode: 0x97 Flags: ----
	case (uint8_t)0x97: instruction.setMnemonic("RES 2, A");
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11111011);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, B Length: 2 Cycles 8 Opcode: 0x98 Flags: ----
	case (uint8_t)0x98: instruction.setMnemonic("RES 3, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, C Length: 2 Cycles 8 Opcode: 0x99 Flags: ----
	case (uint8_t)0x99: instruction.setMnemonic("RES 3, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, D Length: 2 Cycles 8 Opcode: 0x9A Flags: ----
	case (uint8_t)0x9A: instruction.setMnemonic("RES 3, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, E Length: 2 Cycles 8 Opcode: 0x9B Flags: ----
	case (uint8_t)0x9B: instruction.setMnemonic("RES 3, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, H Length: 2 Cycles 8 Opcode: 0x9C Flags: ----
	case (uint8_t)0x9C: instruction.setMnemonic("RES 3, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, L Length: 2 Cycles 8 Opcode: 0x9D Flags: ----
	case (uint8_t)0x9D: instruction.setMnemonic("RES 3, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, (HL) Length: 2 Cycles 16 Opcode: 0x9E Flags: ----
	case (uint8_t)0x9E: instruction.setMnemonic("RES 3, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11110111;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 3, A Length: 2 Cycles 8 Opcode: 0x9F Flags: ----
	case (uint8_t)0x9F: instruction.setMnemonic("RES 3, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11110111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, B Length: 2 Cycles 8 Opcode: 0xA0 Flags: ----
	case (uint8_t)0xA0: instruction.setMnemonic("RES 4, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, C Length: 2 Cycles 8 Opcode: 0xA1 Flags: ----
	case (uint8_t)0xA1: instruction.setMnemonic("RES 4, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, D Length: 2 Cycles 8 Opcode: 0xA2 Flags: ----
	case (uint8_t)0xA2: instruction.setMnemonic("RES 4, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, E Length: 2 Cycles 8 Opcode: 0xA3 Flags: ----
	case (uint8_t)0xA3: instruction.setMnemonic("RES 4, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, H Length: 2 Cycles 8 Opcode: 0xA4 Flags: ----
	case (uint8_t)0xA4: instruction.setMnemonic("RES 4, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, L Length: 2 Cycles 8 Opcode: 0xA5 Flags: ----
	case (uint8_t)0xA5: instruction.setMnemonic("RES 4, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, (HL) Length: 2 Cycles 8 Opcode: 0xA6 Flags: ----
	case (uint8_t)0xA6: instruction.setMnemonic("RES 4, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11101111;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 4, A Length: 2 Cycles 8 Opcode: 0xA7 Flags: ----
	case (uint8_t)0xA7: instruction.setMnemonic("RES 4, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11101111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, B Length: 2 Cycles 8 Opcode: 0xA8 Flags: ----
	case (uint8_t)0xA8: instruction.setMnemonic("RES 5, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, C Length: 2 Cycles 8 Opcode: 0xA9 Flags: ----
	case (uint8_t)0xA9: instruction.setMnemonic("RES 5, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, D Length: 2 Cycles 8 Opcode: 0xAA Flags: ----
	case (uint8_t)0xAA: instruction.setMnemonic("RES 5, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, E Length: 2 Cycles 8 Opcode: 0xAB Flags: ----
	case (uint8_t)0xAB: instruction.setMnemonic("RES 5, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, H Length: 2 Cycles 8 Opcode: 0xAC Flags: ----
	case (uint8_t)0xAC: instruction.setMnemonic("RES 5, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, L Length: 2 Cycles 8 Opcode: 0xAD Flags: ----
	case (uint8_t)0xAD: instruction.setMnemonic("RES 5, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, (HL) Length: 2 Cycles 8 Opcode: 0xAE Flags: ----
	case (uint8_t)0xAE: instruction.setMnemonic("RES 5, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b11011111;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 5, A Length: 2 Cycles 8 Opcode: 0xAF Flags: ----
	case (uint8_t)0xAF: instruction.setMnemonic("RES 5, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b11011111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, B Length: 2 Cycles 8 Opcode: 0xB0 Flags: ----
	case (uint8_t)0xB0: instruction.setMnemonic("RES 6, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, C Length: 2 Cycles 8 Opcode: 0xB1 Flags: ----
	case (uint8_t)0xB1: instruction.setMnemonic("RES 6, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, D Length: 2 Cycles 8 Opcode: 0xB2 Flags: ----
	case (uint8_t)0xB2: instruction.setMnemonic("RES 6, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, E Length: 2 Cycles 8 Opcode: 0xB3 Flags: ----
	case (uint8_t)0xB3: instruction.setMnemonic("RES 6, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, H Length: 2 Cycles 8 Opcode: 0xB4 Flags: ----
	case (uint8_t)0xB4: instruction.setMnemonic("RES 6, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, L Length: 2 Cycles 8 Opcode: 0xB5 Flags: ----
	case (uint8_t)0xB5: instruction.setMnemonic("RES 6, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, (HL) Length: 2 Cycles 16 Opcode: 0xB6 Flags: ----
	case (uint8_t)0xB6: instruction.setMnemonic("RES 6, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b10111111;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 6, A Length: 2 Cycles 8 Opcode: 0xB7 Flags: ----
	case (uint8_t)0xB7: instruction.setMnemonic("RES 6, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b10111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, B Length: 2 Cycles 8 Opcode: 0xB8 Flags: ----
	case (uint8_t)0xB8: instruction.setMnemonic("RES 7, B");//DONE
		cout << instruction.getMnemonic() << endl;
		this->B.setValue(this->B.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, C Length: 2 Cycles 8 Opcode: 0xB9 Flags: ----
	case (uint8_t)0xB9: instruction.setMnemonic("RES 7, C");//DONE
		cout << instruction.getMnemonic() << endl;
		this->C.setValue(this->C.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, D Length: 2 Cycles 8 Opcode: 0xBA Flags: ----
	case (uint8_t)0xBA: instruction.setMnemonic("RES 7, D");//DONE
		cout << instruction.getMnemonic() << endl;
		this->D.setValue(this->D.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, E Length: 2 Cycles 8 Opcode: 0xBB Flags: ----
	case (uint8_t)0xBB: instruction.setMnemonic("RES 7, E");//DONE
		cout << instruction.getMnemonic() << endl;
		this->E.setValue(this->E.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, H Length: 2 Cycles 8 Opcode: 0xBC Flags: ----
	case (uint8_t)0xBC: instruction.setMnemonic("RES 7, H");//DONE
		cout << instruction.getMnemonic() << endl;
		this->H.setValue(this->H.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, L Length: 2 Cycles 8 Opcode: 0xBD Flags: ----
	case (uint8_t)0xBD: instruction.setMnemonic("RES 7, L");//DONE
		cout << instruction.getMnemonic() << endl;
		this->L.setValue(this->L.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, (HL) Length: 2 Cycles 16 Opcode: 0xBE Flags: ----
	case (uint8_t)0xBE: instruction.setMnemonic("RES 7, (HL)");//DONE
		cout << instruction.getMnemonic() << endl;
		addr = ((this->H.getValue() >> 8) & 0xFF00) | (this->L.getValue() & 0x00FF);
		reg1 = this->memory->read(addr);
		reg1 = reg1 & 0b01111111;
		this->memory->write(addr, reg1);
		PC++;
		instructionCaught = true;
		break;
		// RES 7, A Length: 2 Cycles 8 Opcode: 0xBF Flags: ----
	case (uint8_t)0xBF: instruction.setMnemonic("RES 7, A");//DONE
		cout << instruction.getMnemonic() << endl;
		this->A.setValue(this->A.getValue() & 0b01111111);
		PC++;
		instructionCaught = true;
		break;
		// SET 0, B Length: 2 Cycles 8 Opcode: 0xC0 Flags: ----
	case (uint8_t)0xC0: instruction.setMnemonic("SET 0, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, C Length: 2 Cycles 8 Opcode: 0xC1 Flags: ----
	case (uint8_t)0xC1: instruction.setMnemonic("SET 0, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, D Length: 2 Cycles 8 Opcode: 0xC2 Flags: ----
	case (uint8_t)0xC2: instruction.setMnemonic("SET 0, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, E Length: 2 Cycles 8 Opcode: 0xC3 Flags: ----
	case (uint8_t)0xC3: instruction.setMnemonic("SET 0, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, H Length: 2 Cycles 8 Opcode: 0xC4 Flags: ----
	case (uint8_t)0xC4: instruction.setMnemonic("SET 0, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, L Length: 2 Cycles 8 Opcode: 0xC5 Flags: ----
	case (uint8_t)0xC5: instruction.setMnemonic("SET 0, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, (HL) Length: 2 Cycles 16 Opcode: 0xC6 Flags: ----
	case (uint8_t)0xC6: instruction.setMnemonic("SET 0, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 0, A Length: 2 Cycles 8 Opcode: 0xC7 Flags: ----
	case (uint8_t)0xC7: instruction.setMnemonic("SET 0, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, B Length: 2 Cycles 8 Opcode: 0xC8 Flags: ----
	case (uint8_t)0xC8: instruction.setMnemonic("SET 1, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, C Length: 2 Cycles 8 Opcode: 0xC9 Flags: ----
	case (uint8_t)0xC9: instruction.setMnemonic("SET 1, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, D Length: 2 Cycles 8 Opcode: 0xCA Flags: ----
	case (uint8_t)0xCA: instruction.setMnemonic("SET 1, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, E Length: 2 Cycles 8 Opcode: 0xCB Flags: ----
	case (uint8_t)0xCB: instruction.setMnemonic("SET 1, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, H Length: 2 Cycles 8 Opcode: 0xCC Flags: ----
	case (uint8_t)0xCC: instruction.setMnemonic("SET 1, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, L Length: 2 Cycles 8 Opcode: 0xCD Flags: ----
	case (uint8_t)0xCD: instruction.setMnemonic("SET 1, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, (HL) Length: 2 Cycles 16 Opcode: 0xCE Flags: ----
	case (uint8_t)0xCE: instruction.setMnemonic("SET 1, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 1, A Length: 2 Cycles 8 Opcode: 0xCF Flags: ----
	case (uint8_t)0xCF: instruction.setMnemonic("SET 1, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, B Length: 2 Cycles 8 Opcode: 0xD0 Flags: ----
	case (uint8_t)0xD0: instruction.setMnemonic("SET 2, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, C Length: 2 Cycles 8 Opcode: 0xD1 Flags: ----
	case (uint8_t)0xD1: instruction.setMnemonic("SET 2, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, D Length: 2 Cycles 8 Opcode: 0xD2 Flags: ----
	case (uint8_t)0xD2: instruction.setMnemonic("SET 2, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, E Length: 2 Cycles 8 Opcode: 0xD3 Flags: ----
	case (uint8_t)0xD3: instruction.setMnemonic("SET 2, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, H Length: 2 Cycles 8 Opcode: 0xD4 Flags: ----
	case (uint8_t)0xD4: instruction.setMnemonic("SET 2, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, L Length: 2 Cycles 8 Opcode: 0xD5 Flags: ----
	case (uint8_t)0xD5: instruction.setMnemonic("SET 2, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, (HL) Length: 2 Cycles 16 Opcode: 0xD6 Flags: ----
	case (uint8_t)0xD6: instruction.setMnemonic("SET 2, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 2, A Length: 2 Cycles 8 Opcode: 0xD7 Flags: ----
	case (uint8_t)0xD7: instruction.setMnemonic("SET 2, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, B Length: 2 Cycles 8 Opcode: 0xD8 Flags: ----
	case (uint8_t)0xD8: instruction.setMnemonic("SET 3, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, C Length: 2 Cycles 8 Opcode: 0xD9 Flags: ----
	case (uint8_t)0xD9: instruction.setMnemonic("SET 3, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, D Length: 2 Cycles 8 Opcode: 0xDA Flags: ----
	case (uint8_t)0xDA: instruction.setMnemonic("SET 3, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, E Length: 2 Cycles 8 Opcode: 0xDB Flags: ----
	case (uint8_t)0xDB:instruction.setMnemonic("SET 3, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, H Length: 2 Cycles 8 Opcode: 0xDC Flags: ----
	case (uint8_t)0xDC: instruction.setMnemonic("SET 3, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, L Length: 2 Cycles 8 Opcode: 0xDD Flags: ----
	case (uint8_t)0xDD:instruction.setMnemonic("SET 3, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, (HL) Length: 2 Cycles 16 Opcode: 0xDE Flags: ----
	case (uint8_t)0xDE: instruction.setMnemonic("SET 3, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 3, A Length: 2 Cycles 8 Opcode: 0xDF Flags: ----
	case (uint8_t)0xDF: instruction.setMnemonic("SET 3, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, B Length: 2 Cycles 8 Opcode: 0xE0 Flags: ----
	case (uint8_t)0xE0: instruction.setMnemonic("SET 4, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, C Length: 2 Cycles 8 Opcode: 0xE1 Flags: ----
	case (uint8_t)0xE1: instruction.setMnemonic("SET 4, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, D Length: 2 Cycles 8 Opcode: 0xE2 Flags: ----
	case (uint8_t)0xE2: instruction.setMnemonic("SET 4, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, E Length: 2 Cycles 8 Opcode: 0xE3 Flags: ----
	case (uint8_t)0xE3: instruction.setMnemonic("SET 4, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, H Length: 2 Cycles 8 Opcode: 0xE4 Flags: ----
	case (uint8_t)0xE4: instruction.setMnemonic("SET 4, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, L Length: 2 Cycles 8 Opcode: 0xE5 Flags: ----
	case (uint8_t)0xE5: instruction.setMnemonic("SET 4, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, (HL) Length: 2 Cycles 16 Opcode: 0xE6 Flags: ----
	case (uint8_t)0xE6: instruction.setMnemonic("SET 4, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 4, A Length: 2 Cycles 8 Opcode: 0xE7 Flags: ----
	case (uint8_t)0xE7: instruction.setMnemonic("SET 4, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, B Length: 2 Cycles 8 Opcode: 0xE8 Flags: ----
	case (uint8_t)0xE8: instruction.setMnemonic("SET 5, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, C Length: 2 Cycles 8 Opcode: 0xE9 Flags: ----
	case (uint8_t)0xE9: instruction.setMnemonic("SET 5, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, D Length: 2 Cycles 8 Opcode: 0xEA Flags: ----
	case (uint8_t)0xEA: instruction.setMnemonic("SET 5, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, E Length: 2 Cycles 8 Opcode: 0xEB Flags: ----
	case (uint8_t)0xEB: instruction.setMnemonic("SET 5, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, H Length: 2 Cycles 8 Opcode: 0xEC Flags: ----
	case (uint8_t)0xEC: instruction.setMnemonic("SET 5, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, L Length: 2 Cycles 8 Opcode: 0xED Flags: ----
	case (uint8_t)0xED: instruction.setMnemonic("SET 5, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, (HL) Length: 2 Cycles 8 Opcode: 0xEE Flags: ----
	case (uint8_t)0xEE: instruction.setMnemonic("SET 5, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 5, A Length: 2 Cycles 8 Opcode: 0xEF Flags: ----
	case (uint8_t)0xEF: instruction.setMnemonic("RST 28H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, B Length: 2 Cycles 8 Opcode: 0xF0 Flags: ----
	case (uint8_t)0xF0: instruction.setMnemonic("SET 6, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, C Length: 2 Cycles 8 Opcode: 0xF1 Flags: ----
	case (uint8_t)0xF1: instruction.setMnemonic("SET 6, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, D Length: 2 Cycles 8 Opcode: 0xF2 Flags: ----
	case (uint8_t)0xF2: instruction.setMnemonic("SET 6, D");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, E Length: 2 Cycles 8 Opcode: 0xF3 Flags: ----
	case (uint8_t)0xF3: instruction.setMnemonic("SET 6, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, H Length: 2 Cycles 8 Opcode: 0xF4 Flags: ----
	case (uint8_t)0xF4: instruction.setMnemonic("SET 6, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, L Length: 2 Cycles 8 Opcode: 0xF5 Flags: ----
	case (uint8_t)0xF5: instruction.setMnemonic("SET 6, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, (HL) Length: 2 Cycles 16 Opcode: 0xF6 Flags: ----
	case (uint8_t)0xF6: instruction.setMnemonic("SET 6, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 6, A Length: 2 Cycles 8 Opcode: 0xF7 Flags: ----
	case (uint8_t)0xF7: instruction.setMnemonic("SET 6, A");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, B Length: 2 Cycles 8 Opcode: 0xF8 Flags: ----
	case (uint8_t)0xF8: instruction.setMnemonic("SET 7, B");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, C Length: 2 Cycles 8 Opcode: 0xF9 Flags: ----
	case (uint8_t)0xF9: instruction.setMnemonic("SET 7, C");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, D Length: 2 Cycles 8 Opcode: 0xFA Flags: ----
	case (uint8_t)0xFA: instruction.setMnemonic("SET 7, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, E Length: 2 Cycles 8 Opcode: 0xFB Flags: ----
	case (uint8_t)0xFB: instruction.setMnemonic("SET 7, E");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, H Length: 2 Cycles 8 Opcode: 0xFC Flags: ----
	case (uint8_t)0xFC: instruction.setMnemonic("SET 7, H");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, L Length: 2 Cycles 8 Opcode: 0xFD Flags: ----
	case (uint8_t)0xFD: instruction.setMnemonic("SET 7, L");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, (HL) Length: 2 Cycles 8 Opcode: 0xFE Flags: ----
	case (uint8_t)0xFE: instruction.setMnemonic("SET 7, (HL)");
		cout << instruction.getMnemonic() << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
		// SET 7, A Length: 2 Cycles 8 Opcode: 0xFF Flags: ----
	case (uint8_t)0xFF: instruction.setMnemonic("SET 7, A");
		cout << "RST" << endl;
		PC = PC + 1;
		instructionCaught = true;
		break;
	}
	if (!instructionCaught)
	{
		cout << "instrution unsupported opcode" << endl;
	}
	return instructionCaught;
}