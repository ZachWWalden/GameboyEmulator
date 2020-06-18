#pragma once
#include <string>
using namespace std;
class Instruction
{
//attributes
public:
private:
	uint8_t opCode;
	string instrucitonMnemonic;
	short data;
	short numberOfCyclesLeft;
//methods
public:
	Instruction();

	uint8_t getOpCode();
	void setOpCode(uint8_t opCode);

	string getMnemonic();
	void setMnemonic(string mnemonic);

	short getData();
	void setData(short newData);

	short getNumberOfCyclesLeft();
	void setNumberOfCyclesLeft(short numCycles);
private:
};

Instruction::Instruction()
{

}

uint8_t Instruction::getOpCode()
{
	return this->opCode;
}
void Instruction::setOpCode(uint8_t opCode)
{
	this->opCode = opCode;
}

string Instruction::getMnemonic()
{
	return this->instrucitonMnemonic;
}
void Instruction::setMnemonic(string mnemonic)
{
	this->instrucitonMnemonic = mnemonic;
}