// M68kEmulator.cpp : Defines the entry point for the console application.
//

#include "CPUCore.h"
#include "Memory.h"
#include "ProgramLoader.h"
#include <iostream>

using namespace std;

int main()
{
	Memory *memory = new Memory(256);
	CPUCore *cpu = new CPUCore(memory, 68000);
	cpu->debugMode = false;
	ProgramLoader::loadProgram("program.S68", cpu, memory);
	bool cpuRunning = true;
	while (cpuRunning)
		cpuRunning = cpu->startNextCycle();
	cout << "Execution completed." << endl << endl;
	if (cpu->debugMode) {
		cpu->displayInfo();
		memory->dumpMemoryToConsole();
		memory->dumpMemoryToFile("core_dump.txt");
	}
    return 0;
}

