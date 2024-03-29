// GameBoyEmulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <fstream>
#include "CPU.h"
#include "Memory.h"
using namespace std;

int main()
{
	//open rom file
	fstream romFile, bootRom;

	romFile.open("ROM.gb", ios::in | ios::binary);

	bootRom.open("bootRom.bin", ios::in | ios::binary);

	Memory memory(romFile, bootRom);
	romFile.close();

	CPU cpu(&memory, 0L);

	cpu.stepCPU();

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
