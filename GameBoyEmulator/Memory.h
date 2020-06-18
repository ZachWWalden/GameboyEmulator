#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
using namespace std;

class Memory
{
	// Attributes
public:
private:
	uint8_t* bootRom;
	uint8_t* cartridgeRom;
	uint8_t* mainMemory = new uint8_t[0xFFFF];
	int cartBank0Start = 0x0000, cartBank0End = 0x3FFF;
	int cartBank1NStart = 0x4000, cartBank1NEnd = 0x7FFF;
	//bankStartAddress = bankNumber * 16,384
	int vRamStart = 0x8000, vRamEnd = 0x9FFF;
	int exRamStart = 0xA000, exRamEnd = 0xBFFF;
	int ramStart = 0xC000, ramEnd = 0xDFFF;
	int oamRamStart = 0xFE00, oamRamEnd = 0xFE9F;
	int ioRamStart = 0xFF00, ioRamEnd = 0xFF7F;
	int hRamStart = 0xFF80, hRamEnd = 0xFFFF;
	int cartSize = 0L;
	int bootRomSize = 0L;
	//Methods
public:
	Memory();
	Memory(fstream &romFile, fstream &bootRom);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t writeValue);

	uint8_t* getCartRom();
	uint8_t* getMainMemory();
	int getCartRomSize();

private:
	void loadInArray(uint8_t* array, int startAddressMemory, int startAAddressArray, int size);
	void changeCartridgeROMBank(int bankNumber);
};


Memory::Memory()
{

}
Memory::Memory(fstream &romFile, fstream &bootRom)
{
	//read in romFile
	romFile.seekg(0L, ios::end);
	this->cartSize = romFile.tellg();
	this->cartridgeRom = new uint8_t[this->cartSize];
	romFile.clear();
	romFile.seekg(0L, ios::beg);

	uint8_t fileReadIn = 0x00000000;
	int PCess = 0;
	while ((romFile.read(&fileReadIn, sizeof(uint8_t))))
	{
		this->cartridgeRom[PCess] = fileReadIn;
		PCess++;
	}

	//initialize boot rom
	bootRom.seekg(0L, ios::end);
	this->bootRomSize = bootRom.tellg();
	this->bootRom = new uint8_t[this->bootRomSize];
	bootRom.clear();
	bootRom.seekg(0L, ios::beg);

	PCess = 0;

	while ((romFile.read(&fileReadIn, sizeof(uint8_t))))
	{
		this->bootRom[PCess] = fileReadIn;
		PCess++;
	}
}

uint8_t Memory::read(uint16_t address)
{
	return this->cartridgeRom[address];
}

void Memory::write(uint16_t address, uint8_t writeValue)
{
	this->cartridgeRom[address] = writeValue;
}

uint8_t* Memory::getCartRom()
{
	return this->cartridgeRom;
}
uint8_t* Memory::getMainMemory()
{
	return this->mainMemory;
}
int Memory::getCartRomSize()
{
	return this->cartSize;
}



void Memory::changeCartridgeROMBank(int bankNumber)
{

}
void Memory::loadInArray(uint8_t* array, int startAddressMemory, int startAAddressArray, int size)
{
	int j = startAAddressArray;
	for (int i = startAddressMemory; i < size; i++, j++)
	{
		this->mainMemory[i] = array[j];
	}
}